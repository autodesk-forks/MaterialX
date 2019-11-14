//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTVALUE_H
#define MATERIALX_RTVALUE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtToken.h>

#include <MaterialXCore/Value.h>


#ifndef _WIN32
// Disable warnings for breaking strict aliasing.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

namespace MaterialX
{

/// @class RtValueStore
/// Class for allocating and keeping ownership of values
/// that are too large in size to fit into a RtValue.
template<typename T>
class RtValueStore
{
public:
    // Destructor.
    ~RtValueStore()
    {
        for (T* ptr : _storage)
        {
            delete[] ptr;
        }
        _storage.clear();
    }

    // Allocate and return a new value.
    T* alloc(size_t count = 1)
    {
        T* ptr = new T[count];
        _storage.push_back(ptr);
        return ptr;
    }

private:
    std::vector<T*> _storage;
};

/// @struct RtLargeValueStorage
/// Struct with storage for data types that are larger than 16 bytes.
struct RtLargeValueStorage
{
    RtValueStore<string> str;
    RtValueStore<Matrix33> mtx33;
    RtValueStore<Matrix44> mtx44;
};

/// @class RtValue
/// Generic value class for storing values of all the data types
/// supported by the API. Values that fit into 16 bytes of data
/// are stored directly. Values larger than 16 bytes are stored
/// as pointers. The large values are allocated and managed by an
/// instance of the RtValueStore class.
class RtValue
{
public:
    /// Default constructor
    RtValue() : _data{0,0} {}

    /// Explicit value constructor
    explicit RtValue(bool v) { asBool() = v; }
    explicit RtValue(int v) { asInt() = v; }
    explicit RtValue(float v) { asFloat() = v; }
    explicit RtValue(const Color2& v) { asColor2() = v; }
    explicit RtValue(const Color3& v) { asColor3() = v; }
    explicit RtValue(const Color4& v) { asColor4() = v; }
    explicit RtValue(const Vector2& v) { asVector2() = v; }
    explicit RtValue(const Vector3& v) { asVector3() = v; }
    explicit RtValue(const Vector4& v) { asVector4() = v; }
    explicit RtValue(const RtToken& v) { asToken() = v; }
    explicit RtValue(void* v) { asPtr() = v; }

    /// Explicit value constructor for large values
    explicit RtValue(const Matrix33& v, RtValueStore<Matrix33>& store);
    explicit RtValue(const Matrix44& v, RtValueStore<Matrix44>& store);
    explicit RtValue(const string& v, RtValueStore<string>& store);

    /// Return bool value.
    const bool& asBool() const
    {
        return *reinterpret_cast<const bool*>(&_data);
    }
    /// Return reference to bool value.
    bool& asBool()
    {
        return *reinterpret_cast<bool*>(&_data);
    }

    /// Return int value.
    int asInt() const
    {
        return *reinterpret_cast<const int*>(&_data);
    }
    /// Return reference to int value.
    int& asInt()
    {
        return *reinterpret_cast<int*>(&_data);
    }

    /// Return float value.
    float asFloat() const
    {
        return *reinterpret_cast<const float*>(&_data);
    }
    /// Return reference to float value.
    float& asFloat()
    {
        return *reinterpret_cast<float*>(&_data);
    }

    /// Return Color2 value.
    const Color2& asColor2() const
    {
        return *reinterpret_cast<const Color2*>(&_data);
    }
    /// Return reference to Color2 value.
    Color2& asColor2()
    {
        return *reinterpret_cast<Color2*>(&_data);
    }

    /// Return Color3 value.
    const Color3& asColor3() const
    {
        return *reinterpret_cast<const Color3*>(&_data);
    }
    /// Return reference to Color3 value.
    Color3& asColor3()
    {
        return *reinterpret_cast<Color3*>(&_data);
    }

    /// Return Color4 value.
    const Color4& asColor4() const
    {
        return *reinterpret_cast<const Color4*>(&_data);
    }
    /// Return reference to Color4 value.
    Color4& asColor4()
    {
        return *reinterpret_cast<Color4*>(&_data);
    }

    /// Return Vector2 value.
    const Vector2& asVector2() const
    {
        return *reinterpret_cast<const Vector2*>(&_data);
    }
    /// Return reference to Vector2 value.
    Vector2& asVector2()
    {
        return *reinterpret_cast<Vector2*>(&_data);
    }

    /// Return Vector3 value.
    const Vector3& asVector3() const
    {
        return *reinterpret_cast<const Vector3*>(&_data);
    }
    /// Return reference to Vector3 value.
    Vector3& asVector3()
    {
        return *reinterpret_cast<Vector3*>(&_data);
    }

    /// Return Vector4 value.
    const Vector4& asVector4() const
    {
        return *reinterpret_cast<const Vector4*>(&_data);
    }
    /// Return reference to Vector4 value.
    Vector4& asVector4()
    {
        return *reinterpret_cast<Vector4*>(&_data);
    }

    /// Return token value.
    const RtToken& asToken() const
    {
        return *reinterpret_cast<const RtToken*>(&_data);
    }
    /// Return reference to token value.
    RtToken& asToken()
    {
        return *reinterpret_cast<RtToken*>(&_data);
    }

    /// Return const pointer.
    void* const& asPtr() const
    {
        return *reinterpret_cast<void* const*>(&_data);
    }
    /// Return reference to pointer.
    void*& asPtr()
    {
        return *reinterpret_cast<void**>(&_data);
    }

    /// Return Matrix33 value.
    const Matrix33& asMatrix33() const
    {
        return **reinterpret_cast<const Matrix33* const*>(&_data);
    }
    /// Return reference to Matrix33 value.
    Matrix33& asMatrix33()
    {
        return **reinterpret_cast<Matrix33**>(&_data);
    }

    /// Return Matrix44 value.
    const Matrix44& asMatrix44() const
    {
        return **reinterpret_cast<Matrix44* const*>(&_data);
    }
    /// Return reference to Matrix44 value.
    Matrix44& asMatrix44()
    {
        return **reinterpret_cast<Matrix44**>(&_data);
    }

    /// Return string value.
    const string& asString() const
    {
        return **reinterpret_cast<string* const*>(&_data);
    }
    /// Return reference to string value.
    string& asString()
    {
        return **reinterpret_cast<string**>(&_data);
    }

    /// Clear the value with zeores.
    void clear()
    {
        _data[0] = _data[1] = 0;
    }

    bool operator==(const RtValue& other)
    {
        return _data[0] == other._data[0] && _data[1] == other._data[1];
    }

    bool operator!=(const RtValue& other)
    {
        return !(*this==other);
    }

    /// Return a string representing the value
    /// in the given type.
    string getValueString(const RtToken& type) const;

    /// Set the value from a string representation of a
    /// value in the given type.
    void setValueString(const RtToken& type, const string& value, RtLargeValueStorage& store);

    /// Create a new value of given type.
    /// Use the given storage if the type is a large value.
    static RtValue createNew(const RtToken& type, RtLargeValueStorage& store);

private:
    // 16 bytes of data storage to hold the main data types,
    // up to four component vector/color. Larger data types
    // needs to be allocated through the RtValueStore class.
    // Storage is aligned to 64-bit to hold pointers for
    // heap allocated data types as well as other pointers.
    uint64_t _data[2];
};

}

#ifndef _WIN32
#pragma GCC diagnostic pop
#endif

#endif
