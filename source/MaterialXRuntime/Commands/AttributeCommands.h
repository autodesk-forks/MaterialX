//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTATTRIBUTECOMMANDS_H
#define MATERIALX_RTATTRIBUTECOMMANDS_H

/// @file
/// Commands for connection handling.

#include <MaterialXRuntime/RtCommand.h>
#include <MaterialXRuntime/RtPort.h>

namespace MaterialX
{

namespace RtCommand
{
    /// Set an attribute value from data given as a value string.
    void setAttributeFromString(const RtPort& attr, const string& valueString, RtCommandResult& result);

    /// Set value on a bool attribute.
    void setAttribute(const RtPort& attr, bool value, RtCommandResult& result);

    /// Set value on a integer attribute.
    void setAttribute(const RtPort& attr, int value, RtCommandResult& result);

    /// Set value on a float attribute.
    void setAttribute(const RtPort& attr, float value, RtCommandResult& result);

    /// Set value on a color3 attribute.
    void setAttribute(const RtPort& attr, const Color3& value, RtCommandResult& result);

    /// Set value on a color4 attribute.
    void setAttribute(const RtPort& attr, const Color4& value, RtCommandResult& result);

    /// Set value on a vector2 attribute.
    void setAttribute(const RtPort& attr, const Vector2& value, RtCommandResult& result);

    /// Set value on a vector3 attribute.
    void setAttribute(const RtPort& attr, const Vector3& value, RtCommandResult& result);

    /// Set value on a vector4 attribute.
    void setAttribute(const RtPort& attr, const Vector4& value, RtCommandResult& result);

    /// Set value on a token attribute.
    void setAttribute(const RtPort& attr, const RtToken& value, RtCommandResult& result);

    /// Set value on a pointer attribute.
    void setAttribute(const RtPort& attr, void* value, RtCommandResult& result);

    /// Set value on a matrix33 attribute.
    void setAttribute(const RtPort& attr, const Matrix33& value, RtCommandResult& result);

    /// Set value on a matrix44 attribute.
    void setAttribute(const RtPort& attr, const Matrix44& value, RtCommandResult& result);

    /// Set value on a string attribute.
    void setAttribute(const RtPort& attr, const string& value, RtCommandResult& result);

    /// Make a connection from src output to dest input.
    void makeConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result);

    /// Break a connection from src output to dest input.
    void breakConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result);
}

}

#endif
