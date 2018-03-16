#ifndef MATERIALX_GEOMETRYHANDLER_H
#define MATERIALX_GEOMETRYHANDLER_H

#include <string>
#include <memory>
#include <vector>

namespace MaterialX
{
/// Shared pointer to an GeometryHandler
using GeometryHandlerPtr = std::shared_ptr<class GeometryHandler>;

/// Geomtry index buffer
using GeometryIndexBuffer = std::vector<unsigned int>;
/// A float geometry buffer
using FloatGeometryBuffer = std::vector<float>;

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

    /// Input properties that can be used when computing buffer data
    struct InputProperties
    {
        /// Output screen width
        unsigned int screenWidth;
        /// Output screen height
        unsigned int screenHeight;
        /// Output screen offset
        unsigned int screenOffset;

        /// Program input constructor
        InputProperties(unsigned int sw=512, unsigned sh=512, unsigned int so=20)
            : screenWidth(sw)
            , screenHeight(sh)
            , screenOffset(so)
        {}
    };

    /// Create indexing data for geometry. The indexing is assumed to 
    /// be for a set of triangles. That is every 3 values index triangle data.
    /// @param bufferSize The size of the returned buffer in bytes.
    virtual GeometryIndexBuffer& getIndexing(size_t &bufferSize) = 0;

    /// Create position data. Each element is assumed to be 3 float in size.
    /// @param bufferSize The size of the returned buffer in bytes.
    /// @param properties Input properties.
    virtual FloatGeometryBuffer& getPositions(size_t &bufferSize, const InputProperties& properties) = 0;

    /// Create normal data. Each element is assumed to be 3 float in size.
    /// @param bufferSize The size of the returned buffer in bytes.
    virtual FloatGeometryBuffer& getNormals(size_t &bufferSize) = 0;

    /// Create texture coordinate (uv) data. Each element is assumed to be 2 float in size.
    /// @param bufferSize The size of the returned buffer in bytes.
    /// @param index Attribute index for identifying different sets of data. Default value is 0.
    /// Passing in a 0 indicates to use the the "default" or first set of data.
    virtual FloatGeometryBuffer& getTextureCoords(size_t &bufferSize, unsigned int index = 0) = 0;

    /// Create tangent data. Each element is assumed to be 3 float in size.
    /// @param bufferSize The size of the returned buffer in bytes.
    /// @param index Attribute index for identifying different sets of data. Default value is 0.
    /// Passing in a 0 indicates to use the the "default" or first set of data.
    virtual FloatGeometryBuffer& getTangents(size_t &bufferSize, unsigned int index = 0) = 0;

    /// Create bitangent data. Each element is assumed to be 3 float in size.
    /// @param bufferSize The size of the returned buffer in bytes.
    /// @param index Attribute index for identifying different sets of data. Default value is 0.
    /// Passing in a 0 indicates to use the the "default" or first set of data.
    virtual FloatGeometryBuffer& getBitangents(size_t &bufferSize, unsigned int index=0) = 0;

    /// Create color data. Each element is assumed to be 4 float in size.
    /// @param bufferSize The size of the returned buffer in bytes.
    /// @param index Attribute index for identifying different sets of data. Default value is 0.
    /// Passing in a 0 indicates to use the the "default" or first set of data.
    virtual FloatGeometryBuffer& getColors(size_t &bufferSize, unsigned int index = 0) = 0;

    /// Geometry identifier indicating to create data for a screen aligned quad
    static const std::string SCREEN_ALIGNED_QUAD;

  protected:
     std::string _identifier;
};

} // namespace MaterialX
#endif
