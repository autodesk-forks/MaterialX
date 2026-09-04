//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TEXTUREBAKER_VK
#define MATERIALX_TEXTUREBAKER_VK

/// @file
/// Texture baking functionality for the Vulkan backend

#include <MaterialXRender/TextureBaker.h>

#include <MaterialXRenderVk/Export.h>
#include <MaterialXRenderVk/VkRenderer.h>
#include <MaterialXRenderVk/VkFramebuffer.h>

#include <MaterialXGenGlsl/VkShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// A shared pointer to a TextureBakerVk
using TextureBakerVkPtr = shared_ptr<class TextureBakerVk>;

/// A vector of baked documents with their associated names.
using BakedDocumentVec = std::vector<std::pair<std::string, DocumentPtr>>;

/// @class TextureBakerVk
/// An implementation of TextureBaker based on Vulkan (GLSL→SPIR-V) shader generation.
/// Requires zero template changes — TextureBaker<Renderer, ShaderGen> needs exactly:
/// Renderer(w,h,baseType), initialize(), createImageHandler(ImageLoaderPtr),
/// getFramebuffer()->setEncodeSrgb(bool), createProgram(ShaderPtr),
/// renderTextureSpace(min,max), captureImage(ImagePtr), _imageHandler, _baseType —
/// all delivered by Phases 2–3. _generator is ShaderGeneratorPtr (TextureBaker.h:288),
/// so VkShaderGenerator::create() fits.
class MX_RENDERVK_API TextureBakerVk : public TextureBaker<VkRenderer, VkShaderGenerator>
{
  public:
    static TextureBakerVkPtr create(unsigned int width = 1024, unsigned int height = 1024,
                                    Image::BaseType baseType = Image::BaseType::UINT8)
    {
        return TextureBakerVkPtr(new TextureBakerVk(width, height, baseType));
    }

    TextureBakerVk(unsigned int width, unsigned int height, Image::BaseType baseType);
};

MATERIALX_NAMESPACE_END

#endif
