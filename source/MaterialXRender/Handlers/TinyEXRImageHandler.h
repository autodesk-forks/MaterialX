#ifndef MATERIALX_TINYEXRIMAGEHANDLER_H
#define MATERIALX_TINYEXRIMAGEHANDLER_H

#include <MaterialXRender/Handlers/ImageHandler.h>

namespace MaterialX
{
/// Shared pointer to an ImageHandler
using TinyEXRImageHandlerPtr = std::shared_ptr<class TinyEXRImageHandler>;

/// @class @TinyEXRImageHandler
/// Disk image handler wrapper using TinyEXR
///
class TinyEXRImageHandler : public ImageHandler
{
public:
    /// Static instance create function
    static TinyEXRImageHandlerPtr create() { return std::make_shared<TinyEXRImageHandler>(); }

    /// Default constructor
    TinyEXRImageHandler() {}

    /// Default destructor
    virtual ~TinyEXRImageHandler() {}

    /// Save image to disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to save image to
    /// @param imageDesc Description of image
    /// @return if save succeeded
    bool saveImage(const std::string& fileName,
                   const ImageDesc &imageDesc) override;

    /// Load an image from disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to load image from
    /// @param imageDesc Description of image updated during load.
    /// @return if load succeeded
    bool loadImage(const std::string& fileName,
                   ImageDesc &imageDesc) override;
};

} // namespace MaterialX;

#endif
