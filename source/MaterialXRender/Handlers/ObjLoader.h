#ifndef MATERIALX_OBJGEOMETRYHANDLER_H
#define MATERIALX_OBJGEOMETRYHANDLER_H

#include <string>
#include <memory>
#include <MaterialXRender/Handlers/GeometryHandler.h>

namespace MaterialX
{
/// Shared pointer to an ObjLoader
using ObjLoaderPtr = std::shared_ptr<class ObjLoader>;

/// @class @ObjHandler
/// Utility geometry handler to read in OBJ files to providing data for shader binding.
///
class ObjLoader : public GeometryLoader
{
  public:
    /// Static instance create function
    static ObjLoaderPtr create() { return std::make_shared<ObjLoader>(); }

    /// Default constructor
    ObjLoader() {}
    
    /// Default destructor
    virtual ~ObjLoader() {}

    /// Load geometry from disk
    bool load(const std::string& fileName, MeshList& meshList) override;
};

} // namespace MaterialX
#endif
