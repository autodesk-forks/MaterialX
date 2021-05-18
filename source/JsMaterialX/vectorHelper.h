/**
 * Include this in every file that defines Emscripten bindings for functions with
 * std::vector parameters or return types, to automatically convert them to / from JS arrays.
 * It actually doesn't hurt to include this in every binding file ;)
 * Note that this only works for types that are known to Emscripten, i.e. primitive (built-in) types
 * and types that have bindings defined.
 */

#ifndef JSMATERIALX_VECTOR_HELPERS_H
#define JSMATERIALX_VECTOR_HELPERS_H

#ifdef __EMSCRIPTEN__

#include <emscripten/bind.h>

#include <memory>
#include <vector>

namespace emscripten {
namespace internal {

template<typename T>
struct TypeID<std::vector<T>> {
    static constexpr TYPEID get() {
        return LightTypeID<val>::get();
    }
};

template<typename T>
struct TypeID<const std::vector<T>> {
    static constexpr TYPEID get() {
        return LightTypeID<val>::get();
    }
};

template<typename T>
struct TypeID<std::vector<T>&> {
    static constexpr TYPEID get() {
        return LightTypeID<val>::get();
    }
};

template<typename T>
struct TypeID<const std::vector<T>&> {
    static constexpr TYPEID get() {
        return LightTypeID<val>::get();
    }
};

template<typename T>
struct BindingType<std::vector<T>> {
    using ValBinding = BindingType<val>;
    using WireType = ValBinding::WireType;

    static WireType toWireType(const std::vector<T> &vec) {
        return ValBinding::toWireType(val::array(vec));
    }

    static std::vector<T> fromWireType(WireType value) {
        return vecFromJSArray<T>(ValBinding::fromWireType(value));
    }
};


}  // namespace internal
}  // namespace emscripten

#endif // __EMSCRIPTEN__
#endif // JSMATERIALX_VECTOR_HELPERS_H