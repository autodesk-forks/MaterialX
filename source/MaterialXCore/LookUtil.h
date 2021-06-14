//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_LOOK_UTIL_H
#define MATERIALX_LOOK_UTIL_H

#include <MaterialXCore/Look.h>

namespace MaterialX
{

// Return the list of all look elements which are active
LookVec getActiveLooks(const LookGroupPtr& lookGroup);

/// Append the contents of another lookgroup to this lookgroup.
/// Optionally allow appending after a given look.
void appendLookGroup(LookGroupPtr& lookGroup, const LookGroupPtr& lookGroupToAppend, const string& appendAfterLook = EMPTY_STRING);

/// Append the contents of a look to this lookgroup
/// Optionally allow appending after a given look.
void appendLook(LookGroupPtr& lookGroup, const string& lookName, const string& appendAfterLook = EMPTY_STRING);

/// Get a single combined look wihch contains the contents of all the looks in the lookgroup
LookPtr combineLooks(const LookGroupPtr& lookGroup);

} // namespace MaterialX

#endif

