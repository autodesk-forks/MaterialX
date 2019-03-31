//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/Handlers/OiioImageLoader.h>

#if defined(OSWin_) || defined(_WIN32)
#pragma warning( push )
#pragma warning( disable: 4100)
#pragma warning( disable: 4505)
#pragma warning( disable: 4800)
#pragma warning( disable: 4244)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <OpenImageIO/imageio.h>


#if defined(OSWin_) || defined(_WIN32)
#pragma warning( pop ) 
#elif defined(__clang__)
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif

namespace MaterialX
{

bool OiioImageLoader::saveImage(const FilePath& filePath,
                                const ImageDesc &imageDesc)
{
    OIIO::ImageSpec imageSpec;
    imageSpec.width = imageDesc.width;
    imageSpec.height = imageDesc.height;
    imageSpec.nchannels = imageDesc.channelCount;

    OIIO::TypeDesc format = OIIO::TypeDesc::UINT8;
    switch (imageDesc.baseType)
    {
    case ImageDesc::BaseType::FLOAT:
    {
        format.basetype = OIIO::TypeDesc::FLOAT;
        break;
    }
    case ImageDesc::BaseType::HALF_FLOAT:
    {
        format.basetype = OIIO::TypeDesc::HALF;
        break;
    }
    case ImageDesc::BaseType::UINT8:
        break;
    default:
        return false;
    };


    OIIO::ImageOutput* imageOutput = OIIO::ImageOutput::create(filePath);
    if (!imageOutput)
    {
        return false;
    }

    bool written = false;
    if (imageOutput->open(filePath, imageSpec))
    {
        written = imageOutput->write_image(format, imageDesc.resourceBuffer);
        imageOutput->close();
    }
    return written;
}

bool OiioImageLoader::acquireImage(const FilePath& filePath,
                                  ImageDesc& imageDesc,
                                  const HwImageDescRestrictions* restrictions)
{
    imageDesc.width = imageDesc.height = imageDesc.channelCount = 0;
    imageDesc.resourceBuffer = nullptr;

    OIIO::ImageInput* imageInput = OIIO::ImageInput::open(filePath);
    if (!imageInput)
    {
        return false;
    }

    OIIO::ImageSpec imageSpec = imageInput->spec();
    switch (imageSpec.format.basetype)
    {
        case OIIO::TypeDesc::UINT8:
        {
            imageDesc.baseType = ImageDesc::BaseType::UINT8;
            break;
        }
        case OIIO::TypeDesc::FLOAT:
        {
            imageDesc.baseType = ImageDesc::BaseType::FLOAT;
            break;
        }
        case OIIO::TypeDesc::HALF:
        {
            if (restrictions && restrictions->supportedBaseTypes.count(ImageDesc::BaseType::HALF_FLOAT) == 0)
            {
                // 16-bit float is not support so loadin as 32-bit float.
                imageSpec.set_format(OIIO::TypeDesc::FLOAT);
                imageDesc.baseType = ImageDesc::BaseType::FLOAT;
            }
            else
            {
                imageDesc.baseType = ImageDesc::BaseType::HALF_FLOAT;
            }
            break;
        }
        default:
            return false;
    };

    imageDesc.width = imageSpec.width;
    imageDesc.height = imageSpec.height;
    imageDesc.channelCount = imageSpec.nchannels;
    imageDesc.computeMipCount();

    size_t imageBytes = (size_t) imageSpec.image_bytes();
    void* imageBuf = malloc(imageBytes);
    bool read = false;
    if (imageInput->read_image(imageSpec.format, imageBuf))
    {
        imageDesc.resourceBuffer = imageBuf;
        read = true;
    }
    imageInput->close();
    return read;
}

} // namespace MaterialX

