#ifndef MATERIALX_GEOMETRYHANDLER_H
#define MATERIALX_GEOMETRYHANDLER_H

#include <string>
#include <memory>

namespace MaterialX
{
/// Shared pointer to an GeometryHandler
using GeometryHandlerPtr = std::shared_ptr<class GeometryHandler>;

/// @class @GeometryHandler
/// Abstract class representing a geometry handler for providing data for shader binding.
///
class GeometryHandler
{
  public:
    /// Default constructor
    GeometryHandler();
    
    /// Default destructor
    virtual ~GeometryHandler();

    /// Set the identifier which will indicate what geometry to return
    /// e.g. SCREEN_ALIGNED_QUAD is one identifier
    virtual void setIdentifier(const std::string identifier);
    
    /// Get geometry identifier
    const std::string& getIdentifier() const
    {
        return _identifier;
    }

    /// Create indexing data for geometry. The indexing is assumed to 
    /// be for a set of triangles. That is every 3 values index triangle data.
    /// @param bufferSize The size of the returned buffer in bytes.
    /// @param indexCount Number of indices in the indexing
    virtual unsigned int* getIndexing(size_t &bufferSize, unsigned int& indexCount) = 0;

    /// Create position data. Each element is assumed to be 3 float in size.
    /// @param bufferSize The size of the returned buffer in bytes.
    /// @param screenWidth Width of screen. Required to initialzie screen positions. Defaults to 0.
    /// @param screenHeight Height of screen. Required to initialzie screen positions. Defaults to 0.
    virtual float* getPositions(size_t &bufferSize, unsigned screenWidth=0, unsigned int screenHeight=0) = 0;

    /// Create normal data. Each element is assumed to be 3 float in size.
    /// @param bufferSize The size of the returned buffer in bytes.
    virtual float* getNormals(size_t &bufferSize) = 0;

    /// Create texture coordinate (uv) data. Each element is assumed to be 2 float in size.
    /// @param setIdentifier Name identifier for a given set of data. Passing in
    /// an empty string indicates to use the "default" or first set.
    /// @param bufferSize The size of the returned buffer in bytes.
    virtual float* getTextureCoords(const std::string& setIdentifier, size_t &bufferSize) = 0;

    /// Create tangent data. Each element is assumed to be 3 float in size.
    /// @param setIdentifier Name identifier for a given set of data. Passing in
    /// an empty string indicates to use the "default" or first set.
    /// @param bufferSize The size of the returned buffer in bytes.
    virtual float* getTangents(const std::string& setIdentifier, size_t &bufferSize) = 0;

    /// Create bitangent data. Each element is assumed to be 3 float in size.
    /// @param setIdentifier Name identifier for a given set of data. Passing in
    /// an empty string indicates to use the "default" or first set.
    /// @param bufferSize The size of the returned buffer in bytes.
    virtual float* getBitangents(const std::string& setIdentifier, size_t &bufferSize) = 0;

    /// Create color data. Each element is assumed to be 4 float in size.
    /// @param setIdentifier Name identifier for a given set of data. Passing in
    /// an empty string indicates to use the "default" or first set.
    /// @param bufferSize The size of the returned buffer in bytes.
    virtual float* getColors(const std::string& setIdentifier, size_t &bufferSize) = 0;

    /// Geometry identifier indicating to create data for a screen aligned quad
    static const std::string SCREEN_ALIGNED_QUAD;

  protected:
     std::string _identifier;
};

} // namespace MaterialX
#endif
