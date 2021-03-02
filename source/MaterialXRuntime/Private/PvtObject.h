//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTOBJECT_H
#define MATERIALX_PVTOBJECT_H

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtToken.h>
#include <MaterialXRuntime/RtValue.h>

#include <MaterialXRuntime/Private/PvtPath.h>

#include <unordered_map>
#include <set>
#include <atomic>

/// @file
/// TODO: Docs

namespace MaterialX
{

// Class representing an object in the scene hierarchy.
// This is the base class for prims, attributes and relationships.
class PvtObject : public RtRefCounted<PvtObject>
{
    RT_DECLARE_RUNTIME_OBJECT(PvtObject)

public:
    using TypeBits = uint8_t;

public:
    virtual ~PvtObject() {}

    bool isDisposed() const
    {
        return (_typeBits & TypeBits(RtObjType::DISPOSED)) != 0;
    }

    void setDisposed(bool state)
    {
        if (state)
        {
            _typeBits |= TypeBits(RtObjType::DISPOSED);
        }
        else
        {
            _typeBits &= ~TypeBits(RtObjType::DISPOSED);
        }
    }

    bool isCompatible(RtObjType objType) const
    {
        return ((_typeBits & TypeBits(objType)) &
            ~TypeBits(RtObjType::DISPOSED)) != 0;
    }

    /// Return true if this object is of the templated type.
    template<class T>
    bool isA() const
    {
        static_assert(std::is_base_of<PvtObject, T>::value,
            "Templated type must be an PvtObject or a subclass of PvtObject");
        return isCompatible(T::classType());
    }

    // Casting the object to a given type.
    // NOTE: In release builds no type check is performed so the templated type 
    // must be of a type compatible with this object.
    template<class T> T* asA()
    {
        static_assert(std::is_base_of<PvtObject, T>::value,
            "Templated type must be an PvtObject or a subclass of PvtObject");
// TODO: We enable these runtime checks for all build configurations for now,
//       but disabled this later to avoid the extra cost in release builds.
// #ifndef NDEBUG
        // In debug mode we do safety checks on object validity
        // and type cast compatibility.
        if (isDisposed())
        {
            throw ExceptionRuntimeError("Trying to access a disposed object '" + getName().str() + "'");
        }
        if (!isCompatible(T::classType()))
        {
            throw ExceptionRuntimeError("Types are incompatible for type cast, '" + getName().str() + "' is not a '" + T::className().str() + "'");
        }
// #endif
        return static_cast<T*>(this);
    }

    // Casting the object to a given type.
    // NOTE: In release builds no type check is performed so the templated type 
    // must be of a type compatible with this object.
    template<class T> const T* asA() const
    {
        return const_cast<PvtObject*>(this)->asA<T>();
    }

    // Return a handle for the object.
    PvtDataHandle hnd() const
    {
        return PvtDataHandle(const_cast<PvtObject*>(this));
    }

    // Return a handle for the given object.
    static PvtDataHandle hnd(const RtObject& obj)
    {
        return obj.hnd();
    }

    // Return an RtObject for this object.
    RtObject obj() const
    {
        return RtObject(hnd());
    }

    // Retreive a raw pointer to the private data of an RtObject.
    // NOTE: No type check is performed so the templated type 
    // must be a type supported by the object.
    template<class T>
    static T* ptr(const RtObject& obj)
    {
        return static_cast<T*>(obj.hnd().get());
    }

    const RtToken& getName() const
    {
        return _name;
    }

    PvtPath getPath() const;

    PvtPrim* getParent() const
    {
        return _parent;
    }

    PvtPrim* getRoot() const;

    RtStageWeakPtr getStage() const;

    RtTypedValue* addMetadata(const RtToken& name, const RtToken& type);

    void removeMetadata(const RtToken& name);

    // Get metadata without a type check.
    const RtTypedValue* getMetadata(const RtToken& name) const
    {
        auto it = _metadataMap.find(name);
        return it != _metadataMap.end() ? &it->second : nullptr;
    }

    // Get metadata without a type check.
    RtTypedValue* getMetadata(const RtToken& name)
    {
        auto it = _metadataMap.find(name);
        return it != _metadataMap.end() ? &it->second : nullptr;
    }

    // Get metadata with type check.
    RtTypedValue* getMetadata(const RtToken& name, const RtToken& type);

    // Get metadata with type check.
    const RtTypedValue* getMetadata(const RtToken& name, const RtToken& type) const
    {
        return const_cast<PvtObject*>(this)->getMetadata(name, type);
    }

    // For serialization to file we need the order.
    const vector<RtToken>& getMetadataOrder() const
    {
        return _metadataOrder;
    }

protected:
    PvtObject(const RtToken& name, PvtPrim* parent);

    template<typename T>
    void setTypeBit()
    {
        _typeBits |= TypeBits(T::classType());
    }

    // Protected as arbitrary renaming is not supported.
    // Must be done from the owning stage.
    void setName(const RtToken& name)
    {
        _name = name;
    }

    // Protected as arbitrary reparenting is not supported.
    // Must be done from the owning stage.
    void setParent(PvtPrim* parent)
    {
        _parent = parent;
    }

    TypeBits _typeBits;
    RtToken _name; // TODO: Store a path instead of name token
    PvtPrim* _parent;
    RtTokenMap<RtTypedValue> _metadataMap;
    vector<RtToken> _metadataOrder;

    friend class PvtPrim;
    friend class PvtPort;
    friend class PvtInput;
    friend class PvtOutput;
    RT_FRIEND_REF_PTR_FUNCTIONS(PvtObject)
};


using PvtDataHandleVec = vector<PvtDataHandle>;
using PvtDataHandleMap = RtTokenMap<PvtDataHandle>;
using PvtDataHandleSet = std::set<PvtDataHandle>;

// Data handle container with access by name and by index.
class PvtDataHandleList
{
public:
    size_t size() const
    {
        return _handles.size();
    }

    bool empty() const
    {
        return _handles.empty();
    }

    size_t index(const RtToken& name) const
    {
        auto it = _indexByName.find(name);
        return it != _indexByName.end() ? it->second : size_t(-1);
    }

    PvtDataHandle get(const RtToken& name) const
    {
        return get(index(name));
    }

    PvtDataHandle get(size_t i) const
    {
        return i < _handles.size() ? _handles[i] : PvtDataHandle();
    }

    void add(const PvtDataHandle& hnd)
    {
        _handles.push_back(hnd);
        _indexByName[hnd->getName()] = _handles.size() - 1;
    }

    PvtDataHandle remove(const RtToken& name)
    {
        auto it = _indexByName.find(name);
        if (it != _indexByName.end())
        {
            PvtDataHandle hnd = _handles[it->second];
            _handles.erase(_handles.begin() + it->second);
            _indexByName.erase(it);
            return hnd;
        }
        return PvtDataHandle();
    }

    void clear()
    {
        _handles.clear();
        _indexByName.clear();
    }

    const PvtDataHandleVec& all() const
    {
        return _handles;
    }

    PvtDataHandleVec& all()
    {
        return _handles;
    }

private:
    PvtDataHandleVec _handles;
    RtTokenMap<size_t> _indexByName;
};

}

#endif
