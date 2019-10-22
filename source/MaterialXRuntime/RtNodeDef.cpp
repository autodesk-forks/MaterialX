//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtObject.h>

#include <MaterialXRuntime/Private/PrvNodeDef.h>
#include <MaterialXRuntime/Private/PrvStage.h>

namespace MaterialX
{

RtNodeDef::RtNodeDef(const RtObject& obj) :
    RtElement(obj)
{
}

RtObject RtNodeDef::createNew(const RtToken& name, const RtToken& category, RtObject parent)
{
    PrvObjectHandle nodedef = PrvNodeDef::createNew(name, category);

    if (parent)
    {
        if (!parent.hasApi(RtApiType::STAGE))
        {
            throw ExceptionRuntimeError("Given parent object is not a valid stage");
        }
        parent.data()->asA<PrvStage>()->addElement(nodedef);
    }

    return RtObject(nodedef);
}

RtApiType RtNodeDef::getApiType() const
{
    return RtApiType::NODEDEF;
}

const RtToken& RtNodeDef::getCategory() const
{
    return data()->asA<PrvNodeDef>()->getCategory();
}

void RtNodeDef::addPort(RtObject portdef)
{
    return data()->asA<PrvNodeDef>()->addPort(portdef.data());
}

size_t RtNodeDef::numPorts() const
{
    return data()->asA<PrvNodeDef>()->numElements();
}

size_t RtNodeDef::numOutputs() const
{
    return data()->asA<PrvNodeDef>()->numOutputs();
}

RtObject RtNodeDef::getPort(size_t index) const
{
    PrvObjectHandle portdef = data()->asA<PrvNodeDef>()->getElement(index);
    return RtObject(portdef);
}

RtObject RtNodeDef::findPort(const RtToken& name) const
{
    PrvObjectHandle portdef = data()->asA<PrvNodeDef>()->findElementByName(name);
    return RtObject(portdef);
}

}
