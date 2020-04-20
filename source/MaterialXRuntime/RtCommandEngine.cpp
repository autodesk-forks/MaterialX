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

}
