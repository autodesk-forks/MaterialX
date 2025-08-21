//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKRENDERER_H
#define MATERIALX_VKRENDERER_H

#include <vulkan/vulkan.hpp>
#include <MaterialXRenderVk/Export.h>
#include <MaterialXRenderVk/VkProgram.h>

#include <MaterialXRenderVk/VkUniformBuffer.h>
#include <MaterialXRender/ShaderRenderer.h>

MATERIALX_NAMESPACE_BEGIN

using SimpleWindowPtr = std::shared_ptr<class SimpleWindow>;

/// Shared pointer to a VkRenderer
using VkRendererPtr = std::shared_ptr<class VkRenderer>;

/// @class VkRenderer
/// Helper class for rendering generated Glsl Vulkan code to produce images.
///
/// There are two main interfaces which can be used. One which takes in a HwShader and one which
/// allows for explicit setting of shader stage code.
///
/// The main services provided are:
///     - Validation: All shader stages are compiled and atteched to a Glsl Vulkan shader program.
///     - Introspection: The compiled shader program is examined for uniforms and attributes.
///     - Binding: Uniforms and attributes which match the predefined variables generated the Glsl Vulkan code generator
///       will have values assigned to this. This includes matrices, attribute streams, and textures.
///     - Rendering: The program with bound inputs will be used to drawing geometry to an offscreen buffer.
///     An interface is provided to save this offscreen buffer to disk using an externally defined image handler.
///
class MX_RENDERVK_API VkRenderer : public ShaderRenderer
{
  public:
    /// Create a Glsl Vulkan renderer instance
    static VkRendererPtr create(unsigned int width = 512, unsigned int height = 512, Image::BaseType baseType = Image::BaseType::UINT8);
    
    
    /// Returns Metal Device used for rendering
    // id<MTLDevice> getMetalDevice() const;

    /// Destructor
    virtual ~VkRenderer() { }

    /// @name Setup
    /// @{

    /// Internal initialization of stages and OpenGL constructs
    /// required for program validation and rendering.
    /// An exception is thrown on failure.
    /// The exception will contain a list of initialization errors.
    void initialize(RenderContextHandle renderContextHandle = nullptr) override;

    /// @}
    /// @name Rendering
    /// @{

    /// Create Glsl Vulkan program based on an input shader
    /// @param shader Input HwShader
    void createProgram(ShaderPtr shader) override;

    /// Create Glsl Vulkan program based on shader stage source code.
    /// @param stages Map of name and source code for the shader stages.
    void createProgram(const StageMap& stages) override;

    /// Validate inputs for the program
    void validateInputs() override;

    /// Set the size of the rendered image
    void setSize(unsigned int width, unsigned int height) override;

    void prepareMeshBuffers();

    void drawMesh();

    void drawCheckerboard();

    /// Render the current program to an offscreen buffer.
    void render() override;

    /// Render the current program in texture space to an off-screen buffer.
    void renderTextureSpace(const Vector2& uvMin, const Vector2& uvMax);

    /// @}
    /// @name Utilities
    /// @{

    /// Capture the current contents of the off-screen hardware buffer as an image.
    ImagePtr captureImage(ImagePtr image = nullptr) override;

    struct FrameData
    {
        vk::Semaphore imageAcquiredSemaphore;
        vk::Fence drawFence;
        vk::CommandBuffer commandBuffer;
        uint32_t imageIndex;
    };
    struct VkRenderContext
    {
        VulkanDevicePtr vkDevice;
        vk::Device device;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        const VulkanDevice::SwapChainData& swapchain;

        explicit VkRenderContext(const VulkanDevicePtr& dev) :
            vkDevice(dev),
            device(dev->GetDevice()),
            graphicsQueue(device.getQueue(dev->getGraphicsAndPresentQueueIndex().first, 0)),
            presentQueue(device.getQueue(dev->getGraphicsAndPresentQueueIndex().second, 0)),
            swapchain(dev->getSwapChainData())
        {
        }
    };

     struct MeshBuffer
    {
        VkBufferPtr vertexBuffer;
        VkBufferPtr indexBuffer;
        uint32_t indexCount;
        MeshPtr mesh; 

        VkUniformBufferPtr modelUBO;
        vk::DescriptorSet modelDescriptorSet;
    };
    std::vector<MeshBuffer> _meshBuffers;


    /// Set the screen background color.
    void setScreenColor(const Color3& screenColor)
    {
        _screenColor = screenColor;
    }

    /// Return the screen background color.
    Color3 getScreenColor() const
    {
        return _screenColor;
    }

    /// @}
    
    vk::Extent2D getSurfaceExtent()
    {
        return _surfaceExtent;
    }

   
    private:

  protected:
    VkRenderer(unsigned int width, unsigned int height, Image::BaseType baseType);

    virtual void updateViewInformation();
    virtual void updateWorldInformation();
   
    FrameData acquireFrame(const vk::RenderPass& renderPass, const std::vector<vk::Framebuffer>& framebuffers);

    void recordDrawCommands(vk::CommandBuffer& cmd, const vk::RenderPass& renderPass, const vk::Framebuffer& framebuffer);

    Matrix44 createScaleMatrix(float sx, float sy, float sz);

    void submitAndPresent(FrameData& frame, vk::RenderPass renderPass);

    void drawMesh(const MeshPtr& mesh);


    void cleanupFrame(FrameData& frame, const vk::RenderPass& renderPass);

    std::vector<vk::ImageLayout> _swapImageLayouts;

    void setUseCheckerboard(bool enabled) { _useCheckerboard = enabled; }

    //TODO: VULKAN
    void createFrameBuffer(bool encodeSrgb);
 
    void drawCube();
  private:
    VkProgramPtr _program;
    VkBufferPtr _cubeVtxBuffer;
    VkUniformBufferPtr _uniformBuffer;
    vk::Extent2D _surfaceExtent;
    VulkanDevicePtr _vkDevice;
    VkTexturePtr _texture;
    VkBufferPtr _vertexBuffer;
    VkBufferPtr _indexBuffer;
    uint32_t _indexCount = 0;
    VkTexturePtr _checkeredTexture;
    bool _useCheckerboard = false;
    bool _initialized;

    const Vector3 _eye;
    const Vector3 _center;
    const Vector3 _up;
    float _objectScale;

    SimpleWindowPtr _window;
    Color3 _screenColor;

    VkFrameBufferPtr _frameBuffer;

};

MATERIALX_NAMESPACE_END
#endif
