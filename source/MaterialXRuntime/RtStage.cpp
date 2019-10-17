//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtStage.h>

#include <MaterialXRuntime/Private/PrvStage.h>

namespace MaterialX
{

RtStage::RtStage(const RtObject& obj) :
    RtElement(obj)
{
}

RtApiType RtStage::getApiType() const
{
    return RtApiType::STAGE;
}

RtObject RtStage::create(const RtToken& name)
{
    return RtObject(PrvStage::create(name));
}

void RtStage::addReference(RtObject stage)
{
    data()->asA<PrvStage>()->addReference(RtApiBase::data(stage));
}

void RtStage::removeReference(const RtToken& name)
{
    data()->asA<PrvStage>()->removeReference(name);
}

void RtStage::addElement(RtObject elem)
{
    if (elem.hasApi(RtApiType::STAGE))
    {
        throw ExceptionRuntimeError("A stage cannot be added as direct child of another stage. Use addReference() instead to reference the stage.");
    }
    data()->asA<PrvStage>()->addElement(RtApiBase::data(elem));
}

void RtStage::removeElement(const RtToken& name)
{
    data()->asA<PrvStage>()->removeElement(name);
}

RtObject RtStage::findElementByName(const RtToken& name) const
{
    PrvObjectHandle elem = data()->asA<PrvStage>()->findElementByName(name);
    return RtObject(elem);
}

RtObject RtStage::findElementByPath(const string& path) const
{
    PrvObjectHandle elem = data()->asA<PrvStage>()->findElementByPath(path);
    return RtObject(elem);
}

RtStageIterator RtStage::traverseStage(RtTraversalFilter filter)
{
    return RtStageIterator(getObject(), filter);
}

}
