#ifndef MATERIALX_DEFAULTGEOMETRYHANDLER_H
#define MATERIALX_DEFAULTGEOMETRYHANDLER_H

#include <string>
#include <memory>
#include <MaterialXView/Handlers/GeometryHandler.h>

namespace MaterialX
{
/// Shared pointer to an GeometryHandler
using DefaultGeometryHandlerPtr = std::shared_ptr<class DefaultGeometryHandler>;

/// @class @GeometryHandler
/// Utility geometry handler for providing data for shader binding.
///
class DefaultGeometryHandler : public GeometryHandler
{
  public:
    /// Static instance creator
    static DefaultGeometryHandlerPtr creator() { return std::make_shared<DefaultGeometryHandler>(); }

    /// Default constructor
    DefaultGeometryHandler();
    
    /// Default destructor
    virtual ~DefaultGeometryHandler();

    void setIdentifier(const std::string identifier) override;

    unsigned int* getIndexing(size_t &bufferSize, unsigned int& indexCount) override;
    float* getPositions(size_t &bufferSize, unsigned screenWidth = 0, unsigned int screenHeight = 0) override;
    float* getNormals(size_t &bufferSize) override;
    float* getTextureCoords(const std::string& setIdentifier, size_t &bufferSize) override;
    float* getTangents(const std::string& setIdentifier, size_t &bufferSize) override;
    float* getBitangents(const std::string& setIdentifier, size_t &bufferSize) override;
    float* getColors(const std::string& setIdentifier, size_t &bufferSize) override;

  private:
    void clearData();

    unsigned int* _indexing;
    float* _positionData;
    float* _normalData;
    float* _texcoordData;
    float* _tangentData;
    float* _bitangentData;
    float* _colorData;
};

} // namespace MaterialX
#endif
