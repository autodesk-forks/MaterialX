//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtSchema.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

RtSchemaBase::RtSchemaBase(const RtPrim& prim) :
    _hnd(prim._hnd)
{
}

RtSchemaBase::RtSchemaBase(const RtSchemaBase& other) :
    _hnd(other._hnd)
{
}

bool RtSchemaBase::isSupported(const RtPrim& prim) const
{
    return prim && isSupported(prim.hnd());
}

bool RtSchemaBase::isSupported(const PvtDataHandle&) const
{
    return true;
}

RtPrim RtSchemaBase::getPrim() const
{
    return RtPrim(_hnd);
}

PvtPrim* RtSchemaBase::prim() const
{
    return hnd()->asA<PvtPrim>();
}

PvtAttribute* RtSchemaBase::attr(const RtToken& name) const
{
    return prim()->getAttribute(name);
}

PvtRelationship* RtSchemaBase::rel(const RtToken& name) const
{
    return prim()->getRelationship(name);
}


bool RtTypedSchema::isSupported(const PvtDataHandle& hnd) const
{
    // TODO: Implement proper type information and checking
    // to handle schema class inheritance.
    return !hnd->isDisposed() && getTypeInfo().isCompatible(hnd->asA<PvtPrim>()->getTypeName());
}


namespace
{
    const string TYPE_NAME_SEPARATOR = ":";
}

using PvtTypeNameSet = RtTokenSet;

struct PvtTypeNameInfo
{
    RtToken shortName;
    RtToken longName;
    RtTokenVec baseNames;
    RtTokenSet allNames;
};

RtTypeInfo::RtTypeInfo(const char* typeNameHierachy) :
    _ptr(nullptr)
{
    PvtTypeNameInfo* info = new PvtTypeNameInfo();

    const RtToken longName(typeNameHierachy);
    StringVec names = splitString(longName, TYPE_NAME_SEPARATOR);
    info->shortName = names.back();
    info->longName = longName;
    for (size_t i = 0; i < names.size() - 1; ++i)
    {
        const RtToken name(names[i]);
        info->baseNames.push_back(name);
        info->allNames.insert(name);
    }
    info->allNames.insert(info->shortName);

    _ptr = info;
}

RtTypeInfo::~RtTypeInfo()
{
    delete static_cast<PvtTypeNameInfo*>(_ptr);
}

const RtToken& RtTypeInfo::getShortTypeName() const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->shortName;
}

const RtToken& RtTypeInfo::getLongTypeName() const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->longName;
}

size_t RtTypeInfo::numBaseClasses() const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->baseNames.size();
}

const RtToken& RtTypeInfo::getBaseClassType(size_t index) const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->baseNames[index];
}

bool RtTypeInfo::isCompatible(const RtToken& typeName) const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->allNames.count(typeName) > 0;
}

}
