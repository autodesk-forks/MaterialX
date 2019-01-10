#include <MaterialXGenShader/Util.h>
#include <MaterialXRender/Handlers/GeometryHandler.h>

namespace MaterialX
{ 
void GeometryHandler::addLoader(GeometryLoaderPtr loader)
{
    const StringVec& extensions = loader->supportedExtensions();
    for (auto extension : extensions)
    {
        _geometryLoaders.insert(std::pair<std::string, GeometryLoaderPtr>(extension, loader));
    }
}

bool GeometryHandler::loadGeometry(const std::string& fileName)
{
    // Early return if file already loaded
    if (_fileName == fileName)
        return true;

    const float MAX_FLOAT = std::numeric_limits<float>::max();

    // Remove any existing meshes, and reset cached file name
    _meshes.clear();
    _fileName.clear();
    _minimumBounds = { 0.0f, 0.0f, 0.0f };
    _maximumBounds = { 0.0f, 0.0f, 0.0f };

    std::pair <GeometryLoaderMap::iterator, GeometryLoaderMap::iterator> range;
    string extension = MaterialX::getFileExtension(fileName);
    range = _geometryLoaders.equal_range(extension);
    GeometryLoaderMap::iterator first = --range.second;
    GeometryLoaderMap::iterator last = --range.first;
    for (auto it = first; it != last; --it)
    {
        bool loaded = it->second->load(fileName, _meshes);
        if (loaded)
        {
            _fileName = fileName;

            // Update the min and max bounds
            _minimumBounds = { MAX_FLOAT, MAX_FLOAT, MAX_FLOAT };
            _maximumBounds = { -MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT };
            for (auto mesh : _meshes)
            {
                const Vector3& minMesh = mesh->getMinimumBounds();
                _minimumBounds[0] = std::min(minMesh[0], _minimumBounds[0]);
                _minimumBounds[1] = std::min(minMesh[1], _minimumBounds[1]);
                _minimumBounds[2] = std::min(minMesh[1], _minimumBounds[2]);
                const Vector3& maxMesh = mesh->getMinimumBounds();
                _maximumBounds[0] = std::max(maxMesh[0], _maximumBounds[0]);
                _maximumBounds[1] = std::max(maxMesh[1], _maximumBounds[1]);
                _maximumBounds[2] = std::max(maxMesh[1], _maximumBounds[2]);
            }

            return true;
        }
    }
    return false;
}

}