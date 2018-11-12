#ifndef MATERIALX_DEFAULT_COLOR_MANAGEMENT_SYSTEM_H
#define MATERIALX_DEFAULT_COLOR_MANAGEMENT_SYSTEM_H

#include <MaterialXGenShader/ColorManagementSystem.h>

namespace MaterialX
{

/// A shared pointer to a DefaultColorManagementSystem
using DefaultColorManagementSystemPtr = shared_ptr<class DefaultColorManagementSystem>;

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

    /// Load the library of implementations from the provided document
    void loadLibrary(DocumentPtr document) override
    {
        _document = document;
    }

    /// Retrieves an implementation with the provided name
    ImplementationPtr getImplementation(const string& implName) override
    {
        return _document->getImplementation(implName);
    }

    /// Returns an implementation name for a given transform
    string getImplementationName(const ColorSpaceTransform& transform) override;

    static const string CMS_NAME;

  private:
    /// Protected constructor
    DefaultColorManagementSystem(DocumentPtr document, ShaderGenerator& shadergen);

    DocumentPtr _document;
};

} // namespace MaterialX

#endif
