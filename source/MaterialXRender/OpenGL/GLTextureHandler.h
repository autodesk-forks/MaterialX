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
        _maxImageUnits(-1)
    {
    }
    
    /// Default destructor
    virtual ~GLTextureHandler() {}


    /// Utility to create a solid color color image 
    /// This method will create an OpenGL texture resource and return it's resource identifier
    /// as part of the image description returned.
    /// @param color Color to set
    /// @param imageDesc Description of image updated during load.
    /// @return if creation succeeded
    bool createColorImage(const MaterialX::Color4& color,
                          ImageDesc& imageDesc) override;

    /// Acquire an image from disk. 
    /// The first image loader which supports the file name extension will be used.
    /// This method will create an OpenGL texture resource and return it's resource identifier
    /// as part of the image description returned.
    /// @param fileName Name of file to load image from.
    /// @param imageDesc Description of image updated during load.
    /// @param generateMipMaps Generate mip maps if supported.
    /// @return if load succeeded
    bool acquireImage(std::string& fileName, ImageDesc &imageDesc, bool generatateMipMaps) override;

    /// Bind an image. This method will bind the texture to an active texture
    /// unit as defined by the corresponding image description. The method
    /// will fail if there are not enough available image units to bind to.
    /// @param identifier Identifier for image description to bind.
    /// @param imageProprties Binding properties for the image
    /// @return true if succeded to bind
    bool bindImage(const string &identifier, const ImagePropertiesDesc& imageProperties) override;

    /// Utility to make a shader value to an OpenGL address mode
    static int mapAddressModeToGL(const MaterialX::ValuePtr value);

    /// Utility to make a shader value to an OpenGL filter type
    static int mapFilterTypeToGL(const MaterialX::ValuePtr value);

    /// Clear image cache
    void clearImageCache() override;

  protected:
    /// Delete an image
    /// @param imageDesc Image description indicate which image to delete.
    /// Any OpenGL texture resource and as well as any CPU side reosurce memory will be deleted. 
    void deleteImage(MaterialX::ImageDesc& imageDesc) override;

    /// Maximum number of available image units
    int _maxImageUnits;
};

} // namespace MaterialX
#endif
