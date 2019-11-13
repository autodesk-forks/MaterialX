//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PRVELEMENT_H
#define MATERIALX_PRVELEMENT_H

#include <MaterialXRuntime/Private/PrvObject.h>

#include <MaterialXRuntime/RtElement.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

using PrvObjectHandleVec = vector<PrvObjectHandle>;
using PrvObjectHandleSet = std::set<PrvObjectHandle>;

class PrvElement : public PrvObject
{
public:
    virtual ~PrvElement() {}

    const RtToken& getName() const
    {
        return _name;
    }

    void addChild(PrvObjectHandle elem);

    void removeChild(const RtToken& name);

    size_t numChildren() const
    {
        return _children.size();
    }

    PrvObjectHandle getChild(size_t index) const
    {
        return index < _children.size() ? _children[index] : nullptr;
    }

    const PrvObjectHandleVec& getChildren() const
    {
        return _children;
    }

    void clearChildren();

    virtual PrvObjectHandle findChildByName(const RtToken& name) const;

    virtual PrvObjectHandle findChildByPath(const string& path) const;

    void addAttribute(const RtToken& name, const RtToken& type, const RtValue& value);

    void removeAttribute(const RtToken& name);

    void clearAttributes();

    const RtAttribute* getAttribute(const RtToken& name) const
    {
        auto it = _attributesByName.find(name);
        return it != _attributesByName.end() ? it->second.get() : nullptr;
    }

    RtAttribute* getAttribute(const RtToken& name)
    {
        auto it = _attributesByName.find(name);
        return it != _attributesByName.end() ? it->second.get() : nullptr;
    }

    const RtAttribute* getAttribute(size_t index) const
    {
        return index < _attributes.size() ? _attributes[index].get() : nullptr;
    }

    RtAttribute* getAttribute(size_t index)
    {
        return index < _attributes.size() ? _attributes[index].get() : nullptr;
    }

    size_t numAttributes() const
    {
        return _attributes.size();
    }

    static const string PATH_SEPARATOR;

protected:
    PrvElement(RtObjType objType, const RtToken& name);

    void setName(const RtToken& name)
    {
        _name = name;
    }

    RtToken _name;

    PrvObjectHandleVec _children;
    RtTokenMap<PrvObjectHandle> _childrenByName;

    using AttrPtr = std::shared_ptr<RtAttribute>;

    vector<AttrPtr> _attributes;
    RtTokenMap<AttrPtr> _attributesByName;

    friend class PrvStage;
};


class PrvValueStoringElement : public PrvElement
{
public:
    RtLargeValueStorage& getValueStorage()
    {
        return _storage;
    }

protected:
    PrvValueStoringElement(RtObjType objType, const RtToken& name):
        PrvElement(objType, name)
    {}

    RtLargeValueStorage _storage;
};


class PrvUnknown : public PrvElement
{
public:
    PrvUnknown(const RtToken& name, const RtToken& category);

    static PrvObjectHandle createNew(const RtToken& name, const RtToken& category);

    const RtToken& getCategory() const
    {
        return _category;
    }

private:
    const RtToken _category;
};

}

#endif
