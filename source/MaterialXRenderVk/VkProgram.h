//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKPROGRAM_H
#define MATERIALX_VKPROGRAM_H

/// @file

#include <vulkan/vulkan.hpp>
#include <MaterialXRenderVk/Export.h>

#include <MaterialXRender/Camera.h>
#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRenderVk/VkUniformBuffer.h>
#include <MaterialXGenShader/Shader.h>

#ifdef _USE_VULKAN_CPP
#include <MaterialXRenderVk/VkDevice.h>
#endif

MATERIALX_NAMESPACE_BEGIN

// Shared pointer to a VkProgram
using VkProgramPtr = std::shared_ptr<class VkProgram>;
using VkFrameBufferPtr = std::shared_ptr<class VkFrameBuffer>;

/// @class VkProgram
/// A class representing an executable Vulkan program.
///
/// There are two main interfaces which can be used.  One which takes in a HwShader and one which
/// allows for explicit setting of shader stage code.
class MX_RENDERVK_API VkProgram
{
  public:
    /// Create a Vulkan program instance
    static VkProgramPtr create(VulkanDevicePtr device)
    {
        return VkProgramPtr(new VkProgram(device));
    }

    explicit VkProgram(VulkanDevicePtr device);

    /// Destructor
    virtual ~VkProgram();

    /// Refactored Code
    void setVertexShader(const std::string& source);
    void setFragmentShader(const std::string& source);

    bool buildProgram(); // returns true if successful
    void bindViewInformation(CameraPtr camera);
    void updateTextureDescriptor(vk::ImageView view, vk::Sampler sampler);
    struct UniformBufferObject
    {
        glm::mat4 mvp;
        glm::mat4 model;
        glm::vec3 lightDir;
        float padding = 0.0f; 
    };
    struct VertexAttributeDesc
    {
        uint32_t location;
        vk::Format format;
        uint32_t offset;
    };

    vk::ShaderModule createShaderModule(const std::vector<unsigned int>& shaderSPV);

    glm::mat4 convertMatrix44ToGlm(const Matrix44& mat);

    Matrix44 convertGlmToMatrix44(const glm::mat4& glmMat);

    bool build(const std::string& vertexShaderText, const std::string& fragmentShaderText, vk::RenderPass renderPass, bool enableDepthTest, vk::FrontFace frontFace);

    glm::mat4 createScaleMatrix(float sx, float sy, float sz);

    glm::mat4 createRotationMatrix(float angleX, float angleY, float angleZ);

    glm::mat4 createTranslationMatrix(float x, float y, float z);

    glm::mat4x4 lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);

    glm::mat4x4 perspective(float fovY, float aspect, float nearP, float farP);

    glm::mat4x4 createModelViewProjectionClipMatrix(vk::Extent2D const& extent);

    vk::Pipeline createGraphicsPipeline(vk::PipelineCache const& pipelineCache, std::vector<std::tuple<vk::ShaderStageFlagBits, vk::ShaderModule, vk::SpecializationInfo const*>> const& shaderStagesData, uint32_t vertexStride, std::vector<VertexAttributeDesc> const& vertexAttributes, vk::FrontFace frontFace, bool depthBuffered, vk::PipelineLayout const& pipelineLayout, vk::RenderPass const& renderPass);

    vk::Pipeline createGraphicsPipeline(vk::PipelineCache const& pipelineCache, std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& vertexShaderData, std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& fragmentShaderData, uint32_t vertexStride, std::vector<std::pair<vk::Format, uint32_t>> const& vertexInputAttributeFormatOffset, vk::FrontFace frontFace, bool depthBuffered, vk::PipelineLayout const& pipelineLayout, vk::RenderPass const& renderPass);
    vk::Pipeline VkProgram::getPipeline() const
    {
        return _pipeline;
    }

    vk::PipelineLayout VkProgram::getPipelineLayout() const
    {
        return _pipelineLayout;
    }

    vk::DescriptorSet getDescriptorSet() const
    {
        return _descriptorSet;
    }

    VkUniformBufferPtr getUniformBuffer() const { return _uniformBuffer; }
  
    vk::ShaderModule getVertexShaderModule() const { return _vertexShaderModule; }
    vk::ShaderModule getFragmentShaderModule() const { return _fragmentShaderModule; }
    bool GLSLtoSPV(const vk::ShaderStageFlagBits shaderType, std::string const& glslShader, std::vector<unsigned int>& spvShader);

  private:
    VulkanDevicePtr _vkDevice;
    vk::ShaderModule _vertexShaderModule = nullptr;
    vk::ShaderModule _fragmentShaderModule = nullptr;
    VkUniformBufferPtr _uniformBuffer;

    std::string _vertexShaderSource;
    std::string _fragmentShaderSource;

    vk::Pipeline _pipeline;
    vk::PipelineLayout _pipelineLayout;
    vk::DescriptorSet _descriptorSet;
   
    vk::Extent2D _surfaceExtent;
















    /// @name Shader code setup
    /// @{

    /// Set up code stages to validate based on an input hardware shader.
    /// @param shader Hardware shader to use
    void setStages(ShaderPtr shader);

    /// Set the code stages based on a list of stage strings.
    /// Refer to the ordering of stages as defined by a HwShader.
    /// @param stage Name of the shader stage.
    /// @param sourceCode Source code of the shader stage.
    void addStage(const string& stage, const string& sourceCode);

    /// Get source code string for a given stage.
    /// @return Shader stage string. String is empty if not found.
    const string& getStageSourceCode(const string& stage) const;

    /// Clear out any existing stages
    void clearStages();

    /// Return the shader, if any, used to generate this program.
    ShaderPtr getShader() const
    {
        return _shader;
    }

    /// @}
    /// @name Program validation and introspection
    /// @{ bbtr frameBuffer);

    /// Structure to hold information about program inputs.
    /// The structure is populated by directly scanning the program so may not contain
    /// some inputs listed on any associated HwShader as those inputs may have been
    /// optimized out if they are unused.
    struct MX_RENDERVK_API Input
    {
        static int INVALID_METAL_TYPE;

        /// Program location. -1 means an invalid location
        int location;
        /// Metal type of the input. -1 means an invalid type
        int resourceType;
        /// Size.
        int size;
        /// Input type string. Will only be non-empty if initialized stages with a HwShader
        string typeString;
        /// Input value. Will only be non-empty if initialized stages with a HwShader and a value was set during
        /// shader generation.
        MaterialX::ValuePtr value;
        /// Is this a constant
        bool isConstant;
        /// Element path (if any)
        string path;
        /// Unit
        string unit;
        /// Colorspace 
        string colorspace;

        /// Program input constructor
        Input(int inputLocation, int inputType, int inputSize, const string& inputPath) :
            location(inputLocation),
            resourceType(inputType),
            size(inputSize),
            isConstant(false),
            path(inputPath)
        { }
    };
    /// Program input structure shared pointer type
    using InputPtr = std::shared_ptr<Input>;
    /// Program input shaded pointer map type
    using InputMap = std::unordered_map<string, InputPtr>;

    /// Get list of program input uniforms.
    /// The program must have been created successfully first.
    /// An exception is thrown if the parsing of the program for uniforms cannot be performed.
    /// @return Program uniforms list.
    const InputMap& getUniformsList();

    /// Get list of program input attributes.
    /// The program must have been created successfully first.
    /// An exception is thrown if the parsing of the program for attribute cannot be performed.
    /// @return Program attributes list.
    const InputMap& getAttributesList();

    /// Find the locations in the program which starts with a given variable name
    /// @param variable Variable to search for
    /// @param variableList List of program inputs to search
    /// @param foundList Returned list of found program inputs. Empty if none found.
    /// @param exactMatch Search for exact variable name match.
    void findInputs(const string& variable,
                    const InputMap& variableList,
                    InputMap& foundList,
                    bool exactMatch);

    /// @}
    /// @name Program activation
    /// @{

    /// Bind the pipeline state object to the command encoder.
    /// @param renderCmdEncoder encoder that binds the pipeline state object.
    /// @return False if failed
    // bool bind(id<MTLRenderCommandEncoder> renderCmdEncoder);
    
    /// Bind inputs
    ///  @param renderCmdEncoder encoder that inputs will be bound to.
    ///  @param cam Camera object use to view the object
    ///  @param geometryHandler
    ///  @param imageHandler
    ///  @param lightHandler
    ///  @return void - No return value
    /* void prepareUsedResources(id<MTLRenderCommandEncoder> renderCmdEncoder,
                        CameraPtr cam,
                        GeometryHandlerPtr geometryHandler,
                        ImageHandlerPtr imageHandler,
                        LightHandlerPtr lightHandler);
    */
    /// Return true if a uniform with the given name is present.
    bool hasUniform(const string& name);

    /// Bind a value to the uniform with the given name.
    void bindUniform(const string& name, ConstValuePtr value, bool errorIfMissing = true);

    /// Bind attribute buffers to attribute inputs.
    /// A hardware buffer of the given attribute type is created and bound to the program locations
    /// for the input attribute.
    /// @param renderCmdEncoder Metal Render Command Encoder that the attribute being bind to
    /// @param inputs Attribute inputs to bind to
    /// @param mesh Mesh containing streams to bind
    /*
    void bindAttribute(id<MTLRenderCommandEncoder> renderCmdEncoder,
                       const VkProgram::InputMap& inputs,
                       MeshPtr mesh);
   */
    /// Bind input geometry partition (indexing)
    void bindPartition(MeshPartitionPtr partition);

    /// Bind input geometry streams
    // void bindMesh(id<MTLRenderCommandEncoder> renderCmdEncoder, MeshPtr mesh);
    
    /// Queries the index buffer assinged to a mesh partition
    /* 
    id<MTLBuffer> getIndexBuffer(MeshPartitionPtr mesh) {
        if(_indexBufferIds.find(mesh) != _indexBufferIds.end())
            return _indexBufferIds[mesh];
        return nil;
    }
    */

    /// Unbind any bound geometry
    void unbindGeometry();

    /// Bind any input textures
    /*
    void bindTextures(id<MTLRenderCommandEncoder> renderCmdEncoder,
                      LightHandlerPtr lightHandler,
                      ImageHandlerPtr imageHandler);
    */
    void bindTexture(ImageHandlerPtr imageHandler,
                     string shaderTextureName,
                     ImagePtr imagePtr,
                     ImageSamplingProperties samplingProperties);

    /// Bind lighting
    void bindLighting(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler);

    /// Bind view information
    
    /// Bind time and frame
    void bindTimeAndFrame(float time = 1.0f, float frame = 1.0f);

    /// @}
    /// @name Utilities
    /// @{
    
    /// Returns if alpha blending is enabled.
    bool isTransparent() const { return _alphaBlendingEnabled; }
    
    /// Specify textures bound to this program shouldn't be mip mapped.
    void setEnableMipMaps(bool enableMipMapping) { _enableMipMapping = enableMipMapping; }

    /// Print all uniforms to the given stream.
    void printUniforms(std::ostream& outputStream);

    /// Print all attributes to the given stream.
    void printAttributes(std::ostream& outputStream);

    /// @}

  protected:
    VkProgram();

    // Update a list of program input uniforms
    const InputMap& updateUniformList();

    // Update a list of program input attributes
    const InputMap& updateAttributesList();

    // Clear out any cached input lists
    void clearInputLists();

    // Utility to find a uniform value in an uniform list.
    // If uniform cannot be found a null pointer will be return.
    ValuePtr findUniformValue(const string& uniformName, const InputMap& uniformList);

    // // Bind an individual texture to a program uniform location
    // ImagePtr bindTexture(id<MTLRenderCommandEncoder> renderCmdEncoder,
    //                      unsigned int uniformLocation,
    //                      const FilePath& filePath,
    //                      ImageSamplingProperties samplingProperties,
    //                      ImageHandlerPtr imageHandler);
    
    // // Bind an individual texture to a program uniform location
    // ImagePtr bindTexture(id<MTLRenderCommandEncoder> renderCmdEncoder,
    //                      unsigned int uniformLocation,
    //                      ImagePtr imagePtr,
    //                      ImageHandlerPtr imageHandler);
        
    // void bindUniformBuffers(id<MTLRenderCommandEncoder> renderCmdEncoder,
    //                         LightHandlerPtr lightHandler,
    //                         CameraPtr camera);

    // Delete any currently created pso
    void reset();

    // Utility to map a MaterialX type to an METAL type
    // static MTLDataType mapTypeToMetalType(const TypeDesc* type);

  private:
    // Stages used to create program
    // Map of stage name and its source code
    StringMap _stages;

    // // Generated pipeline state object. A non-zero number indicates a valid shader program.
    // id<MTLRenderPipelineState> _pso = nil;
    // MTLRenderPipelineReflection* _psoReflection = nil;

    // List of program input uniforms
    InputMap _uniformList;
    std::unordered_map<std::string, std::string> _globalUniformNameList;
    // List of program input attributes
    InputMap _attributeList;
    
    std::unordered_map<std::string, ImagePtr> _explicitBoundImages;

    // Hardware shader (if any) used for program creation
    ShaderPtr _shader;

    // Attribute buffer resource handles
    // for each attribute identifier in the program
    /*std::unordered_map<string, id<MTLBuffer>> _attributeBufferIds;*/

    // Attribute indexing buffer handle
    /*std::map<MeshPartitionPtr, id<MTLBuffer>> _indexBufferIds;*/

    // Program texture map
    std::unordered_map<string, unsigned int> _programTextures;
    
    // Metal Device Object
    /*id<MTLDevice> _device = nil;*/
    
    // Currently bound mesh
    MeshPtr _boundMesh = nullptr;

    bool _alphaBlendingEnabled = false;
    
    float _time = 0.0f;
    float _frame = 0.0f;
    
    bool _enableMipMapping = true;
};

MATERIALX_NAMESPACE_END

#endif
