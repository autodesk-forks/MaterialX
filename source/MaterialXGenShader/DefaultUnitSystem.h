//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_DEFAULT_UNIT_SYSTEM_H
#define MATERIALX_DEFAULT_UNIT_SYSTEM_H

/// @file
/// Sample default unit system implementation

#include <MaterialXGenShader/UnitSystem.h>

namespace MaterialX
{

/// A shared pointer to a DefaultUnitSystem
using DefaultUnitSystemPtr = shared_ptr<class DefaultUnitSystem>;

/// @class DefaultUnitSystem
/// Class for a default unit systemsystem. The default unit systemsystem users
/// the typical workflow for registering nodes with the exception that the target for them
/// nodedefs is set to DefaultUnitSystem::CMS_NAME.
///
class DefaultUnitSystem : public UnitSystem
{
  public:
    virtual ~DefaultUnitSystem() { }

    /// Create a new DefaultUnitSystem
    static DefaultUnitSystemPtr create(const string& language);

    /// Return the DefaultUnitSystem name
    const string& getName() const override
    {
        return DefaultUnitSystem::UNITSYTEM_NAME;
    }

    static const string UNITSYTEM_NAME;

  protected:
    /// Returns an implementation name for a given transform
    string getImplementationName(const UnitTransform& transform) const override;

    /// Protected constructor
    DefaultUnitSystem(const string& language);

  private:
    string _language;
};

} // namespace MaterialX

#endif
