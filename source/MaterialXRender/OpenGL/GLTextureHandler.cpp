#include <MaterialXCore/Types.h>
#include <MaterialXRender/OpenGL/GLTextureHandler.h>
#include <MaterialXRender/OpenGL/GlslProgram.h>
#include <MaterialXRender/External/GLew/glew.h>

namespace MaterialX
{
bool GLTextureHandler::createColorImage(const MaterialX::Color4& color,
                                        ImageDesc& imageDesc)
{
    ParentClass::createColorImage(color, imageDesc);
    if ((imageDesc.width * imageDesc.height > 0) && imageDesc.resourceBuffer)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &imageDesc.resourceId);
        glActiveTexture(GL_TEXTURE0 + imageDesc.resourceId);
        glBindTexture(GL_TEXTURE_2D, imageDesc.resourceId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, imageDesc.width, imageDesc.height, 0, GL_RGBA, GL_FLOAT, imageDesc.resourceBuffer);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }
    return false;
}

bool GLTextureHandler::getImage(std::string& fileName,
                                ImageDesc &imageDesc,
                                bool generateMipMaps)
{
    if (fileName.empty())
    {
        return false;
    }

    // Check to see if we have already loaded in the texture.
    // If so, reuse the existing texture id
    const ImageDesc* cachedDesc = getCachedImage(fileName);
    if (cachedDesc)
    {
        imageDesc = *cachedDesc;
        return true;
    }

    bool textureLoaded = false;
    if (ParentClass::getImage(fileName, imageDesc, generateMipMaps) &&
        (imageDesc.channelCount == 3 || imageDesc.channelCount == 4))
    {

        imageDesc.resourceId = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &imageDesc.resourceId);
        glActiveTexture(GL_TEXTURE0 + imageDesc.resourceId);
        glBindTexture(GL_TEXTURE_2D, imageDesc.resourceId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageDesc.width, imageDesc.height,
            0, (imageDesc.channelCount == 4 ? GL_RGBA : GL_RGB), GL_FLOAT, imageDesc.resourceBuffer);
        if (generateMipMaps)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        free(imageDesc.resourceBuffer);
        imageDesc.resourceBuffer = nullptr;

        cacheImage(fileName, imageDesc);
        textureLoaded = true;
    }

    if (!textureLoaded)
    {
        const string BLACK_TEXTURE("@internal_black_texture@");
        const ImageDesc* cachedColorDesc = getCachedImage(BLACK_TEXTURE);
        if (cachedColorDesc)
        {
            imageDesc = *cachedColorDesc;
        }
        else
        {
            Color4 color(0.0f, 0.0f, 0.0f, 1.0f);
            ImageDesc desc;
            desc.channelCount = 4;
            desc.width = 1;
            desc.height = 1;
            createColorImage(color, desc);
            glBindTexture(GL_TEXTURE_2D, desc.resourceId);

            cacheImage(BLACK_TEXTURE, desc);
        }
        fileName = BLACK_TEXTURE;
        textureLoaded = true;
    }

    return textureLoaded;
}


bool GLTextureHandler::bindImage(const string &identifier)
{        
    const ImageDesc* cachedDesc = getCachedImage(identifier);
    if (cachedDesc)
    {
        unsigned int resourceId = cachedDesc->resourceId;

        // Bind a texture to the next available slot
        if (_maxImageUnits < 0)
        {
            glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &_maxImageUnits);
        }
        if (resourceId == MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID ||
            resourceId == static_cast<unsigned int>(_maxImageUnits))
        {
            return false;
        }
        glActiveTexture(GL_TEXTURE0 + resourceId);
        glBindTexture(GL_TEXTURE_2D, resourceId);
        return true;
    }
    return false;
}

void GLTextureHandler::clearImageCache()
{
    ImageDescCache& cache = getImageCache();
    for (auto iter : cache)
    {
        deleteImage(iter.second);
    }
    ParentClass::clearImageCache();
}

void GLTextureHandler::deleteImage(MaterialX::ImageDesc& imageDesc)
{
    if (imageDesc.resourceId != MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
    {
        // Unbind a texture from image unit and delete the texture
        glActiveTexture(GL_TEXTURE0 + imageDesc.resourceId);
        glBindTexture(GL_TEXTURE_2D, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glDeleteTextures(1, &imageDesc.resourceId);
    }
    // Delete any CPU side memory
    if (imageDesc.resourceBuffer)
    {
        free(imageDesc.resourceBuffer);
        imageDesc.resourceBuffer = nullptr;
    }
}

}