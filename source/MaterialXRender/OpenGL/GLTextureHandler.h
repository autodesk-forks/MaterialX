#ifndef MATERIALX_GLTEXTUREHANDLER_H
#define MATERIALX_GLTEXTUREHANDLER_H

#include <MaterialXRender/Handlers/ImageHandler.h>

namespace MaterialX
{
/// Shared pointer to an OpenGL texture handler
using GLTextureHandlerPtr = std::shared_ptr<class GLTextureHandler>;

/// @class @GLTextureHandler
/// An OpenGL texture handler class
///
class GLTextureHandler : public ImageHandler
{
  public:
    using ParentClass = ImageHandler;

    /// Static instance create function
    static GLTextureHandlerPtr create(ImageLoaderPtr imageLoader)
    { 
        return std::make_shared<GLTextureHandler>(imageLoader);
    }

    /// Default constructor
    GLTextureHandler(ImageLoaderPtr imageLoader) :
        ParentClass(imageLoader),
        _maxImageUnits(-1),
        _textureUnitsInUse(0)
    {
    }
    
    /// Default destructor
    virtual ~GLTextureHandler() {}

    bool createColorImage(const MaterialX::Color4& color,
                          ImageDesc& imageDesc) override;

    /// Get an image. 
    /// If the image failed to load then a single color image is created instead
    bool getImage(std::string& fileName,
                  ImageDesc &imageDesc, bool generatateMipMaps) override;

    /// Bind an image
    bool bindImage(const string &identifier) override;

    /// Clear image cache
    void clearImageCache() override;

  protected:
    /// Delete image
    void deleteImage(MaterialX::ImageDesc& imageDesc) override;

    /// Maximum image units
    int _maxImageUnits;
    /// Active texture units used
    int _textureUnitsInUse;
};

} // namespace MaterialX
#endif
