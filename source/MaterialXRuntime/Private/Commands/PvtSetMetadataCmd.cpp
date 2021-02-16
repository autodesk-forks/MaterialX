//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtSetMetadataCmd.h>
#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtValue.h>

namespace MaterialX
{

PvtSetMetadataCmd::PvtSetMetadataCmd(RtObject& obj, const RtToken& name, const RtValue& value)
    : PvtCommand()
    , _obj(obj)
    , _name(name)
    , _value(value)
    , _oldValue()
    , _metadataCreated(false)
{
}

PvtCommandPtr PvtSetMetadataCmd::create(RtObject& obj, const RtToken& name, const RtValue& value)
{
    return std::make_shared<PvtSetMetadataCmd>(obj, name, value);
}

void PvtSetMetadataCmd::execute(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {
            // Send message that the metadata is changing
            msg().sendSetMetadataMessage(_obj, _name, _value);

            // Do we need to create the metadata or does it already exist?
            if (!_obj.getMetadata(_name))
            {
                _obj.addMetadata(_name, RtType::STRING);
                _metadataCreated = true;
            }

            // Save old value for undo/redo
            _oldValue = RtValue::clone(RtType::STRING, _obj.getMetadata(_name)->getValue(), _obj.getParent());

            // Set the metadata value
            RtTypedValue* md = _obj.getMetadata(_name);
            if (md)
            {
                md->setValue(_value);
                result = RtCommandResult(true);
            }
            else
            {
                result = RtCommandResult(false, "Unable to get metadata");
            }
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Node to set is no longer valid"));
    }
}

void PvtSetMetadataCmd::undo(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {
            // Send message that the attribute is changing
            msg().sendSetMetadataMessage(_obj, _name, _oldValue);

            if (_metadataCreated)
            {
                _obj.removeMetadata(_name);
				result = RtCommandResult(true);
            }
            else
            {
                // Reset the value
                RtTypedValue* md = _obj.getMetadata(_name);
                if (md)
                {
                    md->setValue(_oldValue);
                    result = RtCommandResult(true);
                }
                else
                {
                    result = RtCommandResult(false, "Unable to get metadata");
                }
            }
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Attribute to set is no longer valid"));
    }
}

void PvtSetMetadataCmd::redo(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {
            // Send message that the metadata is changing
            msg().sendSetMetadataMessage(_obj, _name, _value);

            // Do we need to create the metadata or does it already exist?
            if (!_obj.getMetadata(_name))
            {
                _obj.addMetadata(_name, RtType::STRING);
                _metadataCreated = true;
            }
            // Reset the value
            RtTypedValue* md = _obj.getMetadata(_name);
            if (md)
            {
                md->setValue(_value);
                result = RtCommandResult(true);
            }
            else
            {
                result = RtCommandResult(false, "Unable to get metadata");
            }
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Node to set is no longer valid"));
    }
}

}
