//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTPRIM_H
#define MATERIALX_PVTPRIM_H

#include <MaterialXRuntime/Private/PvtPort.h>
#include <MaterialXRuntime/Private/PvtRelationship.h>

#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

// Allocator class handling allocation of data for prims.
// The data allocated is kept by the allocator and freed
// upon allocator destruction or by calling free() explicitly.
// NOTE: Data is stored as raw byte pointers and destructors
// for allocated objects will not be called when freeing data.
class PvtAllocator
{
public:
    ~PvtAllocator()
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

    // Allocate and return a single object of templated type.
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
    vector<uint8_t*> _storage;
};

class RtPrimIterator;

class PvtPrim : public PvtObject
{
    RT_DECLARE_RUNTIME_OBJECT(PvtPrim)

public:
    template<class T = PvtPrim>
    static PvtDataHandle createNew(const RtTypeInfo* type, const RtToken& name, PvtPrim* parent)
    {
        // Make the name unique.
        const RtToken primName = parent->makeUniqueChildName(name);
        return PvtDataHandle(new T(type, primName, parent));
    }

    RtPrim prim() const
    {
        return RtPrim(hnd());
    }

    void dispose(bool state);

    void destroy();

    const RtTypeInfo* getTypeInfo() const
    {
        return _typeInfo;
    }

    template<class T>
    bool hasApi() const
    {
        static_assert(std::is_base_of<RtSchemaBase, T>::value,
            "Templated type must be a concrete subclass of RtSchemaBase");
        return _typeInfo->isCompatible(T::typeName());
    }

    PvtRelationship* createRelationship(const RtToken& name);

    void removeRelationship(const RtToken& name);

    void renameRelationship(const RtToken& name, const RtToken& newName);

    PvtRelationship* getRelationship(const RtToken& name)
    {
        PvtDataHandle hnd = _rel.get(name);
        return hnd ? hnd->asA<PvtRelationship>() : nullptr;
    }

    const PvtDataHandleVec& getAllRelationships() const
    {
        return _rel.all();
    }

    PvtInput* createInput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    void removeInput(const RtToken& name);

    RtToken renameInput(const RtToken& name, const RtToken& newName, bool makeUnique = true);

    size_t numInputs() const
    {
        return _inputs.size();
    }

    PvtInput* getInput(size_t index) const
    {
        PvtDataHandle hnd = _inputs.get(index);
        return hnd ? hnd->asA<PvtInput>() : nullptr;
    }

    PvtInput* getInput(const RtToken& name) const
    {
        PvtDataHandle hnd = _inputs.get(name);
        return hnd ? hnd->asA<PvtInput>() : nullptr;
    }

    const PvtDataHandleVec& getInputs() const
    {
        return _inputs.all();
    }

    PvtOutput* createOutput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    void removeOutput(const RtToken& name);

    RtToken renameOutput(const RtToken& name, const RtToken& newName, bool makeUnique = true);

    size_t numOutputs() const
    {
        return _outputs.size();
    }

    PvtOutput* getOutput(size_t index = 0) const
    {
        PvtDataHandle hnd = _outputs.get(index);
        return hnd ? hnd->asA<PvtOutput>() : nullptr;
    }

    PvtOutput* getOutput(const RtToken& name) const
    {
        PvtDataHandle hnd = _outputs.get(name);
        return hnd ? hnd->asA<PvtOutput>() : nullptr;
    }

    const PvtDataHandleVec& getOutputs() const
    {
        return _outputs.all();
    }

    size_t numChildren() const
    {
        return _prims.size();
    }

    PvtPrim* getChild(size_t index) const
    {
        PvtDataHandle hnd = _prims.get(index);
        return hnd ? hnd->asA<PvtPrim>() : nullptr;
    }

    PvtPrim* getChild(const RtToken& name) const
    {
        PvtDataHandle hnd = _prims.get(name);
        return hnd ? hnd->asA<PvtPrim>() : nullptr;
    }

    RtPrimIterator getChildren(RtObjectPredicate predicate = nullptr) const;

    const PvtDataHandleVec& getAllChildren() const
    {
        return _prims.all();
    }

    PvtAllocator& getAllocator()
    {
        return _allocator;
    }

    RtToken makeUniqueChildName(const RtToken& name) const;

    // Validate that typenames match when creating a new prim.
    static void validateCreation(const RtTypeInfo& typeInfo, const RtToken& typeName, const RtToken& name)
    {
        if (typeName != typeInfo.getShortTypeName())
        {
            throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
        }
    }

    // Validate that typenames match and that parent path is at the root.
    static void validateCreation(const RtTypeInfo& typeInfo, const RtToken& typeName, const RtToken& name, const RtPath& parentPath)
    {
        validateCreation(typeInfo, typeName, name);

        if (!parentPath.isRoot())
        {
            throw ExceptionRuntimeError("A '" + typeName.str() + "' prim can only be created at the top / root level");
        }
    }

protected:
    PvtPrim(const RtTypeInfo* typeInfo, const RtToken& name, PvtPrim* parent);

    void addChildPrim(const PvtPrim* prim);
    void removeChildPrim(const PvtPrim* prim);

    const RtTypeInfo* _typeInfo;

    // Relationships
    PvtDataHandleList _rel;

    // Inputs
    PvtDataHandleList _inputs;

    // Outputs
    PvtDataHandleList _outputs;

    // Child prims
    PvtDataHandleList _prims;

    PvtAllocator _allocator;

    friend class PvtApi;
    friend class PvtStage;
    friend class RtNodeGraph;
    friend class RtInputIterator;
    friend class RtOutputIterator;
    friend class RtRelationshipIterator;
};

}

#endif
