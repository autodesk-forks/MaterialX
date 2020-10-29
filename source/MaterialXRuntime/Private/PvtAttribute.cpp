//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtAttribute.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>

#include <MaterialXRuntime/RtConnectableApi.h>

namespace MaterialX
{

const RtToken PvtAttribute::DEFAULT_OUTPUT_NAME("out");
const RtToken PvtAttribute::COLOR_SPACE("colorspace");
const RtToken PvtAttribute::UNIT("unit");
const RtToken PvtAttribute::UNIT_TYPE("unittype");

RT_DEFINE_RUNTIME_OBJECT(PvtAttribute, RtObjType::ATTRIBUTE, "PvtAttribute")

PvtAttribute::PvtAttribute(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent) :
    PvtObject(name, parent),
    _value(type, RtValue::createNew(type, parent->prim())),
    _flags(flags)
{
    setTypeBit<PvtAttribute>();
}


RT_DEFINE_RUNTIME_OBJECT(PvtOutput, RtObjType::OUTPUT, "PvtOutput")

PvtOutput::PvtOutput(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent) :
    PvtAttribute(name, type, flags, parent)
{
    setTypeBit<PvtOutput>();
}

bool PvtOutput::isConnectable(const PvtInput* input) const
{
    // We cannot connect to ourselves.
    if (_parent == input->_parent)
    {
        return false;
    }

    // Our corresponding connectable APIs must accept the connection.
    RtConnectableApi* srcApi = RtConnectableApi::get(_parent->prim());
    RtConnectableApi* dstApi = RtConnectableApi::get(input->_parent->prim());
    bool accept = srcApi && srcApi->acceptConnection(hnd(), input->hnd()) &&
                  dstApi && dstApi->acceptConnection(hnd(), input->hnd());
    return accept;
}

void PvtOutput::connect(PvtInput* input)
{
    // Check if the connection exists already.
    if (input->_connection == hnd())
    {
        return;
    }

    // Check if another connection exists already.
    if (input->isConnected())
    {
        throw ExceptionRuntimeError("Input '" + input->getPath().asString() + "' is already connected");
    }

    // Check with the corresponding connectable APIs if they accept the connection.
    RtConnectableApi* srcApi = RtConnectableApi::get(_parent->prim());
    if (!(srcApi && srcApi->acceptConnection(hnd(), input->hnd())))
    {
        throw ExceptionRuntimeError("Output '" + getPath().asString() + "' rejected the connection");
    }
    RtConnectableApi* dstApi = RtConnectableApi::get(input->_parent->prim());
    if (!(dstApi && dstApi->acceptConnection(hnd(), input->hnd())))
    {
        throw ExceptionRuntimeError("Input '" + input->getPath().asString() + "' rejected the connection");
    }

    // Make the connection.
    _connections.push_back(input->hnd());
    input->_connection = hnd();
}

void PvtOutput::disconnect(PvtInput* input)
{
    // Check if the connection exists.
    if (!input->_connection || input->_connection.get() != this)
    {
        return;
    }

    // Break the connection.
    input->_connection = nullptr;
    for (auto it = _connections.begin(); it != _connections.end(); ++it)
    {
        if (it->get() == input)
        {
            _connections.erase(it);
            break;
        }
    }
}

void PvtOutput::clearConnections()
{
    // Break connections to all destination inputs.
    for (PvtDataHandle destH : _connections)
    {
        destH->asA<PvtInput>()->_connection = nullptr;
    }
    _connections.clear();
}


RT_DEFINE_RUNTIME_OBJECT(PvtInput, RtObjType::INPUT, "PvtInput")

PvtInput::PvtInput(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent) :
    PvtAttribute(name, type, flags, parent)
{
    setTypeBit<PvtInput>();
}

void PvtInput::clearConnection()
{
    if (_connection)
    {
        // Break connection to our single source output.
        PvtOutput* source = _connection->asA<PvtOutput>();
        for (auto it = source->_connections.begin(); it != source->_connections.end(); ++it)
        {
            if (it->get() == this)
            {
                source->_connections.erase(it);
                break;
            }
        }
        _connection = nullptr;
    }
}

}
