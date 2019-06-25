#ifndef PLUGIN_H
#define PLUGIN_H

#include <MaterialXFormat/File.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>

class Plugin
{
  public:
    static Plugin& instance();

    /// Plugin initialization
    void initialize(const std::string& loadPath);

    /// Get root path to search for MaterialX libraries
    const MaterialX::FilePath& getLibrarySearchPath() const
    {
        return _librarySearchPath;
    }

    /// Get root path to search for MaterialX resources
    const MaterialX::FilePath& getResourcePath() const
    {
        return _resourcePath;
    }

    /// Get path for shader debugging output
    const MaterialX::FilePath& getShaderDebugPath() const
    {
        return _shaderDebugPath;
    }

  private:
    Plugin()
    {
    }

    MaterialX::FilePath _librarySearchPath;
    MaterialX::FilePath _resourcePath;
    MaterialX::FilePath _shaderDebugPath;
};

#endif /* PLUGIN_H */
