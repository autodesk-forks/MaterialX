#ifndef MATERIALX_GEOMETRYHANDLER_H
#define MATERIALX_GEOMETRYHANDLER_H

#include <MaterialXRender/Handlers/Mesh.h>
#include <memory>
#include <map>

namespace MaterialX
{

/// Shared pointer to a GeometryLoader
using GeometryLoaderPtr = std::shared_ptr<class GeometryLoader>;

/// @class @GeometryLoader
/// Base class representing a geometry loader
class GeometryLoader
{
  public:
    /// Default constructor
    GeometryLoader() {}

    /// Default destructor
    virtual ~GeometryLoader() {}

    /// Returns a list of supported extensions
    /// @return List of support extensions
    const std::vector<std::string>& supportedExtensions()
    {
        return _extensions;
    }

    /// Load geometry from disk
    virtual bool load(const std::string& fileName, MeshList& meshList) = 0;

  protected:
    /// List of supported string extensions
    std::vector<std::string> _extensions;
};

/// Shared pointer to an GeometryHandler
using GeometryHandlerPtr = std::shared_ptr<class GeometryHandler>;

/// Map of extensions to image loaders
using GeometryLoaderMap = std::multimap<std::string, GeometryLoaderPtr>;

/// @class @GeometryHandler
/// Abstract class representing a geometry handler for providing data for shader binding.
///
class GeometryHandler
{
  public: 
    /// Default constructor
    GeometryHandler() {};
    
    /// Default destructor
    virtual ~GeometryHandler() {};

    /// Add geometry loader
    /// @param loader Loader to add to list of available loaders.
    void addLoader(GeometryLoaderPtr loader);

    /// Load geometry from disk
    bool loadGeometry(const std::string& fileName);

    /// Get list of meshes
    const MeshList& getMeshes() const
    {
        return _meshes;
    }

    /// Return the minimum bounds for the geometry
    const Vector3& getMinimumBounds() const
    {
        return _minimumBounds;
    }

    /// Return the minimum bounds for the geometry
    const Vector3& getMaximumBounds() const
    {
        return _maximumBounds;
    }

  protected:
    GeometryLoaderMap _geometryLoaders;
    MeshList _meshes;
    std::string _fileName;
    Vector3 _minimumBounds;
    Vector3 _maximumBounds;
};

} // namespace MaterialX
#endif
