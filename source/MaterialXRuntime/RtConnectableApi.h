//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTCONNECTABLEAPI_H
#define MATERIALX_RTCONNECTABLEAPI_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtAttribute.h>
#include <MaterialXRuntime/RtRelationship.h>

/// @file
/// TODO: Docs

namespace MaterialX
{


class RtConnectableApi;
using RtConnectableApiPtr = shared_ptr<RtConnectableApi>;

/// @class RtConnectableApi
/// API for validating attribute connections and relationships for a prim type.
class RtConnectableApi
{
public:
    /// Destructor.
    virtual ~RtConnectableApi() {}

    /// Return true if a connection is allowed to be made for the given source and destination.
    /// Default implementation accepts all connections and this method should be overridden by
    /// derived classes to change this.
    virtual bool acceptConnection(const RtOutput& src, const RtInput& dest) const;

    /// Return true if a connection is allowed to be made for the given relationsship and target object.
    /// Default implementation accepts all relationships for all targets and this method should be
    /// overridden by derived classes to change this.
    virtual bool acceptRelationship(const RtRelationship& rel, const RtObject& target) const;

    /// Register a connectable API for a given prim typename.
    static void registerApi(const RtToken& typeName, const RtConnectableApiPtr& api);

    /// Unregister a connectable API for a given prim typename.
    static void unregisterApi(const RtToken& typeName);

    /// Register a connectable API for a given typed prim schema.
    template<class PrimType, class Api = RtConnectableApi>
    inline static void registerApi()
    {
        registerApi(PrimType::typeName(), shared_ptr<Api>(new Api));
    }

    /// Unregister a connectable API for a given typed prim schema.
    template<class PrimType>
    inline static void unregisterApi()
    {
        unregisterApi(PrimType::typeName());
    }

    /// Return a connectable API for a given prim.
    static RtConnectableApi* get(const RtPrim& prim);
};

}

#endif
