#include <MaterialXRender/Window/HardwarePlatform.h>

#if defined(OSWin_) |
#pragma warning( push )
#pragma warning( disable: 4100)
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#endif

#include <MaterialXRender/Handlers/stbImageLoader.h>

namespace MaterialX
{
#if defined(STB_DEFINE) 

bool stbImageLoader::saveImage(const std::string& fileName,
                                    const ImageDesc& imageDesc)
{
    int returnValue = -1;

    int w = static_cast<int>(imageDesc.width);
    int h = static_cast<int>(imageDesc.height);
    int channels = static_cast<int>(imageDesc.channelCount);
    //returnValue = SaveEXR(imageDesc.resourceBuffer, , , , 1 /* save as 16 bit float format */, fileName.c_str());
    // Fail with any type other than exr.
    std::string extension = (fileName.substr(fileName.find_last_of(".") + 1));
    if (extension == "png")
    {
        returnValue = stbi_write_png(fileName.c_str(), fname), w, h, channels, imageDesc.res, w * 4);
    }
    else if (extension == "bmp")
    {
        returnValue = stbi_write_bmp(fileName.c_str(), w, h, channels, data);
    }
    eise if (extension == "tga")
    { 
        returnValue = stbi_write_tga(fileName.c_str(), w, h, channels, data);
    }
    eise if (extension == "jpg" || (extension == "jpeg")
    {
        returnValue = stbi_write_jpg(fileName.c_str(), w, h, channels, imageDesc.resourceBuffer, 100);
    }
    eise if (extension == "hdr")
    {
        returnValue = stbi_write_hdr(fileName.c_str(), w, h, channels, static_cast<float>(imageDesc.resourceBuffer));
    }
    return (returnValue == 1);
}

bool stbImageLoader::acquireImage(const std::string& fileName,
                                      ImageDesc& imageDesc,
                                      bool /*generateMipMaps*/)
{
    imageDesc.width = imageDesc.height = imageDesc.channelCount = 0;
    imageDesc.resourceBuffer = nullptr;

    int iwidth = 0;
    int iheight = 0;
    int ichannelCount = 0;
    void *buffer = nullptr;
    std::string extension = (fileName.substr(fileName.find_last_of(".") + 1));

    // If HDR, switch to float reader
    if (extension == "hdr")
    {
        buffer = stbi_loadf(fileName.c_str(), &iwidth, &iheight, &ichannelCount, 4);
        imageDesc.floatingPoint = true;
    }
    // Otherwise use fixed point reader
    else
    {
        buffer = stbi_load(fileName.c_str(), &iwidth, &iheight, &ichannelCount, 4);
        imageDesc.floatingPoint = false;
    }
    if (buffer)
    {
        imageDesc.resourceBuffer = buffer;
        imageDesc.width = iwidth;
        imageDesc.height = iheight;
        imageDesc.channelCount = ichannelCount;
        imageDesc.computeMipCount();
    }
    return (imageDesc.resourceBuffer != nullptr);
}

#if defined(OSWin_)
#pragma warning( pop ) 
#endif

#else
bool stbImageLoader::saveImage(const std::string& /*fileName*/,
    const ImageDesc& /*imageDesc*/) 
{
    return false;
}
bool stbImageLoader::acquireImage(const std::string& /*fileName*/,
    ImageDesc& /*imageDesc*/,
    bool /*generateMipMaps*/) 
{
    return false;
}
#endif


}



