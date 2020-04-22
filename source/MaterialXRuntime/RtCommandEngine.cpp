//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtCommandEngine.h>
#include <MaterialXRuntime/Private/PvtCommandEngine.h>

namespace MaterialX
{

namespace
{
    // Syntactic sugar
    using RtCommandPtrVec = vector<RtCommandPtr>;
    inline RtCommandPtrVec* _getvec(void* ptr)
    {
        return static_cast<RtCommandPtrVec*>(ptr);
    }
}

RtBatchCommand::RtBatchCommand() :
    _ptr(new vector<RtCommandPtr>)
{
}

RtBatchCommand::~RtBatchCommand()
{
    delete _getvec(_ptr);
}

void RtBatchCommand::addCommand(RtCommandPtr cmd)
{
    _getvec(_ptr)->push_back(cmd);
}

void RtBatchCommand::clearCommands()
{
    _getvec(_ptr)->clear();
}

void RtBatchCommand::execute(RtCommandResult& result)
{
    for (RtCommandPtr cmd : *_getvec(_ptr))
    {
        cmd->execute(result);
        if (!result.success())
        {
            break;
        }
    }
}

void RtBatchCommand::undo(RtCommandResult& result)
{
    for (RtCommandPtr cmd : *_getvec(_ptr))
    {
        cmd->undo(result);
        if (!result.success())
        {
            break;
        }
    }
}

void RtBatchCommand::redo(RtCommandResult& result)
{
    for (RtCommandPtr cmd : *_getvec(_ptr))
    {
        cmd->redo(result);
        if (!result.success())
        {
            break;
        }
    }
}


namespace
{
    // Syntactic sugar
    inline PvtCommandEngine* _cast(void* ptr)
    {
        return static_cast<PvtCommandEngine*>(ptr);
    }
}

RtCommandEngine::RtCommandEngine() :
    _ptr(new PvtCommandEngine())
{
}

RtCommandEngine::~RtCommandEngine()
{
    delete _cast(_ptr);
}

void RtCommandEngine::execute(RtCommandPtr cmd, RtCommandResult& result)
{
    _cast(_ptr)->execute(cmd, result);
}

void RtCommandEngine::undo(RtCommandResult& result)
{
    _cast(_ptr)->undo(result);
}

void RtCommandEngine::redo(RtCommandResult& result)
{
    _cast(_ptr)->redo(result);
}

void RtCommandEngine::flushUndoQueue()
{
    _cast(_ptr)->flushUndoQueue();
}

}
