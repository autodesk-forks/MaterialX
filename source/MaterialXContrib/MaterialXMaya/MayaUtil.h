#ifndef MATERIALX_MAYA_MAYAUTIL_H
#define MATERIALX_MAYA_MAYAUTIL_H

#include <maya/MTextureManager.h>
#include <maya/MStateManager.h>

#include <string>
#include <memory>

namespace MaterialXMaya
{
namespace MayaUtil
{
void registerFragment(const std::string& fragmentName, const std::string& fragmentSource);

struct TextureDeleter
{
    void operator()(MHWRender::MTexture* texture);
};

using TextureUniquePtr = std::unique_ptr<
    MHWRender::MTexture,
    TextureDeleter
>;

struct SamplerDeleter
{
    void operator()(const MHWRender::MSamplerState* sampler);
};

using SamplerUniquePtr = std::unique_ptr<
    const MHWRender::MSamplerState,
    SamplerDeleter
>;

} // namespace MayaUtil
} // namespace MaterialXMaya

#endif
