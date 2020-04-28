//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTMESSAGE_H
#define MATERIALX_RTMESSAGE_H

/// @file
/// Classes for notification of data model changes.

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtTypeInfo.h>
#include <MaterialXRuntime/RtAttribute.h>
#include <MaterialXRuntime/RtRelationship.h>

namespace MaterialX
{

class RtAttrIterator;
class RtRelationshipIterator;
class RtPrimIterator;
class RtSchemaBase;


using RtCallbackId = size_t;

/// Function type for notifying when an attribute valus is set.
using RtSetAttributeCallbackFunc = std::function<void(const RtAttribute& attr, const RtValue& value, void* userData)>;

/// Function type for notifying when a connection is made.
using RtMakeConnectionCallbackFunc = std::function<void(const RtOutput& src, const RtInput& dest, void* userData)>;

/// Function type for notifying when a connection is broken.
using RtBreakConnectionCallbackFunc = std::function<void(const RtOutput& src, const RtInput& dest, void* userData)>;


/// @class RtMessage
class RtMessage
{
public:
    static RtCallbackId addSetAttributeCallback(RtSetAttributeCallbackFunc callback, void* userData = nullptr);

    static RtCallbackId addMakeConnectionCallback(RtMakeConnectionCallbackFunc callback, void* userData = nullptr);

    static RtCallbackId addBreakConnectionCallback(RtBreakConnectionCallbackFunc callback, void* userData = nullptr);

    static void removeCallback(RtCallbackId id);
};

}

#endif
