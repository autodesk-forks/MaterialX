#include "../vectorHelper.h"

#include <MaterialXCore/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(util)
    {
        ems::constant("EMPTY_STRING", mx::EMPTY_STRING);

        ems::function("getVersionString", &mx::getVersionString);

        ems::function("getVersionIntegers", ems::optional_override([]() {
                     std::tuple<int, int, int> version = mx::getVersionIntegers();
                     return ems::val::array((int *)&version, (int *)&version + 3);
                 }));

        // Emscripten expects to provide a number from JS for a cpp 'char' parameter. 
        // Using a string seems to be the better interface for JS
        ems::function("createValidName", ems::optional_override([](std::string name) {
            return mx::createValidName(name);
        }));
        ems::function("createValidName", ems::optional_override([](std::string name, std::string replaceChar) {
            return mx::createValidName(name, replaceChar.front());
        }));

        ems::function("isValidName", &mx::isValidName);
        ems::function("isValidNamespace", &mx::isValidNamespace);
        ems::function("incrementName", &mx::incrementName);

        ems::function("splitNamePath", &mx::splitNamePath);
        ems::function("createNamePath", &mx::createNamePath);
        ems::function("parentNamePath", &mx::parentNamePath);
    }
}