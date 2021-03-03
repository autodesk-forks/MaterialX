//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtSetAttributeCmd.h>
#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtValue.h>

namespace MaterialX
{

PvtSetAttributeCmd::PvtSetAttributeCmd(const RtObject& obj, const RtToken& name, const RtValue& value)
    : PvtCommand()
    , _obj(obj)
    , _name(name)
    , _value(value)
    , _oldValue()
    , _metadataCreated(false)
{
}

PvtCommandPtr PvtSetAttributeCmd::create(const RtObject& obj, const RtToken& name, const RtValue& value)
{
    return std::make_shared<PvtSetAttributeCmd>(obj, name, value);
}

void PvtSetAttributeCmd::execute(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {
            // Send message that the attribute is changing
            msg().sendSetAttributeMessage(_obj, _name, _value);

            RtTypedValue* md = _obj.getAttribute(_name, RtType::STRING);

            // Do we need to create the attribute or does it already exist?
            if (!md)
            {
                md = _obj.getAttribute(_name, RtType::STRING);
                _metadataCreated = true;
            }

            // Save old value for undo/redo
            _oldValue = RtValue::clone(RtType::STRING, md->getValue(), _obj.getParent());

            md->setValue(_value);
            result = RtCommandResult(true);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Object to set attribute on is no longer valid"));
    }
}

void PvtSetAttributeCmd::undo(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {

            if (_metadataCreated)
            {
                // Send message that the attribute is being removed
                msg().sendRemoveAttributeMessage(_obj, _name);

                _obj.removeAttribute(_name);
                _metadataCreated = false;
                result = RtCommandResult(true);
            }
            else
            {
                // Send message that the attribute is changing
                msg().sendSetAttributeMessage(_obj, _name, _oldValue);

                // Reset the value
                RtTypedValue* md = _obj.getAttribute(_name, RtType::STRING);
                if (md)
                {
                    md->setValue(_oldValue);
                    result = RtCommandResult(true);
                }
                else
                {
                    result = RtCommandResult(false, "Attribute is no longer valid");
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
        result = RtCommandResult(false, string("Object to set attribute on is no longer valid"));
    }
}

void PvtSetAttributeCmd::redo(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {
            // Send message that the attribute is changing
            msg().sendSetAttributeMessage(_obj, _name, _value);

            RtTypedValue* md = _obj.getAttribute(_name, RtType::STRING);

            // Do we need to create the attribute or does it already exist?
            if (!md)
            {
                md = _obj.getAttribute(_name, RtType::STRING);
                _metadataCreated = true;
            }

            // Reset the value
            md->setValue(_value);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Object to set attribute on is no longer valid"));
    }
}

}
