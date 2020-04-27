//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Commands/AttributeCommands.h>
#include <MaterialXRuntime/RtApi.h>

namespace MaterialX
{

RtSetAttributeCmd::RtSetAttributeCmd(const RtAttribute& attr, const RtValue& value) :
    _attr(attr),
    _value(RtValue::clone(attr.getType(), value, attr.getParent()))
{
}

RtCommandPtr RtSetAttributeCmd::create(const RtAttribute& attr, const RtValue& value)
{
    return std::make_shared<RtSetAttributeCmd>(attr, value);
}

void RtSetAttributeCmd::execute(RtCommandResult& result)
{
    if (_attr.isValid())
    {
        try
        {
            // Save old value for undo/redo
            _oldValue = RtValue::clone(_attr.getType(), _attr.getValue(), _attr.getParent());

            // Set the value
            _attr.setValue(_value);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Attribute to set is no longer valid"));
    }
}

void RtSetAttributeCmd::undo(RtCommandResult& result)
{
    if (_attr.isValid())
    {
        try
        {
            // Reset the value
            _attr.setValue(_oldValue);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Attribute to set is no longer valid"));
    }
}

void RtSetAttributeCmd::redo(RtCommandResult& result)
{
    if (_attr.isValid())
    {
        try
        {
            // Set the value
            _attr.setValue(_value);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Attribute to set is no longer valid"));
    }
}

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

void setAttribute(const RtAttribute& attr, const RtValue& value, RtCommandResult& result)
{
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, value);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, bool value, RtCommandResult& result)
{
    RtValue v(value);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, int value, RtCommandResult& result)
{
    RtValue v(value);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, float value, RtCommandResult& result)
{
    RtValue v(value);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Color2& value, RtCommandResult& result)
{
    RtValue v(value);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Color3& value, RtCommandResult& result)
{
    RtValue v(value);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Color4& value, RtCommandResult& result)
{
    RtValue v(value);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Vector2& value, RtCommandResult& result)
{
    RtValue v(value);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Vector3& value, RtCommandResult& result)
{
    RtValue v(value);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Vector4& value, RtCommandResult& result)
{
    RtValue v(value);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const RtToken& value, RtCommandResult& result)
{
    RtValue v(value);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, void* value, RtCommandResult& result)
{
    RtValue v(value);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Matrix33& value, RtCommandResult& result)
{
    RtPrim prim(attr.getParent());
    RtValue v(value, prim);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Matrix44& value, RtCommandResult& result)
{
    RtPrim prim(attr.getParent());
    RtValue v(value, prim);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const string& value, RtCommandResult& result)
{
    RtPrim prim(attr.getParent());
    RtValue v(value, prim);
    RtCommandPtr cmd = RtSetAttributeCmd::create(attr, v);
    RtApi::get().getCommandEngine().execute(cmd, result);
}

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
