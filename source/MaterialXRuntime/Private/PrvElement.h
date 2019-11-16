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

class PrvAllocator
{
public:
    ~PrvAllocator()
    {
        free();
    }

    // Allocate and return a block of data.
    uint8_t* alloc(size_t size)
    {
        uint8_t* ptr = new uint8_t[size];
        _storage.push_back(ptr);
        return ptr;
    }

    // Allocate and and return a single object of templated type.
    // The object constructor is called to initialize it.
    template<class T>
    T* allocType()
    {
        uint8_t* buffer = alloc(sizeof(T));
        return new (buffer) T();
    }

    // Free all allocated data.
    void free()
    {
        for (uint8_t* ptr : _storage)
        {
            delete[] ptr;
        }
        _storage.clear();
    }

private:
    std::vector<uint8_t*> _storage;
};


class PrvElement : public PrvObject
{
public:
    virtual ~PrvElement() {}

    const RtToken& getName() const
    {
        return _name;
    }

    PrvElement* getParent() const
    {
        return _parent;
    }

    PrvElement* getRoot() const;

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

    virtual PrvObjectHandle findChildByName(const RtToken& name) const;

    virtual PrvObjectHandle findChildByPath(const string& path) const;

    RtAttribute* addAttribute(const RtToken& name, const RtToken& type);

    void removeAttribute(const RtToken& name);

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

    virtual PrvAllocator& getAllocator();

    static const string PATH_SEPARATOR;

protected:
    PrvElement(RtObjType objType, PrvElement* parent, const RtToken& name);

    void setName(const RtToken& name)
    {
        _name = name;
    }

    PrvElement* _parent;
    RtToken _name;
    PrvObjectHandleVec _children;
    RtTokenMap<PrvObjectHandle> _childrenByName;

    using AttrPtr = std::shared_ptr<RtAttribute>;
    vector<AttrPtr> _attributes;
    RtTokenMap<AttrPtr> _attributesByName;

    friend class PrvStage;
};


class PrvAllocatingElement : public PrvElement
{
public:
    PrvAllocator& getAllocator() override
    {
        return _allocator;
    }

protected:
    PrvAllocatingElement(RtObjType objType, PrvElement* parent, const RtToken& name):
        PrvElement(objType, parent, name)
    {}

    PrvAllocator _allocator;
};


class PrvUnknownElement : public PrvElement
{
public:
    PrvUnknownElement(PrvElement* parent, const RtToken& name, const RtToken& category);

    static PrvObjectHandle createNew(PrvElement* parent, const RtToken& name, const RtToken& category);

    const RtToken& getCategory() const
    {
        return _category;
    }

private:
    const RtToken _category;
};

}

#endif
