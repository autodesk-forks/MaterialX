#ifndef MATERIALX_STBIMAGELOADER_H
#define MATERIALX_STBIMAGELOADER_H

#include <MaterialXRender/Handlers/ImageHandler.h>

namespace MaterialX
{
/// Shared pointer to an stbImageLoader
using stbImageLoaderPtr = std::shared_ptr<class stbImageLoader>;

/// @class @stbImageLoader
/// Disk image loader wrapper using stb library
///
class stbImageLoader : public ImageLoader
{
public:
    /// Static instance create function
    static stbImageLoaderPtr create() { return std::make_shared<stbImageLoader>(); }

    /// Default constructor
    stbImageLoader() 
    {
        _extensions.push_back("png");
        _extensions.push_back("bmp");
        _extensions.push_back("tga");
        _extensions.push_back("jpg");
        _extensions.push_back("jpeg");
        _extensions.push_back("hdr");
    }

    /// Default destructor
    virtual ~stbImageLoader() {}    

    /// Save image to disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to save image to
    /// @param imageDesc Description of image
    /// @return if save succeeded
    bool saveImage(const std::string& fileName,
                   const ImageDesc &imageDesc) override;

    /// Load an image from disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to load image from
    /// @param imageDesc Description of image updated during load.
    /// @param generateMipMaps Generate mip maps if supported.
    /// @return if load succeeded
    bool acquireImage(const std::string& fileName, ImageDesc &imageDesc, bool generateMipMaps) override;
};

} // namespace MaterialX;

#endif
