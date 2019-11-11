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

protected:
    PrvElement(RtObjType objType, const RtToken& name);

    void setName(const RtToken& name)
    {
        _name = name;
    }

    using AttrPtr = std::shared_ptr<RtAttribute>;

    RtToken _name;
    vector<AttrPtr> _attributes;
    RtTokenMap<AttrPtr> _attributesByName;
    friend class PrvStage;
};


class PrvCompound : public PrvElement
{
public:
    PrvCompound(RtObjType objType, const RtToken& name);

    virtual ~PrvCompound() {}

    void addElement(PrvObjectHandle elem);
    void removeElement(const RtToken& name);
    void clearElements();

    size_t numElements() const
    {
        return _elements.size();
    }

    PrvObjectHandle getElement(size_t index) const
    {
        return index < _elements.size() ? _elements[index] : nullptr;
    }

    const PrvObjectHandleVec& getElements() const
    {
        return _elements;
    }

    virtual PrvObjectHandle findElementByName(const RtToken& name) const;
    virtual PrvObjectHandle findElementByPath(const string& path) const;

    RtLargeValueStorage& getValueStorage()
    {
        return _storage;
    }

    static const string PATH_SEPARATOR;

protected:
    PrvObjectHandleVec _elements;
    RtTokenMap<PrvObjectHandle> _elementsByName;
    RtLargeValueStorage _storage;
};


class PrvUnknown : public PrvCompound
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
