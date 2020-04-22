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

    /// Undo the last previously executed command.
    void undo(RtCommandResult& result);

    /// Redo the last previously executed undo command.
    void redo(RtCommandResult& result);

    /// Flush the undo and redo queues.
    /// All commands previously executed will no longer be undoable.
    void flushUndoQueue();

private:
    vector<RtCommandPtr> _undoQueue;
    vector<RtCommandPtr> _redoQueue;
};

}

#endif
