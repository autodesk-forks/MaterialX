//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtCommandEngine.h>

namespace MaterialX
{

void PvtCommandEngine::execute(RtCommandPtr cmd, RtCommandResult& result)
{
    cmd->execute(result);

    if (result.success())
    {
        _undoQueue.push_back(cmd);
        _redoQueue.clear();
    }
}

void PvtCommandEngine::undo(RtCommandResult& result)
{
    if (!_undoQueue.empty())
    {
        RtCommandPtr cmd = _undoQueue.back();
        _undoQueue.pop_back();

        cmd->undo(result);

        if (result.success())
        {
            _redoQueue.push_back(cmd);
        }
    }
    else
    {
        result = RtCommandResult(false, string("No command to undo"));
    }
}

void PvtCommandEngine::redo(RtCommandResult& result)
{
    if (!_redoQueue.empty())
    {
        RtCommandPtr cmd = _redoQueue.back();
        _redoQueue.pop_back();

        cmd->redo(result);

        if (result.success())
        {
            _undoQueue.push_back(cmd);
        }
    }
    else
    {
        result = RtCommandResult(false, string("No command to redo"));
    }
}

void PvtCommandEngine::flushUndoQueue()
{
    _undoQueue.clear();
    _redoQueue.clear();
}

}
