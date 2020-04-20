//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTCONNECTIONCOMMANDS_H
#define MATERIALX_RTCONNECTIONCOMMANDS_H

/// @file
/// Commands for connection handling.

#include <MaterialXRuntime/RtCommandEngine.h>
#include <MaterialXRuntime/RtAttribute.h>

namespace MaterialX
{

class RtMakeConnectionCmd : public RtCommandBase
{
public:
    RtMakeConnectionCmd(const RtOutput& src, const RtInput& dest) :
        _src(src),
        _dest(dest)
    {}

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtOutput _src;
    RtInput _dest;
};

class RtBreakConnectionCmd : public RtCommandBase
{
public:
    RtBreakConnectionCmd(const RtOutput& src, const RtInput& dest) :
        _src(src),
        _dest(dest)
    {}

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtOutput _src;
    RtInput _dest;
};

namespace RtCommand
{
    /// Make a connection from src to dest.
    void makeConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result);

    /// Break a connection from src to dest.
    void breakConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result);
}

}

#endif
