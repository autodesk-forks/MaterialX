//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/wgsl/WgslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

namespace
{
// Creator wrapper to avoid having to expose the TypeSystem class in JavaScript.
// Returns the base ShaderGeneratorPtr (whose smart pointer is already registered).
mx::ShaderGeneratorPtr WgslShaderGenerator_create()
{
    return mx::WgslShaderGenerator::create();
}

// Expose the manifest (JSON describing the entry function, uniforms, textures and
// their semantics) that the generator records during generate(). The Three.js bridge
// consumes this to reshape the WGSL into a wgslFn entry and wire its resources.
// A free function (taking the base generator) avoids registering a second smart
// pointer type for the derived class just to call this one method.
std::string WgslShaderGenerator_getGeneratedManifest(const mx::ShaderGeneratorPtr& generator)
{
    auto wgsl = std::dynamic_pointer_cast<mx::WgslShaderGenerator>(generator);
    return wgsl ? wgsl->getGeneratedManifest() : std::string();
}
} // namespace

EMSCRIPTEN_BINDINGS(WgslShaderGenerator)
{
    ems::class_<mx::WgslShaderGenerator, ems::base<mx::GlslShaderGenerator>>("WgslShaderGenerator")
        .class_function("create", &WgslShaderGenerator_create);

    ems::function("getWgslGeneratedManifest", &WgslShaderGenerator_getGeneratedManifest);
}
