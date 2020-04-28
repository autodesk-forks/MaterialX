//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtMessage.h>

namespace MaterialX
{

PvtMessageHandler::PvtMessageHandler() :
    _callbackIdCounter(1)
{}

RtCallbackId PvtMessageHandler::addSetAttributeCallback(RtSetAttributeCallbackFunc callback, void* userData)
{
    PvtSetAttributeObserver observer = PvtSetAttributeObserver(callback, userData);
    _setAttrObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

RtCallbackId PvtMessageHandler::addMakeConnectionCallback(RtMakeConnectionCallbackFunc callback, void* userData)
{
    PvtMakeConnectionObserver observer = PvtMakeConnectionObserver(callback, userData);
    _makeConnectionObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

RtCallbackId PvtMessageHandler::addBreakConnectionCallback(RtBreakConnectionCallbackFunc callback, void* userData)
{
    PvtBreakConnectionObserver observer = PvtBreakConnectionObserver(callback, userData);
    _breakConnectionObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

void PvtMessageHandler::removeCallback(RtCallbackId id)
{
    auto it = _callbackIdToType.find(id);
    if (it != _callbackIdToType.end())
    {
        // Remove from the message type map
        _callbackIdToType.erase(id);

        // Remove the corresponding observer
        switch (it->second)
        {
        case PvtMessageType::SET_ATTRIBUTE:
            _setAttrObservers.erase(id);
            break;
        case PvtMessageType::MAKE_CONNECTION:
            _makeConnectionObservers.erase(id);
            break;
        case PvtMessageType::BREAK_CONNECTION:
            _breakConnectionObservers.erase(id);
            break;
        }
    }
}

}
