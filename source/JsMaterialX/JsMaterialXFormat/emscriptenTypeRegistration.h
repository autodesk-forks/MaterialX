#include <MaterialXFormat/File.h>
#include <emscripten.h>
#include <emscripten/bind.h>


namespace ems = emscripten;
namespace mx = MaterialX;

template<typename T> struct IsFileSearchPath : std::false_type {};
template<> struct IsFileSearchPath<mx::FileSearchPath> : std::true_type {};
template<> struct IsFileSearchPath<mx::FilePath> : std::true_type {};

using FileSearchPathIntermediate = std::string;


namespace emscripten {
namespace internal {
template<typename T>
struct TypeID<T, typename std::enable_if<IsFileSearchPath<typename std::remove_cv<typename std::remove_reference<T>::type>::type>::value, void>::type> {
  static constexpr TYPEID get() {
    return TypeID<FileSearchPathIntermediate>::get();
  }
};

template<typename T>
struct BindingType<T, typename std::enable_if<IsFileSearchPath<T>::value, void>::type> {
  typedef typename BindingType<FileSearchPathIntermediate>::WireType WireType;

  constexpr static WireType toWireType(const T& v) {
    return BindingType<FileSearchPathIntermediate>::toWireType(v.asString());
  }
  constexpr static T fromWireType(WireType v) {
    return T(BindingType<FileSearchPathIntermediate>::fromWireType(v));
  }
};
} // namespace internal
} // namespace emscripten