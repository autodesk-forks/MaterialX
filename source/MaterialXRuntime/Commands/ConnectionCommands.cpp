//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Commands/ConnectionCommands.h>
#include <MaterialXRuntime/RtApi.h>

namespace MaterialX
{

RtCommandPtr RtMakeConnectionCmd::create(const RtOutput& src, const RtInput& dest)
{
    return std::make_shared<RtMakeConnectionCmd>(src, dest);
}

void RtMakeConnectionCmd::execute(RtCommandResult& result)
{
    if (_src.isValid() && _dest.isValid())
    {
        try
        {
            //
            // TODO: Do validation on the connections
            //
            _src.connect(_dest);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Ports to connect are no longer valid"));
    }
}

void RtMakeConnectionCmd::undo(RtCommandResult& result)
{
    if (_src.isValid() && _dest.isValid())
    {
        try
        {
            _src.disconnect(_dest);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Ports to disconnect are no longer valid"));
    }
}


RtCommandPtr RtBreakConnectionCmd::create(const RtOutput& src, const RtInput& dest)
{
    return std::make_shared<RtBreakConnectionCmd>(src, dest);
}

void RtBreakConnectionCmd::execute(RtCommandResult& result)
{
    if (_src.isValid() && _dest.isValid())
    {
        try
        {
            _src.disconnect(_dest);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Ports to disconnect are no longer valid"));
    }
}

void RtBreakConnectionCmd::undo(RtCommandResult& result)
{
    if (_src.isValid() && _dest.isValid())
    {
        try
        {
            _src.connect(_dest);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Ports to connect are no longer valid"));
    }
}


namespace RtCommand
{

void makeConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result)
{
    RtCommandPtr cmd = RtMakeConnectionCmd::create(src, dest);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void breakConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result)
{
    RtCommandPtr cmd = RtBreakConnectionCmd::create(src, dest);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

}

}
