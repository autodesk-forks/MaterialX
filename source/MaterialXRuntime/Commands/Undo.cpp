//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Commands/Undo.h>
#include <MaterialXRuntime/RtApi.h>

namespace MaterialX
{

namespace RtCommand
{

void undo(RtCommandResult& result)
{
    RtApi::get().getCommandEngine().undo(result);
}

void redo(RtCommandResult& result)
{
    RtApi::get().getCommandEngine().redo(result);
}

void flushUndoQueue()
{
    RtApi::get().getCommandEngine().flushUndoQueue();
}

}

}
