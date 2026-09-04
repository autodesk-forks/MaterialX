//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKRENDERER_H
#define MATERIALX_VKRENDERER_H

/// @file
/// Vulkan code renderer

#include <MaterialXRenderVk/Export.h>
#include <MaterialXRenderVk/VkContext.h>
#include <MaterialXRenderVk/VkProgram.h>

#include <MaterialXRender/ShaderRenderer.h>

MATERIALX_NAMESPACE_BEGIN

class VkFramebuffer;
using VkFramebufferPtr = std::shared_ptr<class VkFramebuffer>;

/// Shared pointer to a VkRenderer
using VkRendererPtr = std::shared_ptr<class VkRenderer>;

/// @class VkRenderer
/// Helper class for rendering generated Vulkan (GLSL→SPIR-V) code to produce images.
///
/// Mirrors GlslRenderer's API contract. Headless: no window, no swapchain.
class MX_RENDERVK_API VkRenderer : public ShaderRenderer
{
  public:
    /// Create a Vulkan renderer instance.
    static VkRendererPtr create(unsigned int width = 512, unsigned int height = 512,
                                Image::BaseType baseType = Image::BaseType::UINT8);

    /// Create a texture handler for Vulkan textures. Requires initialize() first.
    /// Not inline (unlike GlslRenderer.h:49-52) because it needs _context, so
    /// initialize() must run first. TextureBaker.inl:90-93 already calls them in
    /// that order, so this is safe.
    ImageHandlerPtr createImageHandler(ImageLoaderPtr imageLoader);

    /// Destructor
    virtual ~VkRenderer();

    /// @name Setup
    /// @{

    /// Internal initialization of the Vulkan instance, device, and framebuffer.
    /// @param renderContextHandle optional VkContext* for sharing an existing device.
    void initialize(RenderContextHandle renderContextHandle = nullptr) override;

    /// @}
    /// @name Rendering
    /// @{

    /// Create a Vulkan program based on an input shader.
    void createProgram(ShaderPtr shader) override;

    /// Create a Vulkan program based on shader stage source code.
    void createProgram(const StageMap& stages) override;

    /// Validate inputs for the program.
    void validateInputs() override;

    /// Update the program with value of the uniform.
    void updateUniform(const string& name, ConstValuePtr value) override;

    /// Set the size of the rendered image.
    void setSize(unsigned int width, unsigned int height) override;

    /// Render the current program to an offscreen buffer.
    void render() override;

    /// Render the current program in texture space to an off-screen buffer.
    /// Not a base virtual (ShaderRenderer declares no renderTextureSpace); the
    /// identical-signature declaration here is what makes TextureBaker's templated
    /// call dispatch to the Vulkan implementation.
    void renderTextureSpace(const Vector2& uvMin, const Vector2& uvMax);

    /// @}
    /// @name Utilities
    /// @{

    /// Capture the current contents of the off-screen buffer as an image.
    ImagePtr captureImage(ImagePtr image = nullptr) override;

    /// Return the Vulkan frame buffer.
    VkFramebufferPtr getFramebuffer() const { return _framebuffer; }

    /// Return the Vulkan program.
    VkProgramPtr getProgram() const { return _program; }

    /// Return the Vulkan context.
    VkContextPtr getContext() const { return _context; }

    /// Set the screen background color.
    void setScreenColor(const Color3& c) { _screenColor = c; }

    /// Return the screen background color.
    Color3 getScreenColor() const { return _screenColor; }

    /// @}

  protected:
    VkRenderer(unsigned int width, unsigned int height, Image::BaseType baseType);

    void createFrameBuffer(bool encodeSrgb);

  private:
    bool _initialized = false;
    VkContextPtr _context;
    VkProgramPtr _program;
    VkFramebufferPtr _framebuffer;
    Color3 _screenColor;
};

MATERIALX_NAMESPACE_END

#endif
