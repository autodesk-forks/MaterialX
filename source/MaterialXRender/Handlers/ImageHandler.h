#ifndef MATERIALX_IMAGEHANDLER_H
#define MATERIALX_IMAGEHANDLER_H

#include <string>
#include <memory>
#include <cmath>
#include <unordered_map>

namespace MaterialX
{
class Color4;

/// @class @ImageDesc
/// Interface to description an image. Images are assumed to be float type.
class ImageDesc
{
  public:
    /// Image width
    unsigned width = 0; 
    /// Image height
    unsigned height = 0;
    /// Number of channels
    unsigned int channelCount = 0;
    /// Number of mip map levels
    unsigned int mipCount = 0;
    /// CPU buffer. May be empty
    float* resourceBuffer = nullptr;
    /// Hardware target dependent resource identifier. May be empty
    unsigned int resourceId = 0;

    /// Compute the number of mip map levels based on size of the image
    void computeMipCount()
    {
        mipCount = (unsigned int)std::log2(std::max(width, height)) + 1;
    }
};

/// Image description cache
using ImageDescCache = std::unordered_map<std::string, ImageDesc>;

/// Shared pointer to an ImageLoader
using ImageLoaderPtr = std::shared_ptr<class ImageLoader>;

/// @class @ImageLoader
/// Abstract class representing an disk image loader
///
class ImageLoader
{
  public:
    /// Default constructor
    ImageLoader() {}

    /// Default destructor
    virtual ~ImageLoader() {}

    /// Save image to disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to save image to
    /// @param imageDesc Description of image
    /// @return if save succeeded
    virtual bool saveImage(const std::string& fileName,
                           const ImageDesc &imageDesc) = 0;

    /// Load an image from disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to load image from
    /// @param imageDesc Description of image updated during load.
    /// @return if load succeeded
    virtual bool loadImage(const std::string& fileName,
                           ImageDesc &imageDesc) = 0;
};

/// Shared pointer to an ImageHandler
using ImageHandlerPtr = std::shared_ptr<class ImageHandler>;

/// @class @ImageHandler
/// A image handler class. Can keep track of images which are loaded
/// from disk via an ImageLoader or created from memory
///
class ImageHandler
{
  public:
    /// Default constructor
    ImageHandler(ImageLoaderPtr imageLoader) :
        _imageLoader(imageLoader)
    {
    }
    
    /// Default destructor
    virtual ~ImageHandler() {}

    /// Save image to disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to save image to
    /// @param imageDesc Description of image
    /// @return if save succeeded
    virtual bool saveImage(const std::string& fileName,
                           const ImageDesc &imageDesc);

    /// Get an image from disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to load image from.
    /// @param imageDesc Description of image updated during load.
    /// @param generateMipMaps Generate mip maps if supported.
    /// @return if load succeeded
    virtual bool getImage(std::string& fileName, ImageDesc& desc, bool generateMipMaps);

    /// Utility to create a solid color color image 
    /// @param color Color to set
    /// @param imageDesc Description of image updated during load.
    /// @return if creation succeeded
    virtual bool createColorImage(const MaterialX::Color4& color,
                                  ImageDesc& imageDesc);
 
    /// Bind an image
    virtual bool bindImage(const std::string& /*identifier*/) = 0;

    /// Clear the image cache
    virtual void clearImageCache()
    {
        _imageCache.clear();
    }

  protected:
    /// Cache an image for reuse.
    /// @param identifier Description identifier to use.
    /// @param imageDesc Image description
    void cacheImage(const std::string& identifier, const ImageDesc& imageDesc);

    /// Uncache an image.
    /// @param identifier Description identifier to use.
    void uncacheImage(const std::string& identifier);

    /// Get an image description in the image cache if it exists
    /// @param identifier Description identifier to search for.
    /// @return A null ptr is returned if not found.
    const ImageDesc* getCachedImage(const std::string& identifier);

    /// Return the image cache
    ImageDescCache& getImageCache()
    {
        return _imageCache;
    }

    /// Delete image
    virtual void deleteImage(ImageDesc& /*imageDesc*/) = 0;

    /// Image loader utility
    ImageLoaderPtr _imageLoader;
    /// Image description cache
    ImageDescCache _imageCache;
};

} // namespace MaterialX
#endif
