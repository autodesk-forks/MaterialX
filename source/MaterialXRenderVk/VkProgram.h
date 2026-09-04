//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKPROGRAM_H
#define MATERIALX_VKPROGRAM_H

/// @file
/// Vulkan program interfaces

#include <MaterialXRenderVk/Export.h>
#include <MaterialXRenderVk/VkContext.h>

#include <MaterialXRender/Camera.h>
#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/LightHandler.h>

#include <MaterialXGenShader/Shader.h>

#include <vulkan/vulkan.h>

#include <map>
#include <unordered_map>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

class VkFramebuffer;
using VkFramebufferPtr = std::shared_ptr<class VkFramebuffer>;

/// Shared pointer to a VkProgram
using VkProgramPtr = std::shared_ptr<class VkProgram>;

/// @class VkProgram
/// A class representing an executable Vulkan program.
///
/// Mirrors GlslProgram's public surface, with GL calls swapped for Vulkan ones.
/// The uniform/attribute introspection uses glslang reflection as the source of
/// truth for layout, decorated with MaterialX metadata from the Shader's
/// VariableBlocks — the same "introspect the API, then decorate from the Shader"
/// merge that GlslProgram::updateUniformsList() performs.
class MX_RENDERVK_API VkProgram
{
  public:
    /// Create a Vulkan program instance.
    static VkProgramPtr create(VkContextPtr context)
    {
        return VkProgramPtr(new VkProgram(context));
    }

    /// Destructor
    virtual ~VkProgram();

    /// @name Shader code setup
    /// @{

    /// Set up code stages to validate based on an input hardware shader.
    void setStages(ShaderPtr shader);

    /// Set the code stages based on a list of stage strings.
    void addStage(const string& stage, const string& sourceCode);

    /// Get source code string for a given stage.
    const string& getStageSourceCode(const string& stage) const;

    /// Clear all shader stages.
    /// NOTE: not on GlslProgram (only MslProgram/SlangProgram); added here because
    /// VkProgram, like those, needs an explicit stage reset.
    void clearStages();

    /// Return the shader, if any, used to generate this program.
    ShaderPtr getShader() const { return _shader; }

    /// @}
    /// @name Program building
    /// @{

    /// Build the Vulkan program: glslang compile of both stages into a single
    /// TProgram, link, buildReflection, GlslangToSpv, create two VkShaderModules,
    /// the descriptor set layout, pipeline layout, per-block UBOs, then seed
    /// uniform defaults. Throws ExceptionRenderError with the glslang log on failure.
    void build(const VkFramebufferPtr& framebuffer);

    /// Return true if built shader program data is present.
    bool hasBuiltData();

    /// Clear built shader program data, if any.
    void clearBuiltData();

    /// @}
    /// @name Program introspection
    /// @{

    /// Structure to hold information about program inputs.
    struct MX_RENDERVK_API Input
    {
        static int INVALID_VK_TYPE;

        /// Vertex attribute location, or -1.
        int location = -1;
        /// Descriptor binding for samplers/blocks, or -1.
        int binding = -1;
        /// Index into _blocks for UBO members, or -1.
        int blockIndex = -1;
        /// std140 byte offset within the block.
        int offset = -1;
        /// Element count (matches GlslProgram::Input::size).
        int size = 0;
        /// GL type enum from glslang reflection (e.g. 0x8b5c=mat4, 0x8b5e=sampler2D).
        /// Used for type dispatch since glslang's internal TType is not installed by vcpkg.
        int glType = INVALID_VK_TYPE;
        /// Whether this input is a sampler. NOTE: GlslProgram::Input has NO isSampler
        /// field — GL detects samplers at bind time by range-checking `gltype` against
        /// GL_SAMPLER_1D..GL_SAMPLER_CUBE (GlslProgram.cpp:547). VkProgram populates
        /// `isSampler` during updateUniformsList() from glslang reflection (glType in
        /// the GL_SAMPLER_* range).
        bool isSampler = false;
        /// Whether this input is a constant.
        bool isConstant = false;
        /// Input type string (from the Shader, if present).
        string typeString;
        /// Input value (from the Shader, if present).
        MaterialX::ConstValuePtr value;
        /// Element path (from the Shader, if present).
        string path;
        /// Unit (from the Shader, if present).
        string unit;
        /// Colorspace (from the Shader, if present).
        string colorspace;

        Input(int inputLocation, int inputBinding, int inputSize, const string& inputPath) :
            location(inputLocation),
            binding(inputBinding),
            size(inputSize),
            path(inputPath)
        { }
    };
    using InputPtr = std::shared_ptr<Input>;
    using InputMap = std::unordered_map<string, InputPtr>;

    /// Get list of program input uniforms.
    const InputMap& getUniformsList();

    /// Get list of program input attributes.
    const InputMap& getAttributesList();

    /// Find the inputs in the program which start with a given variable name.
    void findInputs(const string& variable, const InputMap& variableList,
                    InputMap& foundList, bool exactMatch);

    /// @}
    /// @name Program activation
    /// @{

    /// Bind the program: bind pipeline + descriptor set, flush dirty UBOs.
    /// Returns false if failed.
    bool bind(VkCommandBuffer cmd);

    /// Set the viewport dimensions (for dynamic viewport/scissor state).
    void setViewport(unsigned int width, unsigned int height)
    {
        _viewportWidth = width;
        _viewportHeight = height;
    }

    /// Return true if the program has active attributes.
    bool hasActiveAttributes() const;

    /// Return true if a uniform with the given name is present.
    bool hasUniform(const string& name);

    /// Bind a value to the uniform with the given name.
    void bindUniform(const string& name, ConstValuePtr value, bool errorIfMissing = true);

    /// Bind input geometry partition (indexing).
    void bindPartition(MeshPartitionPtr partition);

    /// Bind input geometry streams.
    void bindMesh(MeshPtr mesh);

    /// Draw a geometry partition. DIVERGENCE: not on GlslProgram (drawPartition lives
    /// on GlslMaterial/GlslRenderer there). Vulkan needs the command buffer at draw
    /// time, so it lives on the program here.
    void drawPartition(VkCommandBuffer cmd, MeshPartitionPtr partition);

    /// Unbind any bound geometry.
    void unbindGeometry();

    /// Bind any input textures.
    void bindTextures(ImageHandlerPtr imageHandler);

    /// Bind lighting.
    void bindLighting(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler);

    /// Bind view information.
    void bindViewInformation(CameraPtr camera);

    /// Bind time and frame.
    void bindTimeAndFrame(float time = 0.0f, float frame = 1.0f);

    /// Unbind the program.
    void unbind() const;

    /// @}
    /// @name Utilities
    /// @{

    /// Return whether alpha blending is enabled. DIVERGENCE: not on GlslProgram
    /// (transparency is detected in GlslRenderer via
    /// getShader()->hasAttribute(HW::ATTR_TRANSPARENT)). Kept for the viewer's pass logic.
    bool isTransparent() const { return _alphaBlendingEnabled; }

    /// Print all uniforms to the given stream.
    void printUniforms(std::ostream& outputStream);

    /// Print all attributes to the given stream.
    void printAttributes(std::ostream& outputStream);

    /// @}
    /// @name Test access
    /// @{

    /// A uniform block with its Vulkan buffer and host shadow.
    struct UniformBlock
    {
        string name;
        uint32_t binding = 0;
        size_t size = 0;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        void* mapped = nullptr;
        std::vector<uint8_t> shadow; // zero-initialized; seeded by bindUniformDefaults()
        bool dirty = true;
    };

    /// Return the uniform blocks (for test verification of UBO shadow contents).
    const std::vector<UniformBlock>& getUniformBlocks() const { return _blocks; }

    /// @}

  public:
    static unsigned int UNDEFINED_VK_RESOURCE_ID;
    static int UNDEFINED_VK_PROGRAM_LOCATION;

  protected:
    VkProgram(VkContextPtr context);

    // Update a list of program input uniforms (reflection first, then decorate
    // from the Shader's VariableBlocks, mirroring GlslProgram.cpp:876-1109).
    const InputMap& updateUniformsList();

    // Update a list of program input attributes.
    const InputMap& updateAttributesList();

    // Decorate the reflection-derived uniform list with MaterialX metadata
    // (path/unit/colorspace/value/typeString) from the Shader's VariableBlocks.
    void decorateFromShader();

    // Utility to find a uniform value in a uniform list.
    ConstValuePtr findUniformValue(const string& uniformName, const InputMap& uniformList);

    // Seed every UBO shadow buffer from the Shader's VariableBlocks.
    // MANDATORY under Vulkan: device memory is uninitialized, unlike D3D11 constant
    // buffers (zeroed) or GL default uniform values.
    void bindUniformDefaults();

    // Flush dirty UBO shadow buffers into mapped coherent memory.
    void flushUniforms();

    // Fill any sampler binding not yet written with ImageHandler::getZeroImage().
    // Sampling an unwritten Vulkan descriptor is undefined behaviour.
    void writeUnboundSamplersWithZeroImage(ImageHandlerPtr imageHandler);

    // Compile both stages to SPIR-V via glslang (single TProgram, link, reflect).
    // Populates _spirv and the reflection-derived uniform/attribute lists.
    bool compileToSpirv(StringVec& errors);

    // Create the descriptor set layout and pipeline layout from reflected bindings.
    void createDescriptorLayout();

    // Create per-block UBOs: host-visible + coherent VkBuffer + VkDeviceMemory,
    // persistently mapped, with a zero-initialized shadow buffer.
    void createUniformBuffers();

    // Create the graphics pipeline.
    void createPipeline(const VkFramebufferPtr& framebuffer);

    struct VertexBinding // one interleaved buffer per mesh
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        std::vector<string> attributeOrder;
        uint32_t stride = 0;
    };

    VkContextPtr _context;

    // Stages used to create program (map of stage name and its source code).
    StringMap _stages;

    // Hardware shader (if any) used for program creation.
    ShaderPtr _shader;

    // Compiled SPIR-V per stage.
    std::map<string, std::vector<uint32_t>> _spirv;

    VkShaderModule _vertexModule = VK_NULL_HANDLE;
    VkShaderModule _fragmentModule = VK_NULL_HANDLE;

    VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
    VkPipeline _pipeline = VK_NULL_HANDLE;

    InputMap _uniformList;
    InputMap _attributeList;

    std::vector<UniformBlock> _blocks;
    std::map<Mesh*, VertexBinding> _vertexBuffers;
    std::map<MeshPartitionPtr, std::pair<VkBuffer, VkDeviceMemory>> _indexBuffers;

    MeshPtr _boundMesh;
    bool _alphaBlendingEnabled = false;
    bool _built = false;

    // Viewport dimensions for dynamic viewport/scissor state.
    uint32_t _viewportWidth = 512;
    uint32_t _viewportHeight = 512;
};

MATERIALX_NAMESPACE_END

#endif
