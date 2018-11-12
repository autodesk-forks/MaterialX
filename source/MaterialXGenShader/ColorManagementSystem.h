#ifndef MATERIALX_COLOR_MANAGEMENT_SYSTEM_H
#define MATERIALX_COLOR_MANAGEMENT_SYSTEM_H

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Library.h>

#include <MaterialXGenShader/Factory.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

class ShaderGenerator;

/// A shared pointer to a ColorManagementSystem
using ColorManagementSystemPtr = shared_ptr<class ColorManagementSystem>;

/// @stuct @ColorSpaceTransform
/// Structure that represents color space transform information
struct ColorSpaceTransform
{
    string sourceSpace;
    string targetSpace;
    const TypeDesc* type;

    /// Comparison operator
    bool operator==(const ColorSpaceTransform &other) const
    {
        return sourceSpace == other.sourceSpace &&
               targetSpace == other.targetSpace &&
               ((type == nullptr && other.type == nullptr) ||
                (type != nullptr && other.type != nullptr && type == other.type));
    }
};

/// ColorSpaceTransform hash function
struct ColorSpaceTransformHash {
    size_t operator()(const ColorSpaceTransform &transform ) const
    {
        if (transform.type == nullptr)
        {
            return std::hash<string>()(transform.sourceSpace + transform.targetSpace);
        }
        else
        {
            return std::hash<string>()(transform.sourceSpace + transform.targetSpace + transform.type->getName());
        }
    }
};

/// @class @ColorManagementSystem
/// Abstract base class for a ColorManagementSystem.
///
class ColorManagementSystem
{
  public:
    /// Return the ColorManagementSystem name
    virtual const string& getName() const = 0;

    /// Return the ColorManagementSystem configFile
    const string& getConfigFile() const
    {
        return _configFile;
    }

    /// Sets the config file
    void setConfigFile(const string& configFile);

    /// Load the library of implementations from the provided document
    virtual void loadLibrary(DocumentPtr document) = 0;

    /// Retrieves an implementation with the provided name
    virtual ImplementationPtr getImplementation(const string& implName) = 0;

    /// Create a node to use to perform the given color space transformation.
    ShaderNodePtr createNode(const ColorSpaceTransform& transform);

  protected:
    template<class T>
    using CreatorFunction = shared_ptr<T>(*)();

    /// Returns a shader node name for a given transform
    virtual string getShaderNodeName(const ColorSpaceTransform& transform);

    /// Returns an implementation name for a given transform
    virtual string getImplementationName(const ColorSpaceTransform& transform) = 0;

    /// Register a node implementation for a given color space transformation.
    bool registerImplementation(const ColorSpaceTransform& transform, CreatorFunction<ShaderNodeImpl> creator);

    /// Protected constructor
    ColorManagementSystem(ShaderGenerator& shadergen, const string& configFile, const string& language);

    Factory<ShaderNodeImpl> _implFactory;
    std::unordered_map<ColorSpaceTransform, ShaderNodePtr, ColorSpaceTransformHash> _cachedImpls;
    string _configFile;
    string _language;
    ShaderGenerator& _shadergen;
};

} // namespace MaterialX

#endif
