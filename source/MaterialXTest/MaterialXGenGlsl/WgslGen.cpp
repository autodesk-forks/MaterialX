//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXGenGlsl/wgsl/WgslShaderGenerator.h>

#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXFormat/Environ.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlIo.h>

#include <fstream>

namespace mx = MaterialX;

namespace
{
// Count non-overlapping occurrences of a substring.
int countOccurrences(const std::string& haystack, const std::string& needle)
{
    int n = 0;
    for (size_t p = haystack.find(needle); p != std::string::npos; p = haystack.find(needle, p + needle.size()))
    {
        n++;
    }
    return n;
}

// Verify that '{' and '}' are balanced and never close below zero.
bool bracesBalanced(const std::string& source)
{
    int depth = 0;
    for (char c : source)
    {
        if (c == '{')
        {
            depth++;
        }
        else if (c == '}')
        {
            if (--depth < 0)
            {
                return false;
            }
        }
    }
    return depth == 0;
}

// Assert the structural invariants every generated pixel shader must satisfy: it is valid-looking
// WGSL (no GLSL leftovers), exposes exactly one entry function, and has balanced braces. These are
// cheap, deterministic checks; full WGSL-syntax validation (naga/tint) runs as a separate CI step.
void checkWgslInvariants(const std::string& pixel, const std::string& label)
{
    INFO("material: " << label);
    REQUIRE(pixel.length() > 0);

    // No GLSL leftovers from the GLSL->WGSL rewrite.
    CHECK(pixel.find("#version") == std::string::npos);
    CHECK(pixel.find("#define") == std::string::npos);
    CHECK(pixel.find("layout(") == std::string::npos);
    CHECK(pixel.find("texture2D ") == std::string::npos); // GLSL combined-sampler type
    CHECK(pixel.find("sampler2D ") == std::string::npos);
    CHECK(pixel.find("gl_") == std::string::npos);

    // Boolean HW uniforms are stored as i32 (WGSL uniform buffers cannot hold bool), so any
    // GLSL boolean condition on one must be lowered to an explicit `!= 0` comparison; a bare
    // `if (u_refractionTwoSided)` would be an invalid `if (i32)` in WGSL.
    CHECK(pixel.find("if (u_refractionTwoSided)") == std::string::npos);

    // MaterialX booleans map to i32 everywhere (uniforms, node-function parameters, args) so
    // the type agrees on both sides of a call. A node-graph parameter spelled `thin_walled: bool`
    // would mismatch the i32 uniform argument passed to it.
    CHECK(pixel.find("thin_walled: bool") == std::string::npos);

    // Library BSDF helpers keep native `bool` parameters; MaterialX boolean inputs passed to
    // them must be wrapped in `bool(...)`. A bare integer literal is rejected by WGSL.
    CHECK(pixel.find("roughness_vector_out, 0, 0.000000") == std::string::npos);

    // Light-bridge helpers must be WGSL `fn`s, not GLSL `int`/`void` definitions.
    CHECK(pixel.find("int numActiveLightSources()") == std::string::npos);
    CHECK(pixel.find("void sampleLightSource") == std::string::npos);

    // Volume shaders (Open PBR, glTF PBR, …) emit VDF helpers; the type must be declared in
    // emitWgslSurfaceTypes() — GLSL struct typedefs are not copied into WGSL output.
    if (pixel.find("mx_anisotropic_vdf") != std::string::npos)
        CHECK(pixel.find("struct VDF") != std::string::npos);

    // mx_hsv.glsl uses chained `float` decls, braceless `else`, `0.0f` literals, and GLSL `int()` casts.
    if (pixel.find("mx_rgbtohsv") != std::string::npos || pixel.find("mx_hsvtorgb") != std::string::npos)
    {
        CHECK(pixel.find("float s =") == std::string::npos);
        CHECK(pixel.find("float g =") == std::string::npos);
        CHECK(pixel.find("else s =") == std::string::npos);
        CHECK(pixel.find("0.0f") == std::string::npos);
        CHECK(pixel.find("int(trunc") == std::string::npos);
    }

    // `else // comment` must not be lowered to an empty `else { }` block (breaks brace balance).
    CHECK(pixel.find("else { // FRESNEL_MODEL_SCHLICK }") == std::string::npos);

    // Native WGSL struct/uniform members must keep `vec3f`/`mat4x4f` spellings (not bare GLSL `vec3`).
    CHECK(pixel.find(": vec3,") == std::string::npos);
    CHECK(pixel.find(": vec3;") == std::string::npos);
    CHECK(pixel.find(": vec4,") == std::string::npos);
    CHECK(pixel.find(": mat4x4,") == std::string::npos);

    // Exactly one WGSL entry function, and balanced scopes.
    CHECK(countOccurrences(pixel, "fn material_main(") == 1);
    CHECK(bracesBalanced(pixel));
}

// Verify the manifest is non-empty and advertises the entry function and a resource block.
void checkManifest(const std::string& manifest, const std::string& label)
{
    INFO("material: " << label);
    REQUIRE(manifest.length() > 0);
    CHECK(manifest.front() == '{');
    CHECK(manifest.find("\"entry\"") != std::string::npos);
    CHECK(manifest.find("material_main") != std::string::npos);
    CHECK(manifest.find("\"bindings\"") != std::string::npos);
}

// Optionally write the generated artifacts for manual inspection, controlled by an environment
// variable so the test never depends on a hardcoded path. Set MATERIALX_WGSL_DUMP_DIR to enable.
void maybeDump(const mx::ShaderPtr& shader, const std::string& manifest, const std::string& name)
{
    const std::string dumpDir = mx::getEnviron("MATERIALX_WGSL_DUMP_DIR");
    if (dumpDir.empty())
    {
        return;
    }
    const std::string base = dumpDir + "/" + name;
    std::ofstream(base + "_pixel.wgsl") << shader->getSourceCode(mx::Stage::PIXEL);
    std::ofstream(base + "_manifest.json") << manifest;
}

// A minimal standard_surface, kept inline so the smoke test is independent of example assets.
const std::string STANDARD_SURFACE_MTLX = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <standard_surface name="ss" type="surfaceshader">
    <input name="base" type="float" value="1.0"/>
    <input name="base_color" type="color3" value="0.8, 0.5, 0.3"/>
    <input name="specular" type="float" value="0.5"/>
    <input name="specular_roughness" type="float" value="0.3"/>
    <input name="metalness" type="float" value="0.0"/>
  </standard_surface>
  <surfacematerial name="mat" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="ss"/>
  </surfacematerial>
</materialx>
)";

// Load the standard MaterialX data libraries used by all cases below.
mx::DocumentPtr loadStandardLibraries(const mx::FileSearchPath& searchPath)
{
    mx::DocumentPtr stdlib = mx::createDocument();
    mx::loadLibraries({ "libraries/targets", "libraries/stdlib", "libraries/pbrlib", "libraries/bxdf" },
                      searchPath, stdlib);
    return stdlib;
}
} // namespace

TEST_CASE("WgslGen: syntax emits native WGSL type names", "[wgslgen]")
{
    const mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();
    const mx::Syntax& syntax = generator->getSyntax();
    CHECK(syntax.getTypeName(mx::Type::FLOAT) == "f32");
    CHECK(syntax.getTypeName(mx::Type::VECTOR3) == "vec3f");
    CHECK(syntax.getTypeName(mx::Type::COLOR3) == "vec3f");
    CHECK(syntax.getTypeName(mx::Type::MATRIX44) == "mat4x4f");
}

TEST_CASE("WgslGen: standard_surface emits valid WGSL", "[wgslgen]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr stdlib = loadStandardLibraries(searchPath);

    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlString(doc, STANDARD_SURFACE_MTLX);
    doc->setDataLibrary(stdlib);

    mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();
    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(searchPath);

    std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
    REQUIRE(!elements.empty());

    mx::ShaderPtr shader = generator->generate(elements[0]->getNamePath(), elements[0], context);
    REQUIRE(shader != nullptr);

    const std::string pixel = shader->getSourceCode(mx::Stage::PIXEL);
    checkWgslInvariants(pixel, "standard_surface");

    auto wgslGen = std::dynamic_pointer_cast<mx::WgslShaderGenerator>(generator);
    REQUIRE(wgslGen != nullptr);
    CHECK(wgslGen->getGeneratedManifest().find("material_main") != std::string::npos);
    checkManifest(wgslGen->getGeneratedManifest(), "standard_surface");

    maybeDump(shader, wgslGen->getGeneratedManifest(), "standard_surface");
}

TEST_CASE("WgslGen: hwSrgbEncodeOutput encodes surface output", "[wgslgen]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr stdlib = loadStandardLibraries(searchPath);

    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlString(doc, STANDARD_SURFACE_MTLX);
    doc->setDataLibrary(stdlib);

    mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();
    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(searchPath);
    context.getOptions().hwSrgbEncodeOutput = true;

    std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
    REQUIRE(!elements.empty());

    mx::ShaderPtr shader = generator->generate(elements[0]->getNamePath(), elements[0], context);
    REQUIRE(shader != nullptr);

    const std::string pixel = shader->getSourceCode(mx::Stage::PIXEL);
    checkWgslInvariants(pixel, "standard_surface_srgb");
    CHECK(pixel.find("mx_srgb_encode(") != std::string::npos);
    CHECK(pixel.find("return vec4f(mx_srgb_encode(") != std::string::npos);
}

TEST_CASE("WgslGen: directional lights emit LightData binding", "[wgslgen]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr stdlib = loadStandardLibraries(searchPath);
    mx::loadLibraries({ "libraries/lights" }, searchPath, stdlib);
    mx::NodeDefPtr directionalLight = stdlib->getNodeDef("ND_directional_light");
    REQUIRE(directionalLight != nullptr);

    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlString(doc, STANDARD_SURFACE_MTLX);
    doc->setDataLibrary(stdlib);

    mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();
    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(searchPath);
    context.getOptions().hwMaxActiveLightSources = 4;
    mx::HwShaderGenerator::bindLightShader(*directionalLight, 1, context);

    std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
    REQUIRE(!elements.empty());

    mx::ShaderPtr shader = generator->generate(elements[0]->getNamePath(), elements[0], context);
    REQUIRE(shader != nullptr);

    const std::string pixel = shader->getSourceCode(mx::Stage::PIXEL);
    checkWgslInvariants(pixel, "standard_surface_lights");
    CHECK(pixel.find("struct LightData") != std::string::npos);
    CHECK(pixel.find("fn numActiveLightSources") != std::string::npos);
    CHECK(pixel.find("fn sampleLightSource") != std::string::npos);
    CHECK(pixel.find("mx_directional_light(light, position, (*result))") == std::string::npos);
    CHECK(pixel.find("u_lightData") != std::string::npos);

    auto wgslGen = std::dynamic_pointer_cast<mx::WgslShaderGenerator>(generator);
    REQUIRE(wgslGen != nullptr);
    const std::string manifest = wgslGen->getGeneratedManifest();
    CHECK(manifest.find("\"role\": \"lightData\"") != std::string::npos);
    CHECK(manifest.find("u_lightData") != std::string::npos);
    CHECK(countOccurrences(pixel, "var<uniform> u_lightData") == 1);

    maybeDump(shader, manifest, "standard_surface_lights");
}

TEST_CASE("WgslGen: example materials emit valid WGSL", "[wgslgen]")
{
    // A representative set across shading models. Textured (brass_tiled) exercises the image-node
    // and sampler-splitting paths; the metals/dielectrics exercise the closure graph broadly.
    const mx::StringVec materials = {
        "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx",
        "resources/Materials/Examples/StandardSurface/standard_surface_gold.mtlx",
        "resources/Materials/Examples/StandardSurface/standard_surface_copper.mtlx",
        "resources/Materials/Examples/StandardSurface/standard_surface_chrome.mtlx",
        "resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx",
        "resources/Materials/Examples/StandardSurface/standard_surface_brick_procedural.mtlx",
        "resources/Materials/Examples/UsdPreviewSurface/usd_preview_surface_default.mtlx",
        "resources/Materials/Examples/GltfPbr/gltf_pbr_default.mtlx",
        "resources/Materials/Examples/OpenPbr/open_pbr_default.mtlx",
    };

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr stdlib = loadStandardLibraries(searchPath);

    for (const std::string& relPath : materials)
    {
        const mx::FilePath resolved = searchPath.find(relPath);
        INFO("material file: " << relPath);
        REQUIRE(resolved.exists());

        mx::DocumentPtr doc = mx::createDocument();
        mx::readFromXmlFile(doc, resolved);
        doc->setDataLibrary(stdlib);

        // Each material gets a fresh generator so per-generation state cannot leak between cases.
        mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();
        mx::GenContext context(generator);
        context.registerSourceCodeSearchPath(searchPath);
        context.registerSourceCodeSearchPath(resolved.getParentPath());

        std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
        REQUIRE(!elements.empty());

        mx::ShaderPtr shader = generator->generate(elements[0]->getNamePath(), elements[0], context);
        REQUIRE(shader != nullptr);

        const std::string name = resolved.getBaseName();
        checkWgslInvariants(shader->getSourceCode(mx::Stage::PIXEL), name);

        auto wgslGen = std::dynamic_pointer_cast<mx::WgslShaderGenerator>(generator);
        REQUIRE(wgslGen != nullptr);
        checkManifest(wgslGen->getGeneratedManifest(), name);

        maybeDump(shader, wgslGen->getGeneratedManifest(), name);
    }
}
