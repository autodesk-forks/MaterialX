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
// Captured reflection data for a single uniform variable.
struct ReflectedUniform
{
    std::string name;
    int glType = 0;
    int offset = -1;
    int binding = -1;
    int blockIndex = -1;
    int size = 0;
    int arrayStride = 0;
};

// Captured reflection data for a uniform block.
struct ReflectedBlock
{
    std::string name;
    int binding = 0;
    int size = 0;
    int numMembers = 0;
};

// Captured reflection data for a pipe input (vertex attribute).
struct ReflectedPipeInput
{
    std::string name;
    int glType = 0;
    int location = -1;
};

// GL type-enum constants used for sampler detection and type dispatch.
// (These are the GL_ACTIVE_UNIFORM enum values glslang reports via glDefineType.)
constexpr int GL_FLOAT = 0x1406;
constexpr int GL_INT = 0x1404;
constexpr int GL_BOOL = 0x8B56;
constexpr int GL_FLOAT_VEC2 = 0x8B50;
constexpr int GL_FLOAT_VEC3 = 0x8B51;
constexpr int GL_FLOAT_VEC4 = 0x8B52;
constexpr int GL_FLOAT_MAT3 = 0x8B5B;
constexpr int GL_FLOAT_MAT4 = 0x8B5C;
constexpr int GL_SAMPLER_2D = 0x8B5E;
constexpr int GL_SAMPLER_CUBE = 0x8B60;

// True if a GL type enum is a sampler (combined image sampler).
bool isSamplerType(int glType)
{
    return glType == GL_SAMPLER_2D || glType == GL_SAMPLER_CUBE ||
           glType == 0x9055 /*GL_SAMPLER_2D_ARRAY*/ || glType == 0x8DC1 /*GL_SAMPLER_2D_RECT*/;
}

// Compile both stages into a single glslang TProgram, link, reflect, and emit SPIR-V.
// This fixes the old branch's per-stage TProgram mistake: a fresh TProgram per stage
// prevents cross-stage reflection with the unified binding numbering the generator emits.
bool compileToSpvInternal(const string& vertexSource, const string& fragmentSource,
                          std::vector<uint32_t>& vertexSpv, std::vector<uint32_t>& fragmentSpv,
                          std::vector<ReflectedBlock>& blocks,
                          std::vector<ReflectedUniform>& uniforms,
                          std::vector<ReflectedPipeInput>& pipeInputs,
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

    // Capture uniform blocks. (glslang 16's reflection API is not per-stage —
    // these aggregate across all linked stages.)
    int numBlocks = program.getNumUniformBlocks();
    for (int i = 0; i < numBlocks; i++)
    {
        const glslang::TObjectReflection& block = program.getUniformBlock(i);
        ReflectedBlock rb;
        rb.name = block.name;
        rb.binding = block.getBinding();
        rb.size = block.size;
        rb.numMembers = block.numMembers;
        blocks.push_back(std::move(rb));
    }

    // Capture uniform variables (block members + loose samplers).
    int numUniforms = program.getNumUniformVariables();
    for (int i = 0; i < numUniforms; i++)
    {
        const glslang::TObjectReflection& u = program.getUniform(i);
        ReflectedUniform ru;
        ru.name = u.name;
        ru.glType = u.glDefineType;
        ru.offset = u.offset;
        ru.binding = u.getBinding();
        ru.blockIndex = u.index; // owning block index, -1 if loose
        ru.size = u.size;
        ru.arrayStride = u.arrayStride;
        uniforms.push_back(std::move(ru));
    }

    // Capture pipe inputs (vertex attributes).
    int numInputs = program.getNumPipeInputs();
    for (int i = 0; i < numInputs; i++)
    {
        const glslang::TObjectReflection& p = program.getPipeInput(i);
        ReflectedPipeInput rpi;
        rpi.name = p.name;
        rpi.glType = p.glDefineType;
        rpi.location = static_cast<int>(p.layoutLocation());
        pipeInputs.push_back(std::move(rpi));
    }

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
    std::vector<ReflectedBlock> blocks;
    std::vector<ReflectedUniform> uniforms;
    std::vector<ReflectedPipeInput> pipeInputs;
    if (!compileToSpvInternal(vsIt->second, fsIt->second, vertexSpv, fragmentSpv,
                              blocks, uniforms, pipeInputs, errors))
    {
        return false;
    }

    _spirv[Stage::VERTEX] = std::move(vertexSpv);
    _spirv[Stage::PIXEL] = std::move(fragmentSpv);

    // Populate UBO blocks from reflection.
    _blocks.clear();
    for (const auto& rb : blocks)
    {
        UniformBlock block;
        block.name = rb.name;
        block.binding = static_cast<uint32_t>(rb.binding);
        block.size = static_cast<size_t>(rb.size);
        block.shadow.resize(rb.size, 0);
        _blocks.push_back(std::move(block));
    }

    // Populate the uniform list from reflection, then decorate from the Shader.
    _uniformList.clear();
    for (const auto& ru : uniforms)
    {
        // Skip the unindexed light-data aliases (u_lightData.direction) — keep only
        // the indexed forms (u_lightData[0].direction) which is what bindLighting uses.
        // glslang reflects both; the unindexed form duplicates [0]'s offset.
        if (ru.blockIndex >= 0 && ru.name.find('[') == string::npos)
        {
            // This is a block member without an index. Keep it only if no indexed
            // variant exists (i.e. it's not an array-of-struct member).
            // Heuristic: LightData members come in both forms; skip the bare form
            // when the name starts with "u_lightData.".
            if (ru.name.rfind("u_lightData.", 0) == 0)
                continue;
        }

        auto input = std::make_shared<Input>(
            UNDEFINED_VK_PROGRAM_LOCATION,
            ru.binding,
            ru.size > 0 ? ru.size : 1,
            EMPTY_STRING);
        input->glType = ru.glType;
        input->offset = ru.offset;
        input->blockIndex = ru.blockIndex;
        input->binding = ru.binding;
        input->isSampler = isSamplerType(ru.glType);
        _uniformList[ru.name] = input;
    }

    // Decorate from the Shader's VariableBlocks (path/unit/colorspace/value/typeString).
    decorateFromShader();

    // Populate the attribute list from reflection.
    _attributeList.clear();
    for (const auto& rpi : pipeInputs)
    {
        auto input = std::make_shared<Input>(
            rpi.location,
            UNDEFINED_VK_PROGRAM_LOCATION,
            1,
            EMPTY_STRING);
        input->glType = rpi.glType;
        _attributeList[rpi.name] = input;
    }

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

    // Create the descriptor set layout + pipeline layout from reflected bindings.
    createDescriptorLayout();

    // Create per-block UBOs (host-visible + coherent, mapped, with shadow buffers).
    createUniformBuffers();

    // Seed every UBO shadow from the Shader's VariableBlock defaults (trap #1).
    bindUniformDefaults();

    // Phase 3 will add: createPipeline(framebuffer).
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
    // Reflection populates _uniformList during compileToSpirv(). If we haven't
    // compiled yet but have a Shader, fall back to Shader-only decoration.
    if (_uniformList.empty() && _shader)
    {
        StringVec errors;
        if (!compileToSpirv(errors))
        {
            throw ExceptionRenderError("Failed to compile Vulkan shader for introspection", errors);
        }
    }
    return _uniformList;
}

const VkProgram::InputMap& VkProgram::updateAttributesList()
{
    if (_attributeList.empty() && _shader)
    {
        StringVec errors;
        if (!compileToSpirv(errors))
        {
            throw ExceptionRenderError("Failed to compile Vulkan shader for introspection", errors);
        }
    }
    return _attributeList;
}

void VkProgram::decorateFromShader()
{
    if (!_shader)
        return;

    // Build a lookup of Shader ports by variable name so we can decorate the
    // reflection-derived uniform list with MaterialX metadata.
    std::unordered_map<string, const ShaderPort*> portByName;
    for (size_t i = 0; i < _shader->numStages(); i++)
    {
        const ShaderStage& stage = _shader->getStage(i);
        for (const auto& blockPair : stage.getUniformBlocks())
        {
            const VariableBlock& block = *blockPair.second;
            for (size_t j = 0; j < block.size(); j++)
            {
                const ShaderPort* port = block[j];
                if (port)
                    portByName[port->getVariable()] = port;
            }
        }
    }

    // Decorate each reflected uniform with metadata from the matching Shader port.
    // The reflected name may be a composed form like "u_lightData[0].direction";
    // match the base variable (before '[' or '.') to the Shader port.
    for (auto& kv : _uniformList)
    {
        const string& name = kv.first;
        Input& input = *kv.second;

        // Extract the base variable name: everything before '[' or '.'.
        string baseName = name;
        size_t cut = name.find_first_of("[.");
        if (cut != string::npos)
            baseName = name.substr(0, cut);

        auto it = portByName.find(baseName);
        if (it != portByName.end())
        {
            const ShaderPort* port = it->second;
            input.typeString = port->getType().getName();
            if (!port->getValue())
                input.value = port->getValue();
            input.unit = port->getUnit();
            input.colorspace = port->getColorSpace();
            if (input.path.empty())
                input.path = port->getPath();
        }
    }
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

    Input& input = *it->second;
    input.value = value;

    // Samplers have no UBO backing — their binding is written via descriptor writes
    // in bindTextures()/bindLighting(), not here.
    if (input.isSampler || input.blockIndex < 0 || input.offset < 0)
        return;

    if (input.blockIndex >= static_cast<int>(_blocks.size()))
    {
        if (errorIfMissing)
            throw ExceptionRenderError("Uniform block index out of range for: " + name);
        return;
    }

    UniformBlock& block = _blocks[input.blockIndex];
    std::vector<uint8_t>& shadow = block.shadow;
    size_t offset = static_cast<size_t>(input.offset);

    if (!value)
        return;

    // Write the value into the shadow buffer at the std140 offset, with type-specific
    // layout. The mat3 case (trap #2) is the critical one: std140 lays out mat3 as
    // 3 columns of vec4 (48 bytes), but Matrix33::data() is 9 tightly-packed floats
    // (36 bytes). A naive memcpy would corrupt everything after it in the block.
    const string& typeStr = value->getTypeString();
    if (typeStr == "matrix33" || typeStr == "Matrix33")
    {
        Matrix33 m = value->asA<Matrix33>();
        const float* data = m.data();
        // Write 3 rows, each padded to 16 bytes (vec4) in std140.
        for (int row = 0; row < 3; row++)
        {
            if (offset + row * 16 + 12 > shadow.size())
                break;
            std::memcpy(&shadow[offset + row * 16], &data[row * 3], sizeof(float) * 3);
        }
    }
    else if (typeStr == "matrix44" || typeStr == "Matrix44")
    {
        Matrix44 m = value->asA<Matrix44>();
        if (offset + 64 <= shadow.size())
            std::memcpy(&shadow[offset], m.data(), 64);
    }
    else if (typeStr == "color3" || typeStr == "Color3")
    {
        Color3 v = value->asA<Color3>();
        if (offset + 12 <= shadow.size())
            std::memcpy(&shadow[offset], v.data(), 12);
    }
    else if (typeStr == "color4" || typeStr == "Color4")
    {
        Color4 v = value->asA<Color4>();
        if (offset + 16 <= shadow.size())
            std::memcpy(&shadow[offset], v.data(), 16);
    }
    else if (typeStr == "vector3" || typeStr == "Vector3")
    {
        Vector3 v = value->asA<Vector3>();
        if (offset + 12 <= shadow.size())
            std::memcpy(&shadow[offset], v.data(), 12);
    }
    else if (typeStr == "vector4" || typeStr == "Vector4")
    {
        Vector4 v = value->asA<Vector4>();
        if (offset + 16 <= shadow.size())
            std::memcpy(&shadow[offset], v.data(), 16);
    }
    else if (typeStr == "vector2" || typeStr == "Vector2")
    {
        Vector2 v = value->asA<Vector2>();
        if (offset + 8 <= shadow.size())
            std::memcpy(&shadow[offset], v.data(), 8);
    }
    else if (typeStr == "float")
    {
        float v = value->asA<float>();
        if (offset + 4 <= shadow.size())
            std::memcpy(&shadow[offset], &v, 4);
    }
    else if (typeStr == "integer" || typeStr == "int")
    {
        int v = value->asA<int>();
        if (offset + 4 <= shadow.size())
            std::memcpy(&shadow[offset], &v, 4);
    }
    else if (typeStr == "boolean" || typeStr == "bool")
    {
        bool v = value->asA<bool>();
        int b = v ? 1 : 0;
        if (offset + 4 <= shadow.size())
            std::memcpy(&shadow[offset], &b, 4);
    }

    block.dirty = true;
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
    // Seed every UBO shadow buffer from the Shader's VariableBlock defaults.
    // MANDATORY under Vulkan: device memory is uninitialized at allocation,
    // unlike D3D11 constant buffers (zeroed) or GL default uniform values.
    // Without this, materials render as garbage, not black.
    if (!_shader)
        return;

    // Build a lookup of Shader ports by variable name.
    std::unordered_map<string, const ShaderPort*> portByName;
    for (size_t i = 0; i < _shader->numStages(); i++)
    {
        const ShaderStage& stage = _shader->getStage(i);
        for (const auto& blockPair : stage.getUniformBlocks())
        {
            const VariableBlock& block = *blockPair.second;
            for (size_t j = 0; j < block.size(); j++)
            {
                const ShaderPort* port = block[j];
                if (port)
                    portByName[port->getVariable()] = port;
            }
        }
    }

    // For each reflected uniform that has a default value in the Shader, write it
    // into the shadow buffer (non-throwing — missing uniforms are skipped).
    for (auto& kv : _uniformList)
    {
        const string& name = kv.first;
        Input& input = *kv.second;
        if (input.isSampler || input.blockIndex < 0 || input.offset < 0)
            continue;

        // Match the base variable name (before '[' or '.').
        string baseName = name;
        size_t cut = name.find_first_of("[.");
        if (cut != string::npos)
            baseName = name.substr(0, cut);

        auto it = portByName.find(baseName);
        if (it == portByName.end())
            continue;

        const ShaderPort* port = it->second;
        if (!port->getValue())
            continue;

        // Write the default value into the shadow (suppress errors for missing).
        bindUniform(name, port->getValue(), false);
    }

    // Flush the seeded defaults into the mapped UBO memory.
    flushUniforms();
}

void VkProgram::flushUniforms()
{
    // memcpy dirty shadow buffers into mapped coherent memory.
    for (auto& block : _blocks)
    {
        if (!block.dirty || !block.mapped || block.shadow.empty())
            continue;
        std::memcpy(block.mapped, block.shadow.data(), block.shadow.size());
        block.dirty = false;
    }
}

void VkProgram::writeUnboundSamplersWithZeroImage(ImageHandlerPtr imageHandler)
{
    (void)imageHandler;
    // Phase 3.
}

void VkProgram::createDescriptorLayout()
{
    // Build one descriptor set layout (set 0) with:
    //  - one UBO binding per reflected block (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
    //  - one sampler binding per reflected sampler (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER).
    // All bindings use vk::ShaderStageFlagBits::eAllGraphics since the generator emits
    // unique bindings across both stages with no set= qualifier (Background B).
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    // UBO bindings.
    for (const auto& block : _blocks)
    {
        VkDescriptorSetLayoutBinding uboBinding{};
        uboBinding.binding = block.binding;
        uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboBinding.descriptorCount = 1;
        uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        uboBinding.pImmutableSamplers = nullptr;
        bindings.push_back(uboBinding);
    }

    // Sampler bindings (loose uniforms with blockIndex == -1 and isSampler).
    for (const auto& kv : _uniformList)
    {
        const Input& input = *kv.second;
        if (!input.isSampler || input.binding < 0)
            continue;
        VkDescriptorSetLayoutBinding samplerBinding{};
        samplerBinding.binding = static_cast<uint32_t>(input.binding);
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.descriptorCount = 1;
        samplerBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerBinding.pImmutableSamplers = nullptr;
        bindings.push_back(samplerBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(_context->getDevice(), &layoutInfo, nullptr,
                                         &_descriptorSetLayout),
             "Failed to create Vulkan descriptor set layout");

    // Create a descriptor pool large enough for the UBOs + samplers.
    uint32_t uboCount = static_cast<uint32_t>(_blocks.size());
    uint32_t samplerCount = 0;
    for (const auto& kv : _uniformList)
        if (kv.second->isSampler)
            samplerCount++;

    std::vector<VkDescriptorPoolSize> poolSizes;
    if (uboCount > 0)
    {
        VkDescriptorPoolSize uboPoolSize{};
        uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboPoolSize.descriptorCount = uboCount;
        poolSizes.push_back(uboPoolSize);
    }
    if (samplerCount > 0)
    {
        VkDescriptorPoolSize samplerPoolSize{};
        samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerPoolSize.descriptorCount = samplerCount;
        poolSizes.push_back(samplerPoolSize);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VK_CHECK(vkCreateDescriptorPool(_context->getDevice(), &poolInfo, nullptr, &_descriptorPool),
             "Failed to create Vulkan descriptor pool");

    // Allocate the descriptor set.
    VkDescriptorSetLayout layouts[] = { _descriptorSetLayout };
    VkDescriptorSetAllocateInfo setAllocInfo{};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = _descriptorPool;
    setAllocInfo.descriptorSetCount = 1;
    setAllocInfo.pSetLayouts = layouts;

    VK_CHECK(vkAllocateDescriptorSets(_context->getDevice(), &setAllocInfo, &_descriptorSet),
             "Failed to allocate Vulkan descriptor set");

    // Create the pipeline layout (empty push-constant range for now).
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    VK_CHECK(vkCreatePipelineLayout(_context->getDevice(), &pipelineLayoutInfo, nullptr,
                                    &_pipelineLayout),
             "Failed to create Vulkan pipeline layout");
}

void VkProgram::createUniformBuffers()
{
    VkDevice device = _context->getDevice();

    for (auto& block : _blocks)
    {
        if (block.size == 0)
            continue;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = block.size;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &block.buffer),
                 "Failed to create Vulkan UBO buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, block.buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = _context->findMemoryType(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (allocInfo.memoryTypeIndex == UINT32_MAX)
        {
            throw ExceptionRenderError("Failed to find host-visible memory type for UBO");
        }

        VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &block.memory),
                 "Failed to allocate Vulkan UBO memory");

        vkBindBufferMemory(device, block.buffer, block.memory, 0);

        // Persistently map the UBO.
        VK_CHECK(vkMapMemory(device, block.memory, 0, block.size, 0, &block.mapped),
                 "Failed to map Vulkan UBO memory");

        // Zero the shadow + mapped memory so uninitialized uniforms are deterministic.
        std::fill(block.shadow.begin(), block.shadow.end(), 0);
        std::memset(block.mapped, 0, block.size);
        block.dirty = true;
    }
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
