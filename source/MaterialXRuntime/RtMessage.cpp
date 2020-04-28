//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtMessage.h>

#include <MaterialXRuntime/Private/PvtApi.h>

namespace MaterialX
{

RtCallbackId RtMessage::addSetAttributeCallback(RtSetAttributeCallbackFunc callback, void* userData)
{
    return PvtApi::cast(RtApi::get())->getMessageHandler().addSetAttributeCallback(callback, userData);
}

RtCallbackId RtMessage::addMakeConnectionCallback(RtMakeConnectionCallbackFunc callback, void* userData)
{
    return PvtApi::cast(RtApi::get())->getMessageHandler().addMakeConnectionCallback(callback, userData);
}

RtCallbackId RtMessage::addBreakConnectionCallback(RtBreakConnectionCallbackFunc callback, void* userData)
{
    return PvtApi::cast(RtApi::get())->getMessageHandler().addBreakConnectionCallback(callback, userData);
}

void RtMessage::removeCallback(RtCallbackId id)
{
    PvtApi::cast(RtApi::get())->getMessageHandler().removeCallback(id);
}

}

