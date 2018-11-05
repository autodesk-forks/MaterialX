#include <MaterialXCore/Types.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXRender/Handlers/ImageHandler.h>
#include <cmath>

namespace MaterialX
{
bool ImageHandler::saveImage(const std::string& fileName,
                            const ImageDesc &imageDesc)
{
    for (auto loader : _imageLoaders)
    {
        if (loader->supportsExtension(getFileExtension(fileName)))
        {
            return loader->saveImage(fileName, imageDesc);
        }
    }
    return false;
}

bool ImageHandler::getImage(std::string& fileName,
                             ImageDesc &imageDesc, 
                             bool /*generatateMipMaps*/)
{
    for (auto loader : _imageLoaders)
    {
        if (loader->supportsExtension(getFileExtension(fileName)))
        {
            return loader->loadImage(fileName, imageDesc);
        }
    }
    return false;
}

bool ImageHandler::createColorImage(const MaterialX::Color4& color,
                                    ImageDesc& desc)
{
    // Create a solid color image
    //
    desc.resourceBuffer = new float[desc.width * desc.height * desc.channelCount];
    float* pixel = desc.resourceBuffer;
    for (size_t i = 0; i<desc.width; i++)
    {
        for (size_t j = 0; j<desc.height; j++)
        {
            for (unsigned int c = 0; c < desc.channelCount; c++)
            {
                *pixel++ = color[c];
            }
        }
    }
    desc.computeMipCount();
    return true;
}

void ImageHandler::cacheImage(const std::string& identifier, const ImageDesc& desc)
{
    if (!_imageCache.count(identifier))
    {
        _imageCache[identifier] = desc;
    }
}

void ImageHandler::uncacheImage(const std::string& identifier)
{
    _imageCache.erase(identifier);
}

const ImageDesc* ImageHandler::getCachedImage(const std::string& identifier)
{
    if (_imageCache.count(identifier))
    {
        return &(_imageCache[identifier]);
    }
    return nullptr;
}

}