//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNameResolver.h>

#include <MaterialXRuntime/Private/PvtNameResolver.h>

namespace MaterialX
{

RtStringResolverPairPtr RtStringResolverPair::createNew(const RtNameResolverInfo::ElementType elementType,
                                                        MaterialX::StringResolverPtr toMaterialXResolver,
                                                        MaterialX::StringResolverPtr fromMaterialXResolver)
{
    RtStringResolverPairPtr result(new RtStringResolverPair(elementType, toMaterialXResolver, fromMaterialXResolver));
    return result;
}

const RtNameResolverInfo::ElementType RtStringResolverPair::getType() const
{
    return _stringResolverPair->getType();
}

MaterialX::StringResolverPtr RtStringResolverPair::getToMaterialXResolver()
{
    return _stringResolverPair->getToMaterialXResolver();
}

MaterialX::StringResolverPtr RtStringResolverPair::getFromMaterialXResolver()
{
    return _stringResolverPair->getFromMaterialXResolver();
}

RtStringResolverPair::RtStringResolverPair(const RtNameResolverInfo::ElementType elementType,
                                           MaterialX::StringResolverPtr toMaterialXResolver,
                                           MaterialX::StringResolverPtr fromMaterialXResolver)
    : _stringResolverPair(new PvtStringResolverPair(elementType, toMaterialXResolver, fromMaterialXResolver))
{
}

RtNameResolverRegistrarPtr RtNameResolverRegistrar::createNew()
{
    RtNameResolverRegistrarPtr result(new RtNameResolverRegistrar());
    return result;
}

RtNameResolverRegistrar::RtNameResolverRegistrar()
    : _nameResolverRegistrar(new PvtNameResolverRegistrar())
{
}

void RtNameResolverRegistrar::registerNameResolvers(RtNameResolverInfo& info)
{
    _nameResolverRegistrar->registerNameResolvers(info);
}

void RtNameResolverRegistrar::deregisterNameResolvers(const RtToken& name)
{
    _nameResolverRegistrar->deregisterNameResolvers(name);
}

RtToken RtNameResolverRegistrar::resolveIdentifier(const RtToken& valueToResolve, const RtNameResolverInfo::ElementType elementType, bool toMaterialX) const
{
    return _nameResolverRegistrar->resolveIdentifier(valueToResolve, elementType, toMaterialX);
}

}
