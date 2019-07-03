//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Types.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXRender/ImageHandler.h>
#include <cmath>

namespace MaterialX
{
string ImageDesc::BASETYPE_UINT8 = "UINT8";
string ImageDesc::BASETYPE_HALF = "HALF";
string ImageDesc::BASETYPE_FLOAT = "FLOAT";

string ImageDesc::IMAGETYPE_2D = "IMAGE2D";

void ImageDesc::freeResourceBuffer()
{
    if (resourceBuffer)
    {
        if (resourceBufferDeallocator)
        {
            resourceBufferDeallocator(resourceBuffer);
        }
        else
        {
            free(resourceBuffer);
        }
        resourceBuffer = nullptr;
    }
}

string ImageLoader::BMP_EXTENSION = "bmp";
string ImageLoader::EXR_EXTENSION = "exr";
string ImageLoader::GIF_EXTENSION = "gif";
string ImageLoader::HDR_EXTENSION = "hdr";
string ImageLoader::JPG_EXTENSION = "jpg";
string ImageLoader::JPEG_EXTENSION = "jpeg";
string ImageLoader::PIC_EXTENSION = "pic";
string ImageLoader::PNG_EXTENSION = "png";
string ImageLoader::PSD_EXTENSION = "psd";
string ImageLoader::TGA_EXTENSION = "tga";
string ImageLoader::TIF_EXTENSION = "tif";
string ImageLoader::TIFF_EXTENSION = "tiff";
string ImageLoader::TX_EXTENSION = "tx";
string ImageLoader::TXT_EXTENSION = "txt";
string ImageLoader::TXR_EXTENSION = "txr";

ImageHandler::ImageHandler(ImageLoaderPtr imageLoader)
{
    addLoader(imageLoader);
}

void ImageHandler::addLoader(ImageLoaderPtr loader)
{
    if (loader)
    {
        const StringSet& extensions = loader->supportedExtensions();
        for (auto extension : extensions)
        {
            _imageLoaders.insert(std::pair<string, ImageLoaderPtr>(extension, loader));
        }
    }
}

void ImageHandler::supportedExtensions(StringSet& extensions)
{
    extensions.clear();
    for (auto loader : _imageLoaders)
    {
        const StringSet& loaderExtensions = loader.second->supportedExtensions();
        extensions.insert(loaderExtensions.begin(), loaderExtensions.end());
    }
}

bool ImageHandler::saveImage(const FilePath& filePath,
                             const ImageDesc &imageDesc,
                             bool verticalFlip)
{
    FilePath foundFilePath = findFile(filePath);
    if (foundFilePath.isEmpty())
    {
        return false;
    }

    string extension = foundFilePath.getExtension();
    ImageLoaderMap::reverse_iterator iter;
    for (iter = _imageLoaders.rbegin(); iter != _imageLoaders.rend(); ++iter)
    {
        ImageLoaderPtr loader = iter->second;
        if (loader && loader->supportedExtensions().count(extension))
        {
            bool saved = iter->second->saveImage(foundFilePath, imageDesc, verticalFlip);
            if (saved)
            {
                return true;
            }
        }
    }
    return false;
}

bool ImageHandler::acquireImage(const FilePath& filePath, ImageDesc& imageDesc, bool /*generateMipMaps*/, const Color4* /*fallbackColor*/)
{
    if (filePath.isEmpty())
    {
        return false;
    }

    string extension = filePath.getExtension();
    ImageLoaderMap::reverse_iterator iter;
    for (iter = _imageLoaders.rbegin(); iter != _imageLoaders.rend(); ++iter)
    {
        ImageLoaderPtr loader = iter->second;
        if (loader && loader->supportedExtensions().count(extension))
        {
            bool acquired = loader->loadImage(filePath, imageDesc, getRestrictions());
            if (acquired)
            {
                return true;
            }
        }
    }
    return false;
}

bool ImageHandler::createColorImage(const Color4& color,
                                    ImageDesc& desc)
{
    unsigned int bufferSize = desc.width * desc.height * desc.channelCount;
    if (bufferSize < 1)
    {
        return false;
    }

    // Create a solid color image
    //
    desc.resourceBuffer = new float[bufferSize];
    float* pixel = static_cast<float*>(desc.resourceBuffer);
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
    desc.resourceBufferDeallocator = [](void *buffer)
    {
        delete[] static_cast<float*>(buffer);
    };
    return true;
}

bool ImageHandler::bindImage(const FilePath& /*filePath*/, const ImageSamplingProperties& /*samplingProperties*/)
{
    return false;
}

bool ImageHandler::unbindImage(const FilePath& /*filePath*/)
{
    return false;
}

void ImageHandler::cacheImage(const string& filePath, const ImageDesc& desc)
{
    if (!_imageCache.count(filePath))
    {
        _imageCache[filePath] = desc;
    }
}

void ImageHandler::uncacheImage(const string& filePath)
{
    _imageCache.erase(filePath);
}

const ImageDesc* ImageHandler::getCachedImage(const string& filePath)
{
    if (_imageCache.count(filePath))
    {
        return &(_imageCache[filePath]);
    }
    return nullptr;
}

FilePath ImageHandler::findFile(const FilePath& filePath)
{
    return _searchPath.find(filePath);
}

void ImageHandler::deleteImage(ImageDesc& imageDesc)
{
    imageDesc.freeResourceBuffer();
}

void ImageHandler::clearImageCache()
{
    for (auto iter : _imageCache)
    {
        deleteImage(iter.second);
    }
    _imageCache.clear();
}

FilePath ImageHandler::getResolveUDIMInformation(const FilePath& filePath, const string& udimString, vector<int>& udimTile)
{
    FilePath resolvedFilePath = filePath;
    udimTile.resize(2);
    // If not tiled then the location is tile 0,0.
    udimTile[0] = udimTile[1] = 0;

    if (udimString.empty())
    {
        return resolvedFilePath;
    }    

    int udimVal = std::stoi(udimString);
    if (udimVal <= 1000 || udimVal >= 2000)
    {
        throw Exception("Invalid udim value specified" + udimString);
    }

    // Compute resolved UDIM name
    StringMap map;
    map[UDIM_TOKEN] = udimString;
    resolvedFilePath = FilePath(replaceSubstrings(filePath.asString(), map));
    
    // Compute UDIM tile location
    udimVal -= 1000;
    udimTile[0] = udimVal % 10;
    udimTile[0] = (udimTile[0] == 0) ? 9 : udimTile[0] - 1;
    udimTile[1] = (udimVal - udimTile[0] - 1) / 10;

    return resolvedFilePath;
}

FilePathVec ImageHandler::getResolveUDIMInformation(const FilePath& filePath, const StringVec& udimSet, vector<int>& udimTiles)
{
    FilePathVec resolvedFilePaths;
    if (udimSet.empty())
    {
        return resolvedFilePaths;
    }    

    for (const string& udimString : udimSet)
    {
        if (udimString.empty())
        {
            continue;
        }

        vector<int> udimTile;
        FilePath resolvedFilePath = getResolveUDIMInformation(filePath, udimString, udimTile);
        resolvedFilePaths.push_back(resolvedFilePath);
        udimTiles.push_back(udimTile[0]);
        udimTiles.push_back(udimTile[1]);
    }

    return resolvedFilePaths;
}

} // namespace MaterialX
