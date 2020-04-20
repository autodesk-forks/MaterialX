//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTCOMMANDENGINE_H
#define MATERIALX_PVTCOMMANDENGINE_H


#include <MaterialXRuntime/RtCommandEngine.h>

#include <vector>

namespace MaterialX
{

class PvtCommandEngine
{
public:
    /// Execute a new command.
    void execute(RtCommandPtr cmd, RtCommandResult& result);

    /// Execute the command on top of the undo queue.
    void undo(RtCommandResult& result);

    /// Execute the command on top of the redo queue.
    void redo(RtCommandResult& result);

private:
    vector<RtCommandPtr> _undoQueue;
    vector<RtCommandPtr> _redoQueue;
};

}

#endif
