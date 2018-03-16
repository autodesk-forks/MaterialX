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

    GeometryIndexBuffer& getIndexing(size_t &bufferSize) override;
    FloatGeometryBuffer& getPositions(size_t &bufferSize, const InputProperties& properties) override;
    FloatGeometryBuffer& getNormals(size_t &bufferSize) override;
    FloatGeometryBuffer& getTextureCoords(size_t &bufferSize, unsigned int index = 0) override;
    FloatGeometryBuffer& getTangents(size_t &bufferSize, unsigned int index = 0) override;
    FloatGeometryBuffer& getBitangents(size_t &bufferSize, unsigned int index = 0) override;
    FloatGeometryBuffer& getColors(size_t &bufferSize, unsigned int index = 0) override;

  private:
    void clearData();

    GeometryIndexBuffer _indexing;
    FloatGeometryBuffer _positionData;
    FloatGeometryBuffer _normalData;
    FloatGeometryBuffer _texcoordData;
    FloatGeometryBuffer _tangentData;
    FloatGeometryBuffer _bitangentData;
    FloatGeometryBuffer _colorData;
};

} // namespace MaterialX
#endif
