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

    // Remove any existing meshes, and reset cached file name
    _meshes.clear();
    _fileName.clear();

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
            return true;
        }
    }
    return false;
}

}