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

class RtSetAttributeCmd : public RtCommandBase
{
public:
    RtSetAttributeCmd(const RtAttribute& attr, const RtValue& value);

    static RtCommandPtr create(const RtAttribute& attr, const RtValue& value);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;
    void redo(RtCommandResult& result) override;

private:
    RtAttribute _attr;
    RtValue _value;
    RtValue _oldValue;
};

class RtMakeConnectionCmd : public RtCommandBase
{
public:
    RtMakeConnectionCmd(const RtOutput& src, const RtInput& dest) :
        _src(src),
        _dest(dest)
    {}

    static RtCommandPtr create(const RtOutput& src, const RtInput& dest);

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

    static RtCommandPtr create(const RtOutput& src, const RtInput& dest);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtOutput _src;
    RtInput _dest;
};




namespace RtCommand
{
    /// Set value on an attribute.
    void setAttribute(const RtAttribute& attr, const RtValue& value, RtCommandResult& result);

    /// Set value on a bool attribute.
    void setAttribute(const RtAttribute& attr, bool value, RtCommandResult& result);

    /// Set value on a integer attribute.
    void setAttribute(const RtAttribute& attr, int value, RtCommandResult& result);

    /// Set value on a float attribute.
    void setAttribute(const RtAttribute& attr, float value, RtCommandResult& result);

    /// Set value on a color2 attribute.
    void setAttribute(const RtAttribute& attr, const Color2& value, RtCommandResult& result);

    /// Set value on a color3 attribute.
    void setAttribute(const RtAttribute& attr, const Color3& value, RtCommandResult& result);

    /// Set value on a color4 attribute.
    void setAttribute(const RtAttribute& attr, const Color4& value, RtCommandResult& result);

    /// Set value on a vector2 attribute.
    void setAttribute(const RtAttribute& attr, const Vector2& value, RtCommandResult& result);

    /// Set value on a vector3 attribute.
    void setAttribute(const RtAttribute& attr, const Vector3& value, RtCommandResult& result);

    /// Set value on a vector4 attribute.
    void setAttribute(const RtAttribute& attr, const Vector4& value, RtCommandResult& result);

    /// Set value on a token attribute.
    void setAttribute(const RtAttribute& attr, const RtToken& value, RtCommandResult& result);

    /// Set value on a pointer attribute.
    void setAttribute(const RtAttribute& attr, void* value, RtCommandResult& result);

    /// Set value on a matrix33 attribute.
    void setAttribute(const RtAttribute& attr, const Matrix33& value, RtCommandResult& result);

    /// Set value on a matrix44 attribute.
    void setAttribute(const RtAttribute& attr, const Matrix44& value, RtCommandResult& result);

    /// Set value on a string attribute.
    void setAttribute(const RtAttribute& attr, const string& value, RtCommandResult& result);

    /// Make a connection from src output to dest input.
    void makeConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result);

    /// Break a connection from src output to dest input.
    void breakConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result);
}

}

#endif
