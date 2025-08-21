//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderVk/VkProgram.h>


MATERIALX_NAMESPACE_BEGIN

VkProgram::VkProgram() :
    _vkDevice(nullptr)
{

    glslang::InitializeProcess();
}

VkProgram::VkProgram(VulkanDevicePtr device) :
    _vkDevice(device)
{

    glslang::InitializeProcess();
}

VkProgram::~VkProgram()
{
    if (_vertexShaderModule)
        _vkDevice->GetDevice().destroyShaderModule(_vertexShaderModule);
    if (_fragmentShaderModule)
        _vkDevice->GetDevice().destroyShaderModule(_fragmentShaderModule);
    if (_pipeline)
    {
        _vkDevice->GetDevice().destroyPipeline(_pipeline);
    }
    if (_pipelineLayout)
    {
        _vkDevice->GetDevice().destroyPipelineLayout(_pipelineLayout);
    }

    glslang::FinalizeProcess();
}

void VkProgram::setVertexShader(const std::string& source)
{
    _vertexShaderSource = source;
}

void VkProgram::setFragmentShader(const std::string& source)
{
    _fragmentShaderSource = source;
}

bool VkProgram::GLSLtoSPV(const vk::ShaderStageFlagBits shaderType, std::string const& glslShader, std::vector<unsigned int>& spvShader)
{
    static bool glslangInitialized = false;
    if (!glslangInitialized)
    {
        glslang::InitializeProcess();
        glslangInitialized = true;
    }

    EShLanguage stage;
    switch (shaderType)
    {
        case vk::ShaderStageFlagBits::eVertex:
            stage = EShLangVertex;
            break;
        case vk::ShaderStageFlagBits::eFragment:
            stage = EShLangFragment;
            break;
        case vk::ShaderStageFlagBits::eCompute:
            stage = EShLangCompute;
            break;
        default:
            return false; 
    }

    const char* shaderStrings[1];
    shaderStrings[0] = glslShader.c_str();

    glslang::TShader shader(stage);
    shader.setStrings(shaderStrings, 1);

    int clientInputSemanticsVersion = 100; 
    glslang::EShTargetClientVersion clientVersion = glslang::EShTargetVulkan_1_2;
    glslang::EShTargetLanguageVersion targetVersion = glslang::EShTargetSpv_1_5;

    shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, clientInputSemanticsVersion);
    shader.setEnvClient(glslang::EShClientVulkan, clientVersion);
    shader.setEnvTarget(glslang::EShTargetSpv, targetVersion);

    TBuiltInResource Resources = {};
    Resources.maxLights = 32;
    Resources.maxClipPlanes = 6;
    Resources.maxTextureUnits = 32;
    Resources.maxTextureCoords = 32;
    Resources.maxVertexAttribs = 64;
    Resources.maxVertexUniformComponents = 4096;
    Resources.maxVaryingFloats = 64;
    Resources.maxVertexTextureImageUnits = 32;
    Resources.maxCombinedTextureImageUnits = 80;
    Resources.maxTextureImageUnits = 32;
    Resources.maxFragmentUniformComponents = 4096;
    Resources.maxDrawBuffers = 32;
    Resources.maxVertexUniformVectors = 128;
    Resources.maxVaryingVectors = 8;
    Resources.maxFragmentUniformVectors = 16;
    Resources.maxVertexOutputVectors = 16;
    Resources.maxFragmentInputVectors = 15;
    Resources.minProgramTexelOffset = -8;
    Resources.maxProgramTexelOffset = 7;
    Resources.maxClipDistances = 8;
    Resources.maxComputeWorkGroupCountX = 65535;
    Resources.maxComputeWorkGroupCountY = 65535;
    Resources.maxComputeWorkGroupCountZ = 65535;
    Resources.maxComputeWorkGroupSizeX = 1024;
    Resources.maxComputeWorkGroupSizeY = 1024;
    Resources.maxComputeWorkGroupSizeZ = 64;
    Resources.maxComputeUniformComponents = 1024;
    Resources.maxComputeTextureImageUnits = 16;
    Resources.maxComputeImageUniforms = 8;
    Resources.maxComputeAtomicCounters = 8;
    Resources.maxComputeAtomicCounterBuffers = 1;
    Resources.maxVaryingComponents = 60;
    Resources.maxVertexOutputComponents = 64;
    Resources.maxGeometryInputComponents = 64;
    Resources.maxGeometryOutputComponents = 128;
    Resources.maxFragmentInputComponents = 128;
    Resources.maxImageUnits = 8;
    Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
    Resources.maxCombinedShaderOutputResources = 8;
    Resources.maxImageSamples = 0;
    Resources.maxVertexImageUniforms = 0;
    Resources.maxTessControlImageUniforms = 0;
    Resources.maxTessEvaluationImageUniforms = 0;
    Resources.maxGeometryImageUniforms = 0;
    Resources.maxFragmentImageUniforms = 8;
    Resources.maxCombinedImageUniforms = 8;
    Resources.maxGeometryTextureImageUnits = 16;
    Resources.maxGeometryOutputVertices = 256;
    Resources.maxGeometryTotalOutputComponents = 1024;
    Resources.maxGeometryUniformComponents = 1024;
    Resources.maxGeometryVaryingComponents = 64;
    Resources.maxTessControlInputComponents = 128;
    Resources.maxTessControlOutputComponents = 128;
    Resources.maxTessControlTextureImageUnits = 16;
    Resources.maxTessControlUniformComponents = 1024;
    Resources.maxTessControlTotalOutputComponents = 4096;
    Resources.maxTessEvaluationInputComponents = 128;
    Resources.maxTessEvaluationOutputComponents = 128;
    Resources.maxTessEvaluationTextureImageUnits = 16;
    Resources.maxTessEvaluationUniformComponents = 1024;
    Resources.maxTessPatchComponents = 120;
    Resources.maxPatchVertices = 32;
    Resources.maxTessGenLevel = 64;
    Resources.maxViewports = 16;
    Resources.maxVertexAtomicCounters = 0;
    Resources.maxTessControlAtomicCounters = 0;
    Resources.maxTessEvaluationAtomicCounters = 0;
    Resources.maxGeometryAtomicCounters = 0;
    Resources.maxFragmentAtomicCounters = 8;
    Resources.maxCombinedAtomicCounters = 8;
    Resources.maxAtomicCounterBindings = 1;
    Resources.maxVertexAtomicCounterBuffers = 0;
    Resources.maxTessControlAtomicCounterBuffers = 0;
    Resources.maxTessEvaluationAtomicCounterBuffers = 0;
    Resources.maxGeometryAtomicCounterBuffers = 0;
    Resources.maxFragmentAtomicCounterBuffers = 1;
    Resources.maxCombinedAtomicCounterBuffers = 1;
    Resources.maxAtomicCounterBufferSize = 16384;
    Resources.maxTransformFeedbackBuffers = 4;
    Resources.maxTransformFeedbackInterleavedComponents = 64;
    Resources.maxCullDistances = 8;
    Resources.maxCombinedClipAndCullDistances = 8;
    Resources.maxSamples = 4;
    Resources.limits.nonInductiveForLoops = 1;
    Resources.limits.whileLoops = 1;
    Resources.limits.doWhileLoops = 1;
    Resources.limits.generalUniformIndexing = 1;
    Resources.limits.generalAttributeMatrixVectorIndexing = 1;
    Resources.limits.generalVaryingIndexing = 1;
    Resources.limits.generalSamplerIndexing = 1;
    Resources.limits.generalVariableIndexing = 1;
    Resources.limits.generalConstantMatrixVectorIndexing = 1;

    if (!shader.parse(&Resources, 100, false, EShMsgDefault))
    {
        printf("GLSL parsing failed:\n%s\n", shader.getInfoLog());
        return false;
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(EShMsgDefault))
    {
        printf("GLSL linking failed:\n%s\n", program.getInfoLog());
        return false;
    }

    glslang::SpvOptions spvOptions;
    spvOptions.generateDebugInfo = true;
    spvOptions.disableOptimizer = false;
    spvOptions.optimizeSize = false;

    glslang::GlslangToSpv(*program.getIntermediate(stage), spvShader, &spvOptions);

    return true;
}


bool VkProgram::buildProgram()
{
    try
    {
        if (!_vertexShaderSource.empty())
        {
            std::vector<unsigned int> shaderSPV;
            if (!GLSLtoSPV(vk::ShaderStageFlagBits::eVertex, _vertexShaderSource, shaderSPV))
            {
                throw std::runtime_error("Vertex shader compilation failed.");
            }
            _vertexShaderModule = createShaderModule(shaderSPV);
        }

        if (!_fragmentShaderSource.empty())
        {
            std::vector<unsigned int> shaderSPV;
            if (!GLSLtoSPV(vk::ShaderStageFlagBits::eFragment, _fragmentShaderSource, shaderSPV))
            {
                throw std::runtime_error("Fragment shader compilation failed.");
            }
            _fragmentShaderModule = createShaderModule(shaderSPV);
        }

        return _vertexShaderModule && _fragmentShaderModule;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Shader build failed: " << e.what() << std::endl;
        return false;
    }
}


vk::ShaderModule VkProgram::createShaderModule(const std::vector<unsigned int>& shaderSPV)
{
    vk::ShaderModuleCreateInfo createInfo(
        {},                                  
        shaderSPV.size() * sizeof(uint32_t), 
        shaderSPV.data()                     
    );

    return _vkDevice->GetDevice().createShaderModule(createInfo);
}

glm::mat4 VkProgram::convertMatrix44ToGlm(const Matrix44& mat)
{
    glm::mat4 glmMat;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            glmMat[row][col] = mat[row][col]; 
        }
    }
    return glmMat;
}

Matrix44 VkProgram::convertGlmToMatrix44(const glm::mat4& glmMat)
{
    Matrix44 mat;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            mat[row][col] = glmMat[row][col];
        }
    }
    return mat;
}

void VkProgram::bindViewInformation(CameraPtr camera)
{
    float scaleFactor = 2.5f;
    static int meshIndex = 0;
    glm::vec3 offset = glm::vec3(meshIndex * 20.0f, 2.0f, 2.0f); 
    float angleX = glm::radians(-45.0);
    float angleY = glm::radians(45.0);
    float angleZ = glm::radians(45.0);

    glm::mat4 modelMatrix = createTranslationMatrix(0.0f, 0.0f, 0.0f) *
                            createRotationMatrix(angleX, angleY, angleZ) *
                            createScaleMatrix(scaleFactor, scaleFactor, scaleFactor);
    ++meshIndex;

    glm::mat4 glmProj = convertMatrix44ToGlm(camera->getProjectionMatrix());
    glm::mat4 glmView = convertMatrix44ToGlm(camera->getViewMatrix());

    UniformBufferObject ubo;
    ubo.mvp = glmProj * glmView * modelMatrix;
    ubo.model = modelMatrix;
    ubo.lightDir = glm::normalize(glm::vec3(0.5f, 0.7f, 1.0f));
    ubo.padding = 0.0f;

    _uniformBuffer->updateData(&ubo, sizeof(ubo));
}

void VkProgram::updateTextureDescriptor(vk::ImageView view, vk::Sampler sampler)
{
    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = view;
    imageInfo.sampler = sampler;
    vk::WriteDescriptorSet write{ _descriptorSet, 1, 0,
                                  1, vk::DescriptorType::eCombinedImageSampler, &imageInfo, nullptr, nullptr };
    _vkDevice->GetDevice().updateDescriptorSets({ write }, nullptr);
}

bool VkProgram::build(const std::string& vertexShaderText,
                      const std::string& fragmentShaderText,
                      vk::RenderPass renderPass,
                      bool enableDepthTest,
                      vk::FrontFace frontFace)
{
    setVertexShader(vertexShaderText);
    setFragmentShader(fragmentShaderText);

    if (!buildProgram())
        return false;

    _uniformBuffer = VkUniformBuffer::create(_vkDevice);
    if (!_uniformBuffer->initialize(sizeof(UniformBufferObject)))
        return false;

    vk::DescriptorSetLayout descriptorSetLayout = _uniformBuffer->getDescriptorSetLayout();
    _descriptorSet = _uniformBuffer->getDescriptorSet();
    _pipelineLayout = _vkDevice->GetDevice().createPipelineLayout(
        vk::PipelineLayoutCreateInfo({}, 1, &descriptorSetLayout));


    // Create graphics pipeline
    _pipeline = createGraphicsPipeline(
        _vkDevice->getPipelineCache(),
        { _vertexShaderModule, nullptr },
        { _fragmentShaderModule, nullptr },
        sizeof(float) * 8, 
        {
            { vk::Format::eR32G32B32Sfloat, 0 },  // position offset
            { vk::Format::eR32G32B32Sfloat, 24 }, // normal offset
            { vk::Format::eR32G32Sfloat, 40 }     // uv offset
        },
        frontFace,
        enableDepthTest,
        _pipelineLayout,
        renderPass);

    return true;
}


glm::mat4 VkProgram::createScaleMatrix(float sx, float sy, float sz)
{
    glm::mat4 scaleMatrix(1.0f);
    scaleMatrix[0][0] = sx;
    scaleMatrix[1][1] = sy;
    scaleMatrix[2][2] = sz;
    return scaleMatrix;
}

glm::mat4 VkProgram::createRotationMatrix(float angleX, float angleY, float angleZ)
{
    float cx = cos(angleX), sx = sin(angleX);
    float cy = cos(angleY), sy = sin(angleY);
    float cz = cos(angleZ), sz = sin(angleZ);

    glm::mat4 rotX(1.0f);
    rotX[1][1] = cx;
    rotX[1][2] = -sx;
    rotX[2][1] = sx;
    rotX[2][2] = cx;

    glm::mat4 rotY(1.0f);
    rotY[0][0] = cy;
    rotY[0][2] = sy;
    rotY[2][0] = -sy;
    rotY[2][2] = cy;

    glm::mat4 rotZ(1.0f);
    rotZ[0][0] = cz;
    rotZ[0][1] = -sz;
    rotZ[1][0] = sz;
    rotZ[1][1] = cz;

    return rotZ * rotY * rotX; 
}

glm::mat4 VkProgram::createTranslationMatrix(float x, float y, float z)
{
    glm::mat4 translation(1.0f);
    translation[3][0] = x;
    translation[3][1] = y;
    translation[3][2] = z;
    return translation;
}

glm::mat4x4 VkProgram::lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{
    glm::vec3 f = glm::normalize(center - eye);      
    glm::vec3 r = glm::normalize(glm::cross(f, up)); 
    glm::vec3 u = glm::cross(r, f);                  

    glm::mat4x4 result(1.0f);
    result[0][0] = r.x;
    result[1][0] = r.y;
    result[2][0] = r.z;

    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;

    result[0][2] = -f.x;
    result[1][2] = -f.y;
    result[2][2] = -f.z;

    result[3][0] = -glm::dot(r, eye);
    result[3][1] = -glm::dot(u, eye);
    result[3][2] = glm::dot(f, eye);
    return result;
}

glm::mat4x4 VkProgram::perspective(float fovY, float aspect, float nearP, float farP)
{
    float tanHalfFovy = tan(fovY / 2.0f);

    glm::mat4x4 result(0.0f);
    result[0][0] = 1.0f / (aspect * tanHalfFovy);
    result[1][1] = 1.0f / (tanHalfFovy);
    result[2][2] = -(farP + nearP) / (farP - nearP);
    result[2][3] = -1.0f;
    result[3][2] = -(2.0f * farP * nearP) / (farP - nearP);

    return result;
}

glm::mat4x4 VkProgram::createModelViewProjectionClipMatrix(vk::Extent2D const& extent)
{
    {
        float fov = glm::radians(45.0f);
        if (extent.width > extent.height)
        {
            fov *= static_cast<float>(extent.height) / static_cast<float>(extent.width);
        }

        glm::mat4x4 model = glm::mat4x4(1.0f);
        glm::mat4x4 view = lookAt(glm::vec3(-5.0f, 3.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        float aspect = static_cast<float>(_surfaceExtent.width) / _surfaceExtent.height;
        glm::mat4x4 projection = perspective(fov, aspect, 0.1f, 100.0f);

      
        glm::mat4x4 clip = glm::mat4x4( 1.0f,  0.0f, 0.0f, 0.0f,
                                      0.0f, 1.0f, 0.0f, 0.0f,
                                      0.0f,  0.0f, 0.5f, 0.0f,
                                      0.0f,  0.0f, 0.5f, 1.0f );  
      
      return clip * projection * view * model;
    }
}


vk::Pipeline VkProgram::createGraphicsPipeline(
    vk::PipelineCache const& pipelineCache,
    std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& vertexShaderData,
    std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& fragmentShaderData,
    uint32_t vertexStride,
    std::vector<std::pair<vk::Format, uint32_t>> const& vertexInputAttributeFormatOffset,
    vk::FrontFace frontFace,
    bool depthBuffered,
    vk::PipelineLayout const& pipelineLayout,
    vk::RenderPass const& renderPass)
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStages = {
        vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, vertexShaderData.first, "main", vertexShaderData.second),
        vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, fragmentShaderData.first, "main", fragmentShaderData.second)
    };

    vk::VertexInputBindingDescription bindingDescription{
        0, vertexStride, vk::VertexInputRate::eVertex
    };

    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    attributeDescriptions.reserve(vertexInputAttributeFormatOffset.size());

    for (uint32_t i = 0; i < static_cast<uint32_t>(vertexInputAttributeFormatOffset.size()); ++i)
    {
        attributeDescriptions.emplace_back(
            i, 0,
            vertexInputAttributeFormatOffset[i].first,
            vertexInputAttributeFormatOffset[i].second);
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputState(
        {}, 1, &bindingDescription,
        static_cast<uint32_t>(attributeDescriptions.size()), attributeDescriptions.data());

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList);

    vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);

    vk::PipelineRasterizationStateCreateInfo rasterizer(
        {}, false, false, vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack, frontFace,
        false, 0, 0, 0, 1.0f);

    vk::PipelineMultisampleStateCreateInfo multisampling({}, vk::SampleCountFlagBits::e1);

    vk::PipelineDepthStencilStateCreateInfo depthStencil(
        {}, depthBuffered, depthBuffered,
        vk::CompareOp::eLessOrEqual, false, false);

    vk::PipelineColorBlendAttachmentState colorBlendAttachment(
        false, {}, {}, {}, {}, {}, {},
        vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA);

    vk::PipelineColorBlendStateCreateInfo colorBlending(
        {}, false, vk::LogicOp::eNoOp,
        1, &colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f });

    std::array<vk::DynamicState, 2> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

    vk::GraphicsPipelineCreateInfo pipelineInfo(
        {}, static_cast<uint32_t>(pipelineShaderStages.size()), pipelineShaderStages.data(),
        &vertexInputState, &inputAssembly,
        nullptr, &viewportState,
        &rasterizer, &multisampling,
        &depthStencil, &colorBlending,
        &dynamicState, pipelineLayout, renderPass);

    auto result = _vkDevice->GetDevice().createGraphicsPipeline(pipelineCache, pipelineInfo);
    assert(result.result == vk::Result::eSuccess);
    return result.value;
}




MATERIALX_NAMESPACE_END




