//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtBreakConnectionCmd.h>

namespace MaterialX
{

PvtCommandPtr PvtBreakConnectionCmd::create(const RtOutput& src, const RtInput& dest)
{
    return std::make_shared<PvtBreakConnectionCmd>(src, dest);
}

void PvtBreakConnectionCmd::execute(RtCommandResult& result)
{
    if (_src.isValid() && _dest.isValid())
    {
        try
        {
            // Send message that the connection is to be broken
            msg().sendBreakConnectionMessage(_src, _dest);

            // Break the connection.
            _src.disconnect(_dest);
            result = RtCommandResult(true);
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

void PvtBreakConnectionCmd::undo(RtCommandResult& result)
{
    if (_src.isValid() && _dest.isValid())
    {
        try
        {
            // Send message that the connection is to be made
            msg().sendMakeConnectionMessage(_src, _dest);

            // Make the connection.
            _src.connect(_dest);
            result = RtCommandResult(true);
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

}
