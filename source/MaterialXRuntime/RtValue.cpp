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

RtValue RtValue::createNew(const RtToken& type, RtLargeValueStorage& store)
{
    const RtTypeDef* typeDef = RtTypeDef::findType(type);
    if (!typeDef)
    {
        throw ExceptionRuntimeError("Type '" + type.str() + "' is not a registered type");
    }
    return typeDef->createValue(store);
}

void RtValue::marshal(const RtToken& type, const RtValue& src, string& dest)
{
    const RtTypeDef* typeDef = RtTypeDef::findType(type);
    if (!typeDef)
    {
        throw ExceptionRuntimeError("Type '" + type.str() + "' is not a registered type");
    }
    typeDef->marshalValue(src, dest);
}

void RtValue::unmarshal(const RtToken& type, const string& src, RtValue& dest)
{
    const RtTypeDef* typeDef = RtTypeDef::findType(type);
    if (!typeDef)
    {
        throw ExceptionRuntimeError("Type '" + type.str() + "' is not a registered type");
    }
    typeDef->unmarshalValue(src, dest);
}

void RtValue::copy(const RtToken& type, const RtValue& src, RtValue& dest)
{
    const RtTypeDef* typeDef = RtTypeDef::findType(type);
    if (!typeDef)
    {
        throw ExceptionRuntimeError("Type '" + type.str() + "' is not a registered type");
    }
    typeDef->copyValue(src, dest);
}

}

#ifndef _WIN32
#pragma GCC diagnostic pop
#endif
