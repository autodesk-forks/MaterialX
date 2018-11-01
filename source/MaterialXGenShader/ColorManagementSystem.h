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
/// A shared pointer to a DefaultColorManagementSystem
using DefaultColorManagementSystemPtr = shared_ptr<class DefaultColorManagementSystem>;
/// A shared pointer to an OCIOColorManagementSystemPtr
using OCIOColorManagementSystemPtr = shared_ptr<class OCIOColorManagementSystem>;

/// @stuct @ColorSpaceTransform
/// Structure that represents color space transform information
struct ColorSpaceTransform
{
    string sourceSpace;
    string targetSpace;
    TypeDesc* type;

    /// Comparison operator
    bool operator==(const ColorSpaceTransform &other) const
    {
        return sourceSpace == other.sourceSpace &&
               targetSpace == other.targetSpace &&
               ((type == nullptr && other.type == nullptr) ||
                (type != nullptr && other.type != nullptr && type->getName() == other.type->getName()));
    }
};

/// ColorSpaceTransform hash function
struct ColorSpaceTransformHash {
    size_t operator()(const ColorSpaceTransform &transform ) const
    {
        if (transform.type == nullptr)
        {
            return std::hash<string>()(transform.sourceSpace) ^ std::hash<string>()(transform.targetSpace);
        }
        else
        {
            return std::hash<string>()(transform.sourceSpace) ^ std::hash<string>()(transform.targetSpace) ^ std::hash<string>()(transform.type->getName());
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
    void setConfigFile(const string& configFile)
    {
        _configFile = configFile;
        initialize(); // re-initialize
    }

    /// Initialization function
    virtual void initialize() = 0;

    /// Returns a shader node name for a given transform
    virtual string getShaderNodeName(const ColorSpaceTransform& transform);

    /// Returns an implementation name for a given transform
    virtual string getImplementationName(const ColorSpaceTransform& transform) = 0;

    /// Create a node to use to perform the given color space transformation.
    ShaderNodePtr createNode(const ColorSpaceTransform& transform);

  protected:
    template<class T>
    using CreatorFunction = shared_ptr<T>(*)();

    /// Register a node implementation for a given color space transformation.
    bool registerImplementation(const ColorSpaceTransform& transform, CreatorFunction<ShaderNodeImpl> creator);

    /// Protected constructor
    ColorManagementSystem(DocumentPtr document, ShaderGenerator& shadergen, const string& configFile);

    Factory<ShaderNodeImpl> _implFactory;
    std::unordered_map<ColorSpaceTransform, ShaderNodePtr, ColorSpaceTransformHash> _cachedImpls;
    string _configFile;
    DocumentPtr _document;
    ShaderGenerator& _shadergen;
};

/// @class @DefaultColorManagementSystem
/// Class for a default color management system. The default color management system users
/// the typical workflow for registering nodes with the exception that the target for them
/// nodedefs is set to DefaultColorManagementSystem::CMS_NAME.
///
class DefaultColorManagementSystem : public ColorManagementSystem
{
  public:
    /// Create a new DefaultColorManagementSystem
    static DefaultColorManagementSystemPtr create(DocumentPtr document, ShaderGenerator& shadergen, const string& configFile);

    /// Return the DefaultColorManagementSystem name
    const string& getName() const
    {
        return DefaultColorManagementSystem::CMS_NAME;
    }

    /// Initialization function
    void initialize() override;

    /// Returns an implementation name for a given transform
    string getImplementationName(const ColorSpaceTransform& transform) override;

    static const string CMS_NAME;

  protected:
    /// Protected constructor
    DefaultColorManagementSystem(DocumentPtr document, ShaderGenerator& shadergen);
};

/// @class @OCIOColorManagementSystem
/// Class for an OCIO color management system. The OCIO color management system generates node
/// implementations for the required colorspaces from the OCIO config file and ultimately the
/// OCIO library.
///
class OCIOColorManagementSystem : public ColorManagementSystem
{
  public:
    static OCIOColorManagementSystemPtr create(DocumentPtr document, ShaderGenerator& shadergen, const string& configFile);

    /// Return the OCIOColorManagementSystem name
    const string& getName() const
    {
        return OCIOColorManagementSystem::CMS_NAME;
    }

    /// Initialization function
    void initialize() override
    {
    }

    /// Returns an implementation name for a given transform
    string getImplementationName(const ColorSpaceTransform& transform) override;

    static const string CMS_NAME;

  protected:
    /// Protected constructor
    OCIOColorManagementSystem(DocumentPtr document, ShaderGenerator& shadergen, const string& configFile);
};

} // namespace MaterialX

#endif
