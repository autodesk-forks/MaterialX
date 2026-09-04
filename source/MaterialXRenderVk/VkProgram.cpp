//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderVk/VkProgram.h>
#include <MaterialXRenderVk/VkUtil.h>

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderStage.h>

#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <algorithm>
#include <cstring>
#include <mutex>
#include <sstream>

MATERIALX_NAMESPACE_BEGIN

unsigned int VkProgram::UNDEFINED_VK_RESOURCE_ID = 0;
int VkProgram::UNDEFINED_VK_PROGRAM_LOCATION = -1;
int VkProgram::Input::INVALID_VK_TYPE = -1;

VkProgram::VkProgram(VkContextPtr context) :
    _context(context)
{
}

VkProgram::~VkProgram()
{
    clearBuiltData();
}

void VkProgram::setStages(ShaderPtr shader)
{
    _shader = shader;
    clearStages();
    for (size_t i = 0; i < shader->numStages(); i++)
    {
        const ShaderStage& stage = shader->getStage(i);
        addStage(stage.getName(), stage.getSourceCode());
    }
}

void VkProgram::addStage(const string& stage, const string& sourceCode)
{
    _stages[stage] = sourceCode;
}

const string& VkProgram::getStageSourceCode(const string& stage) const
{
    auto it = _stages.find(stage);
    if (it != _stages.end())
    {
        return it->second;
    }
    return EMPTY_STRING;
}

void VkProgram::clearStages()
{
    _stages.clear();
}

bool VkProgram::hasBuiltData()
{
    return _built;
}

void VkProgram::clearBuiltData()
{
    if (_vertexModule != VK_NULL_HANDLE && _context && _context->getDevice() != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(_context->getDevice(), _vertexModule, nullptr);
        _vertexModule = VK_NULL_HANDLE;
    }
    if (_fragmentModule != VK_NULL_HANDLE && _context && _context->getDevice() != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(_context->getDevice(), _fragmentModule, nullptr);
        _fragmentModule = VK_NULL_HANDLE;
    }

    VkDevice device = _context ? _context->getDevice() : VK_NULL_HANDLE;
    for (auto& block : _blocks)
    {
        if (block.mapped && device != VK_NULL_HANDLE)
        {
            vkUnmapMemory(device, block.memory);
            block.mapped = nullptr;
        }
        if (block.buffer != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
            vkDestroyBuffer(device, block.buffer, nullptr);
        if (block.memory != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
            vkFreeMemory(device, block.memory, nullptr);
    }
    _blocks.clear();

    if (_descriptorSetLayout != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);
        _descriptorSetLayout = VK_NULL_HANDLE;
    }
    if (_pipelineLayout != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
        _pipelineLayout = VK_NULL_HANDLE;
    }
    if (_descriptorPool != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
        _descriptorPool = VK_NULL_HANDLE;
    }
    if (_pipeline != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, _pipeline, nullptr);
        _pipeline = VK_NULL_HANDLE;
    }

    _spirv.clear();
    _uniformList.clear();
    _attributeList.clear();
    _built = false;
}

namespace
{

// Compile both stages into a single glslang TProgram, link, reflect, and emit SPIR-V.
// This fixes the old branch's per-stage TProgram mistake: a fresh TProgram per stage
// prevents cross-stage reflection with the unified binding numbering the generator emits.
bool compileToSpvInternal(const string& vertexSource, const string& fragmentSource,
                          std::vector<uint32_t>& vertexSpv, std::vector<uint32_t>& fragmentSpv,
                          StringVec& errors)
{
    // glslang is not thread-safe; guard the global initialization.
    static std::mutex glslangMutex;
    std::lock_guard<std::mutex> lock(glslangMutex);

    static bool initialized = false;
    if (!initialized)
    {
        if (!glslang::InitializeProcess())
        {
            errors.push_back("Failed to initialize glslang.");
            return false;
        }
        initialized = true;
    }

    const int defaultVersion = 450;
    const EProfile defaultProfile = ECoreProfile;
    const bool forceVersionProfile = false;
    const bool isForwardCompatible = false;

    auto buildShader = [&](EShLanguage stage, const string& source, const char* name) ->
        std::unique_ptr<glslang::TShader>
    {
        auto shader = std::make_unique<glslang::TShader>(stage);
        const char* sourceCstr = source.c_str();
        int sourceLength = static_cast<int>(source.length());
        shader->setStringsWithLengths(&sourceCstr, &sourceLength, 1);
        shader->setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, defaultVersion);
        shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
        shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);
        shader->setEntryPoint("main");
        shader->setSourceEntryPoint("main");

        const TBuiltInResource* resources = GetDefaultResources();
        EShMessages messages = static_cast<EShMessages>(
            EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

        if (!shader->parse(resources, defaultVersion, defaultProfile, forceVersionProfile,
                           isForwardCompatible, messages))
        {
            errors.push_back(std::string("glslang parse error (") + name + "): " + shader->getInfoLog());
            return nullptr;
        }
        return shader;
    };

    auto vs = buildShader(EShLangVertex, vertexSource, "vertex");
    auto fs = buildShader(EShLangFragment, fragmentSource, "fragment");
    if (!vs || !fs)
        return false;

    glslang::TProgram program;
    program.addShader(vs.get());
    program.addShader(fs.get());

    EShMessages messages = static_cast<EShMessages>(
        EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);
    if (!program.link(messages))
    {
        errors.push_back(std::string("Vulkan GLSL link failed: ") + program.getInfoLog());
        return false;
    }

    // Build reflection with flags that expose UBO members, samplers, and pipe I/O.
    program.buildReflection(EShReflectionSeparateBuffers |
                            EShReflectionAllBlockVariables |
                            EShReflectionAllIOVariables |
                            EShReflectionSharedStd140UBO);

    glslang::GlslangToSpv(*program.getIntermediate(EShLangVertex), vertexSpv);
    glslang::GlslangToSpv(*program.getIntermediate(EShLangFragment), fragmentSpv);

    return true;
}

} // anonymous namespace

bool VkProgram::compileToSpirv(StringVec& errors)
{
    auto vsIt = _stages.find(Stage::VERTEX);
    auto fsIt = _stages.find(Stage::PIXEL);
    if (vsIt == _stages.end() || fsIt == _stages.end())
    {
        errors.push_back("Missing vertex or pixel shader stage.");
        return false;
    }

    std::vector<uint32_t> vertexSpv, fragmentSpv;
    if (!compileToSpvInternal(vsIt->second, fsIt->second, vertexSpv, fragmentSpv, errors))
    {
        return false;
    }

    _spirv[Stage::VERTEX] = std::move(vertexSpv);
    _spirv[Stage::PIXEL] = std::move(fragmentSpv);
    return true;
}

void VkProgram::build(const VkFramebufferPtr& framebuffer)
{
    clearBuiltData();

    StringVec errors;
    if (!compileToSpirv(errors))
    {
        throw ExceptionRenderError("Failed to compile Vulkan shader to SPIR-V", errors);
    }

    // Create shader modules.
    auto createModule = [&](const std::vector<uint32_t>& code) -> VkShaderModule
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = code.data();
        VkShaderModule module = VK_NULL_HANDLE;
        VK_CHECK(vkCreateShaderModule(_context->getDevice(), &createInfo, nullptr, &module),
                 "Failed to create Vulkan shader module");
        return module;
    };

    _vertexModule = createModule(_spirv[Stage::VERTEX]);
    _fragmentModule = createModule(_spirv[Stage::PIXEL]);

    // Introspect uniforms and attributes from the Shader (decoration source).
    // Full glslang reflection is wired in Phase 2; for Phase 1 we populate from
    // the Shader's VariableBlocks so the module builds and links.
    updateUniformsList();
    updateAttributesList();

    // Phase 2 will add: createDescriptorLayout(), UBO staging, bindUniformDefaults(),
    // createPipeline(framebuffer). For Phase 1, build() succeeds once SPIR-V compiles.
    (void)framebuffer;

    _built = true;
}

const VkProgram::InputMap& VkProgram::getUniformsList()
{
    if (_uniformList.empty())
    {
        updateUniformsList();
    }
    return _uniformList;
}

const VkProgram::InputMap& VkProgram::getAttributesList()
{
    if (_attributeList.empty())
    {
        updateAttributesList();
    }
    return _attributeList;
}

const VkProgram::InputMap& VkProgram::updateUniformsList()
{
    _uniformList.clear();
    if (!_shader)
        return _uniformList;

    // Decorate from the Shader's uniform blocks. Phase 2 replaces this with
    // glslang reflection as the source of truth, decorated from the Shader.
    for (size_t i = 0; i < _shader->numStages(); i++)
    {
        const ShaderStage& stage = _shader->getStage(i);
        for (const auto& blockPair : stage.getUniformBlocks())
        {
            const VariableBlock& block = *blockPair.second;
            for (size_t j = 0; j < block.size(); j++)
            {
                const ShaderPort* port = block[j];
                if (!port)
                    continue;
                const string& name = port->getVariable();
                if (_uniformList.find(name) == _uniformList.end())
                {
                    auto input = std::make_shared<Input>(
                        UNDEFINED_VK_PROGRAM_LOCATION, UNDEFINED_VK_PROGRAM_LOCATION,
                        1, port->getPath());
                    input->typeString = port->getType().getName();
                    input->value = port->getValue();
                    input->unit = port->getUnit();
                    input->colorspace = port->getColorSpace();
                    _uniformList[name] = input;
                }
            }
        }
    }
    return _uniformList;
}

const VkProgram::InputMap& VkProgram::updateAttributesList()
{
    _attributeList.clear();
    if (!_shader)
        return _attributeList;

    // Decorate from the Shader's vertex attributes.
    for (size_t i = 0; i < _shader->numStages(); i++)
    {
        const ShaderStage& stage = _shader->getStage(i);
        for (const auto& blockPair : stage.getInputBlocks())
        {
            const VariableBlock& block = *blockPair.second;
            for (size_t j = 0; j < block.size(); j++)
            {
                const ShaderPort* port = block[j];
                if (!port)
                    continue;
                const string& name = port->getVariable();
                if (_attributeList.find(name) == _attributeList.end())
                {
                    auto input = std::make_shared<Input>(
                        UNDEFINED_VK_PROGRAM_LOCATION, UNDEFINED_VK_PROGRAM_LOCATION,
                        1, port->getPath());
                    input->typeString = port->getType().getName();
                    _attributeList[name] = input;
                }
            }
        }
    }
    return _attributeList;
}

ConstValuePtr VkProgram::findUniformValue(const string& uniformName, const InputMap& uniformList)
{
    auto it = uniformList.find(uniformName);
    if (it != uniformList.end())
    {
        return it->second->value;
    }
    return nullptr;
}

void VkProgram::findInputs(const string& variable, const InputMap& variableList,
                          InputMap& foundList, bool exactMatch)
{
    for (const auto& it : variableList)
    {
        const string& name = it.first;
        if (exactMatch)
        {
            if (name == variable)
            {
                foundList[name] = it.second;
            }
        }
        else
        {
            if (name.find(variable) != string::npos)
            {
                foundList[name] = it.second;
            }
        }
    }
}

bool VkProgram::bind(VkCommandBuffer cmd)
{
    (void)cmd;
    // Phase 3: bind pipeline + descriptor set, flush dirty UBOs.
    return true;
}

void VkProgram::unbind() const
{
    // Phase 3.
}

bool VkProgram::hasActiveAttributes() const
{
    return !_attributeList.empty();
}

bool VkProgram::hasUniform(const string& name)
{
    return _uniformList.find(name) != _uniformList.end();
}

void VkProgram::bindUniform(const string& name, ConstValuePtr value, bool errorIfMissing)
{
    auto it = _uniformList.find(name);
    if (it == _uniformList.end())
    {
        if (errorIfMissing)
        {
            throw ExceptionRenderError("Cannot find uniform: " + name);
        }
        return;
    }
    it->second->value = value;
    // Phase 2: write into _blocks[input->blockIndex].shadow at input->offset, mark dirty.
}

void VkProgram::bindPartition(MeshPartitionPtr partition)
{
    (void)partition;
    // Phase 3.
}

void VkProgram::bindMesh(MeshPtr mesh)
{
    (void)mesh;
    // Phase 3.
}

void VkProgram::drawPartition(VkCommandBuffer cmd, MeshPartitionPtr partition)
{
    (void)cmd;
    (void)partition;
    // Phase 3.
}

void VkProgram::unbindGeometry()
{
    // Phase 3.
}

void VkProgram::bindTextures(ImageHandlerPtr imageHandler)
{
    // Guard: bindTextures dereferences _shader (cf. GlslProgram.cpp:541). On the
    // raw-source (StageMap) path _shader is null, so skip here.
    if (!_shader)
        return;
    (void)imageHandler;
    // Phase 3.
}

void VkProgram::bindLighting(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler)
{
    if (!_shader)
        return;
    (void)lightHandler;
    (void)imageHandler;
    // Phase 3.
}

void VkProgram::bindViewInformation(CameraPtr camera)
{
    if (!_shader)
        return;
    (void)camera;
    // Phase 3.
}

void VkProgram::bindTimeAndFrame(float time, float frame)
{
    if (!_shader)
        return;
    (void)time;
    (void)frame;
    // Phase 3.
}

void VkProgram::bindUniformDefaults()
{
    // Phase 2: seed every UBO shadow buffer from the Shader's VariableBlocks.
}

void VkProgram::flushUniforms()
{
    // Phase 2: memcpy dirty shadows into mapped coherent memory.
}

void VkProgram::writeUnboundSamplersWithZeroImage(ImageHandlerPtr imageHandler)
{
    (void)imageHandler;
    // Phase 3.
}

void VkProgram::createDescriptorLayout()
{
    // Phase 2.
}

void VkProgram::createPipeline(const VkFramebufferPtr& framebuffer)
{
    (void)framebuffer;
    // Phase 3.
}

void VkProgram::printUniforms(std::ostream& outputStream)
{
    for (const auto& it : _uniformList)
    {
        outputStream << "Uniform: " << it.first;
        if (!it.second->typeString.empty())
            outputStream << " (" << it.second->typeString << ")";
        outputStream << "\n";
    }
}

void VkProgram::printAttributes(std::ostream& outputStream)
{
    for (const auto& it : _attributeList)
    {
        outputStream << "Attribute: " << it.first;
        if (!it.second->typeString.empty())
            outputStream << " (" << it.second->typeString << ")";
        outputStream << "\n";
    }
}

MATERIALX_NAMESPACE_END
