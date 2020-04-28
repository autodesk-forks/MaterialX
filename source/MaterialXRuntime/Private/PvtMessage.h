//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTMESSAGE_H
#define MATERIALX_PVTMESSAGE_H

#include <MaterialXRuntime/RtMessage.h>

namespace MaterialX
{

enum class PvtMessageType
{
    SET_ATTRIBUTE,
    MAKE_CONNECTION,
    BREAK_CONNECTION,
    NUM_TYPES,
};

template<typename T>
using PvtCallbackIdMap = std::unordered_map<RtCallbackId, T>;

template<PvtMessageType TYPE, typename Callback>
struct PvtObserver
{
    PvtMessageType type;
    Callback callback;
    void* userData;

    PvtObserver() : type(PvtMessageType::NUM_TYPES), callback(nullptr), userData(nullptr) {}
    PvtObserver(Callback cb, void* data) : type(TYPE), callback(cb), userData(data) {}
};

using PvtSetAttributeObserver = PvtObserver<PvtMessageType::SET_ATTRIBUTE, RtSetAttributeCallbackFunc>;
using PvtMakeConnectionObserver = PvtObserver<PvtMessageType::MAKE_CONNECTION, RtMakeConnectionCallbackFunc>;
using PvtBreakConnectionObserver = PvtObserver<PvtMessageType::BREAK_CONNECTION, RtBreakConnectionCallbackFunc>;

class PvtMessageHandler
{
public:
    PvtMessageHandler();

    RtCallbackId addSetAttributeCallback(RtSetAttributeCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addMakeConnectionCallback(RtMakeConnectionCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addBreakConnectionCallback(RtBreakConnectionCallbackFunc callback, void* userData = nullptr);

    void removeCallback(RtCallbackId id);

private:
    RtCallbackId _callbackIdCounter;
    PvtCallbackIdMap<PvtMessageType> _callbackIdToType;
    PvtCallbackIdMap<PvtSetAttributeObserver> _setAttrObservers;
    PvtCallbackIdMap<PvtMakeConnectionObserver> _makeConnectionObservers;
    PvtCallbackIdMap<PvtBreakConnectionObserver> _breakConnectionObservers;
};

}

#endif
