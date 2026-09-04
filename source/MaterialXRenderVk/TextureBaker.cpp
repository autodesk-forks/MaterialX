//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderVk/TextureBaker.h>

MATERIALX_NAMESPACE_BEGIN

TextureBakerVk::TextureBakerVk(unsigned int width, unsigned int height, Image::BaseType baseType) :
    TextureBaker<VkRenderer, VkShaderGenerator>(width, height, baseType, true)
{
}

MATERIALX_NAMESPACE_END
