#ifndef MATERIALX_GEOMETRYHANDLER_H
#define MATERIALX_GEOMETRYHANDLER_H

#include <string>
#include <memory>

namespace MaterialX
{
/// Shared pointer to an GeometryHandler
using GeometryHandlerPtr = std::shared_ptr<class GeometryHandler>;

/// @class @GeometryHandler
/// Utility geometry handler for providing data for shader binding.
///
class GeometryHandler
{
  public:
    /// Static instance creator
    static GeometryHandlerPtr creator() { return std::make_shared<GeometryHandler>(); }

    /// Default constructor
    GeometryHandler();
    
    /// Default destructor
    virtual ~GeometryHandler();

    void setIdentifier(const std::string identifier);

    virtual unsigned int* getIndexing(size_t &bufferSize);
    virtual float* getPositions(size_t &bufferSize);
    virtual float* getNormals(size_t &bufferSize);
    virtual float* getTextureCoords(const std::string& setIdentifier, size_t &bufferSize);
    virtual float* getTangents(const std::string& setIdentifier, size_t &bufferSize);
    virtual float* getBitangents(const std::string& setIdentifier, size_t &bufferSize);
    virtual float* getColors(const std::string& setIdentifier, size_t &bufferSize);

    static const std::string SCREEN_ALIGNED_QUAD;

  private:
    void clearData();

    unsigned int* _indexing;
    float* _positionData;
    float* _normalData;
    float* _texcoordData;
    float* _tangentData;
    float* _bitangentData;
    float* _colorData;

    std::string _identifier;
};

} // namespace MaterialX
#endif
