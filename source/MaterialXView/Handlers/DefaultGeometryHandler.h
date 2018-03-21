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

    IndexBuffer& getIndexing() override;
    FloatBuffer& getPositions(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getNormals(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getTextureCoords(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getTangents(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getBitangents(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getColors(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getAttribute(const std::string& attributeType, unsigned int& stride, unsigned int index = 0) override;

  private:
    void clearData();

    IndexBuffer _indexing;
    FloatBuffer _positionData;
    FloatBuffer _normalData;
    FloatBuffer _texcoordData[2];
    FloatBuffer _tangentData[2];
    FloatBuffer _bitangentData[2];
    FloatBuffer _colorData[2];
};

} // namespace MaterialX
#endif
