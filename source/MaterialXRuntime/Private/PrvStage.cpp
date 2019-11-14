//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvStage.h>
#include <MaterialXRuntime/Private/PrvNodeDef.h>
#include <MaterialXRuntime/Private/PrvNode.h>
#include <MaterialXRuntime/Private/PrvNodeGraph.h>

#include <MaterialXRuntime/RtObject.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

PrvStage::PrvStage(const RtToken& name) :
    PrvValueStoringElement(RtObjType::STAGE, nullptr, name),
    _selfRefCount(0)
{
}

PrvObjectHandle PrvStage::createNew(const RtToken& name)
{
    return std::make_shared<PrvStage>(name);
}

void PrvStage::addReference(PrvObjectHandle stage)
{
    if (!stage->hasApi(RtApiType::STAGE))
    {
        throw ExceptionRuntimeError("Given object is not a valid stage");
    }
    if (_refStagesSet.count(stage))
    {
        throw ExceptionRuntimeError("Given object is not a valid stage");
    }

    stage->asA<PrvStage>()->_selfRefCount++;
    _refStages.push_back(stage);
}

void PrvStage::removeReference(const RtToken& name)
{
    for (auto it = _refStages.begin(); it != _refStages.end(); ++it)
    {
        PrvStage* stage = (*it)->asA<PrvStage>();
        if (stage->getName() == name)
        {
            stage->_selfRefCount--;
            _refStagesSet.erase(*it);
            _refStages.erase(it);
            break;
        }
    }
}

PrvObjectHandle PrvStage::findChildByName(const RtToken& name) const
{
    auto it = _childrenByName.find(name);
    if (it != _childrenByName.end())
    {
        return it->second;
    }
    for (auto rs : _refStages)
    {
        PrvStage* refStage = rs->asA<PrvStage>();
        PrvObjectHandle elem = refStage->findChildByName(name);
        if (elem)
        {
            return elem;
        }
    }
    return nullptr;
}

PrvObjectHandle PrvStage::findChildByPath(const string& path) const
{
    const StringVec elementNames = splitString(path, PATH_SEPARATOR);
    if (elementNames.empty())
    {
        return nullptr;
    }

    size_t i = 0;
    RtToken name(elementNames[i++]);
    PrvObjectHandle elem = findChildByName(name);

    while (elem && i < elementNames.size())
    {
        name = elementNames[i++];
        if (elem->getObjType() == RtObjType::NODE)
        {
            // For nodes find the portdef on the corresponding nodedef
            PrvNode* node = elem->asA<PrvNode>();
            PrvNodeDef* nodedef = node->getNodeDef()->asA<PrvNodeDef>();
            elem = nodedef->findChildByName(name);
        }
        else
        {
            elem = elem->asA<PrvElement>()->findChildByName(name);
        }
    }

    if (!elem || i < elementNames.size())
    {
        // The full path was not found so search
        // any referenced stages as well.
        for (auto it : _refStages)
        {
            PrvStage* refStage = it->asA<PrvStage>();
            elem = refStage->findChildByPath(path);
            if (elem)
            {
                break;
            }
        }
    }

    return elem;
}

}
