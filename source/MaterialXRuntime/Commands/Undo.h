//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTUNDOCOMMANDS_H
#define MATERIALX_RTUNDOCOMMANDS_H

/// @file
/// Commands for undo/redo handling.

#include <MaterialXRuntime/RtCommandEngine.h>

namespace MaterialX
{

namespace RtCommand
{
    /// Undo the last executed command.
    void undo(RtCommandResult& result);

    /// Redo the last executed undo command.
    void redo(RtCommandResult& result);
}

}

#endif
