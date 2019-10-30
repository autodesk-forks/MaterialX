//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtTypeDef.h>

#include <sstream>

#ifndef _WIN32
// Disable warnings for breaking strict aliasing.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

namespace MaterialX
{

RtValue::RtValue(const Matrix33& v, RtValueStore<Matrix33>& store)
{
    *reinterpret_cast<Matrix33**>(&_data) = store.alloc();
    asMatrix33() = v;
}

RtValue::RtValue(const Matrix44& v, RtValueStore<Matrix44>& store)
{
    *reinterpret_cast<Matrix44**>(&_data) = store.alloc();
    asMatrix44() = v;
}

RtValue::RtValue(const string& v, RtValueStore<string>& store)
{
    string* ptr = store.alloc();
    *ptr = v;
    asPtr() = ptr;
}

string RtValue::getValueString(const RtToken& type) const
{
    std::stringstream ss;
    if (type == RtType::BOOLEAN)
    {
        ss << asBool();
    }
    else if (type == RtType::FLOAT)
    {
        ss << asFloat();
    }
    else if (type == RtType::INTEGER)
    {
        ss << asInt();
    }
    else if (type == RtType::COLOR2)
    {
        const Color2& v = asColor2();
        ss << v[0] << ", " << v[1];
    }
    else if (type == RtType::COLOR3)
    {
        const Color3& v = asColor3();
        ss << v[0] << ", " << v[1] << ", " << v[2];
    }
    else if (type == RtType::COLOR4)
    {
        const Color4& v = asColor4();
        ss << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3];
    }
    else if (type == RtType::VECTOR2)
    {
        const Vector2& v = asVector2();
        ss << v[0] << ", " << v[1];
    }
    else if (type == RtType::VECTOR3)
    {
        const Vector3& v = asVector3();
        ss << v[0] << ", " << v[1] << ", " << v[2];
    }
    else if (type == RtType::VECTOR4)
    {
        const Vector4& v = asVector4();
        ss << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3];
    }
    else if (type == RtType::MATRIX33)
    {
        const Matrix33& v = asMatrix33();
        for (size_t i = 0; i < 3; i++)
        {
            for (size_t j = 0; j < 3; j++)
            {
                ss << v[i][j];
                if (i + j < 9)
                {
                    ss << ", ";
                }
            }
        }
    }
    else if (type == RtType::MATRIX44)
    {
        const Matrix44& v = asMatrix44();
        for (size_t i = 0; i < 4; i++)
        {
            for (size_t j = 0; j < 4; j++)
            {
                ss << v[i][j];
                if (i + j < 16)
                {
                    ss << ", ";
                }
            }
        }
    }
    else if (type == RtType::STRING || type == RtType::FILENAME)
    {
        ss << asString();
    }
    else if (type == RtType::TOKEN)
    {
        ss << asToken().str();
    }
    else
    {
        return EMPTY_STRING;
    }
    return ss.str();
}

}

#ifndef _WIN32
#pragma GCC diagnostic pop
#endif
