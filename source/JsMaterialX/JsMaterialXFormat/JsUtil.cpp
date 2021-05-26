#include "../Helpers.h"
#include "./emscriptenTypeRegistration.h"
#include <MaterialXFormat/Util.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(xformatUtil)
{
  ems::constant("PATH_LIST_SEPARATOR", mx::PATH_LIST_SEPARATOR);
  ems::constant("MATERIALX_SEARCH_PATH_ENV_VAR", mx::MATERIALX_SEARCH_PATH_ENV_VAR);
  ems::constant("MATERIALX_ASSET_DEFINITION_PATH_ENV_VAR", mx::MATERIALX_ASSET_DEFINITION_PATH_ENV_VAR);
  ems::constant("MATERIALX_ASSET_TEXTURE_PATH_ENV_VAR", mx::MATERIALX_ASSET_TEXTURE_PATH_ENV_VAR);
  BIND_FUNC_RAW_PTR("loadLibrary", mx::loadLibrary, 2, 4, const mx::FilePath&, mx::DocumentPtr, const mx::FileSearchPath&, const mx::XmlReadOptions*);
}

