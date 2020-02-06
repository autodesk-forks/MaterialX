//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTNAMERESOLVER_H
#define MATERIALX_PVTNAMERESOLVER_H

#include <MaterialXRuntime/RtNameResolver.h>

#include <MaterialXCore/Element.h>

#include <unordered_map>

/// @file
/// TODO: Docs

namespace MaterialX
{

/// Map of token to RtStringResolverPairPtr
using RtStringResolverMap = RtTokenMap<RtStringResolverPairPtr>;

class PvtStringResolverPair
{
  public:
    /// Constructor
    PvtStringResolverPair(const RtToken& type,
                          MaterialX::StringResolverPtr toMaterialXResolver,
                          MaterialX::StringResolverPtr fromMaterialXResolver);

    /// \brief Returns the type of element resolved by the pair of resolvers
    /// \return String type of element resolved by the pair of resolvers
    const RtToken getType() const;

    /// \brief Returns the resolver used to convert string identifiers to MaterialX document format
    /// \return StringResolverPtr to the resolver used to convert string identifiers to MaterialX document format
    MaterialX::StringResolverPtr getToMaterialXResolver();

    /// \brief Returns the resolver used to convert string identifiers from MaterialX document format
    /// \return StringResolverPtr to the resolver used to convert string identifiers from MaterialX document format
    MaterialX::StringResolverPtr getFromMaterialXResolver();

  private:
    const RtToken _type;
    MaterialX::StringResolverPtr _toMaterialXResolver;
    MaterialX::StringResolverPtr _fromMaterialXResolver;
};

/// @class PvtNameResolverRegistrar
class PvtNameResolverRegistrar
{
public:
    /// Constructor
    PvtNameResolverRegistrar();

    /// \brief Registers a pair of string resolvers for resolving scene identifiers to/from MaterialX
    /// \param nameResolverContext Name resolution information
    void registerNameResolvers(RtNameResolverInfo& info);

    /// \brief Deregisters a named pair of string resolvers
    /// \param name The name given to the pair of identifiers to deregister
    void deregisterNameResolvers(const RtToken& name);

    /// \brief Resolves the provided string. Any registered custom resolvers are applied to determine
    /// the final resolved value. The resolvers are applied in the order that they are registered.
    ///
    /// \param valueToResolve The value to resolve
    /// \param elementType The type of element to resolve
    /// \param toMaterialX Whether to convert to/from MaterialX
    RtToken resolveIdentifier(const RtToken& valueToResolve, const RtNameResolverInfo::ElementType elementType, bool toMaterialX = true) const;                                              

private:
    RtStringResolverMap _resolvers;
};

}

#endif

