//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Look.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

const string MaterialAssign::MATERIAL_ATTRIBUTE = "material";
const string MaterialAssign::EXCLUSIVE_ATTRIBUTE = "exclusive";

const string Visibility::VIEWER_GEOM_ATTRIBUTE = "viewergeom";
const string Visibility::VIEWER_COLLECTION_ATTRIBUTE = "viewercollection";
const string Visibility::VISIBILITY_TYPE_ATTRIBUTE = "vistype";
const string Visibility::VISIBLE_ATTRIBUTE = "visible";

const string LookGroup::LOOKS_ATTRIBUTE = "looks";
const string LookGroup::ACTIVE_ATTRIBUTE = "active";

vector<MaterialAssignPtr> getGeometryBindings(const NodePtr& materialNode, const string& geom)
{
    vector<MaterialAssignPtr> matAssigns;
    for (LookPtr look : materialNode->getDocument()->getLooks())
    {
        for (MaterialAssignPtr matAssign : look->getMaterialAssigns())
        {
            if (matAssign->getReferencedMaterial() == materialNode)
            {
                if (geomStringsMatch(geom, matAssign->getActiveGeom()))
                {
                    matAssigns.push_back(matAssign);
                    continue;
                }
                CollectionPtr coll = matAssign->getCollection();
                if (coll && coll->matchesGeomString(geom))
                {
                    matAssigns.push_back(matAssign);
                    continue;
                }
            }
        }
    }
    return matAssigns;
}

//
// Look methods
//

MaterialAssignPtr Look::addMaterialAssign(const string& name, const string& material)
{
    MaterialAssignPtr matAssign = addChild<MaterialAssign>(name);
    if (!material.empty())
    {
        matAssign->setMaterial(material);
    }
    return matAssign;
}

vector<MaterialAssignPtr> Look::getActiveMaterialAssigns() const
{
    vector<MaterialAssignPtr> activeAssigns;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<MaterialAssignPtr> assigns = elem->asA<Look>()->getMaterialAssigns();
        activeAssigns.insert(activeAssigns.end(), assigns.begin(), assigns.end());
    }
    return activeAssigns;
}

vector<PropertyAssignPtr> Look::getActivePropertyAssigns() const
{
    vector<PropertyAssignPtr> activeAssigns;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<PropertyAssignPtr> assigns = elem->asA<Look>()->getPropertyAssigns();
        activeAssigns.insert(activeAssigns.end(), assigns.begin(), assigns.end());
    }
    return activeAssigns;
}

vector<PropertySetAssignPtr> Look::getActivePropertySetAssigns() const
{
    vector<PropertySetAssignPtr> activeAssigns;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<PropertySetAssignPtr> assigns = elem->asA<Look>()->getPropertySetAssigns();
        activeAssigns.insert(activeAssigns.end(), assigns.begin(), assigns.end());
    }
    return activeAssigns;
}

vector<VariantAssignPtr> Look::getActiveVariantAssigns() const
{
    vector<VariantAssignPtr> activeAssigns;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<VariantAssignPtr> assigns = elem->asA<Look>()->getVariantAssigns();
        activeAssigns.insert(activeAssigns.end(), assigns.begin(), assigns.end());
    }
    return activeAssigns;
}

vector<VisibilityPtr> Look::getActiveVisibilities() const
{
    vector<VisibilityPtr> activeVisibilities;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<VisibilityPtr> visibilities = elem->asA<Look>()->getVisibilities();
        activeVisibilities.insert(activeVisibilities.end(), visibilities.begin(), visibilities.end());
    }
    return activeVisibilities;
}

void Look::append(const LookPtr& source)
{
    for (auto child : source->getChildren())
    {
        if (!child)
        {
            continue;
        }
        string name = source->getName() + "_" + child->getName();

        ConstElementPtr previous = getChild(name);
        if (previous)
        {
            name = createValidChildName(name);
        }

        // Create the copied element.
        ElementPtr childCopy = addChildOfCategory(child->getCategory(), name);
        childCopy->copyContentFrom(child);
    }
}

//
// MaterialAssign methods
//

NodePtr MaterialAssign::getReferencedMaterial() const
{
    return resolveRootNameReference<Node>(getMaterial());
}

vector<VariantAssignPtr> MaterialAssign::getActiveVariantAssigns() const
{
    vector<VariantAssignPtr> activeAssigns;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<VariantAssignPtr> assigns = elem->asA<MaterialAssign>()->getVariantAssigns();
        activeAssigns.insert(activeAssigns.end(), assigns.begin(), assigns.end());
    }
    return activeAssigns;
}

//
// Lookgroup methods
//

LookVec LookGroup::getActiveLooks() const
{
    string looks = getActiveLook();
    if (looks.empty())
    {
        looks = getLooks();
    }
    const StringVec& lookList = splitString(looks, ",");
    LookVec activeLooks;
    if (!lookList.empty())
    {
        vector<LookPtr> lookElements = getDocument()->getLooks();
        if (!lookElements.empty())
        {
            for (auto lookName : lookList)
            {
                for (auto lookElement : lookElements)
                {
                    if (lookElement->getName() == lookName)
                    {
                        activeLooks.push_back(lookElement);
                    }
                }
            }
        }
    }
    return activeLooks;
}

// Need ability to replace active look here, or outside ?
// for default look want to set whether to use or not.
void LookGroup::append(const LookGroupPtr& sourceGroup, bool ignoreDuplicateLooks)
{
    const string COMMA_SEPERATOR = ",";

    string sourceLooks = sourceGroup->getLooks();
    if (sourceLooks.empty())
    {
        return;
    }
    const StringVec& sourceLookList = splitString(sourceLooks, ",");

    DocumentPtr doc = getDocument();

    StringVec destLookList = splitString(getLooks(), COMMA_SEPERATOR);
    const StringSet destLookSet(destLookList.begin(), destLookList.end());

    StringVec destActiveLookList = splitString(getActiveLook(), COMMA_SEPERATOR);

    for (auto lookName : sourceLookList)
    {
        string newLookName = lookName;
        if (destLookSet.count(lookName))
        {
            if (ignoreDuplicateLooks)
            {
                continue;
            }
            else
            {
                newLookName = doc->createValidChildName(lookName);
            }
        }

        destLookList.push_back(COMMA_SEPERATOR + newLookName);
        destActiveLookList.push_back(COMMA_SEPERATOR + newLookName);
    }

    setLooks(mergeStringVec(destLookList, COMMA_SEPERATOR));
    setActiveLook(mergeStringVec(destActiveLookList, COMMA_SEPERATOR));
}

LookPtr LookGroup::combineLooks() 
{
    DocumentPtr document = getDocument();
    LookVec looks = getActiveLooks();
    if (looks.empty())
    {
        return nullptr;
    }

    // Create a new look as a copy of the first look
    LookPtr mergedLook = document->addLook();
    mergedLook->copyContentFrom(looks[0]);

    // Merge in subsequent looks if any
    for (size_t i=1; i<looks.size(); i++)
    {
        mergedLook->append(looks[i]);
    }
    const string& mergedLookName = mergedLook->getName();
    setActiveLook(mergedLookName);
    setLooks(mergedLookName);

    return mergedLook;
}


} // namespace MaterialX
