//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXGenGlsl/wgsl/GlslToWgsl.h>

namespace mx = MaterialX;

namespace
{
bool contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}
int countOccurrences(const std::string& haystack, const std::string& needle)
{
    int n = 0;
    for (size_t p = haystack.find(needle); p != std::string::npos; p = haystack.find(needle, p + needle.size()))
        n++;
    return n;
}
} // namespace

TEST_CASE("GlslToWgsl: type mapping", "[genglsl][wgsl]")
{
    CHECK(mx::WgslRewrite::mapType("float") == "f32");
    CHECK(mx::WgslRewrite::mapType("int") == "i32");
    CHECK(mx::WgslRewrite::mapType("uint") == "u32");
    CHECK(mx::WgslRewrite::mapType("vec2") == "vec2f");
    CHECK(mx::WgslRewrite::mapType("vec3") == "vec3f");
    CHECK(mx::WgslRewrite::mapType("vec4") == "vec4f");
    CHECK(mx::WgslRewrite::mapType("mat3") == "mat3x3f");
    CHECK(mx::WgslRewrite::mapType("mat4") == "mat4x4f");
    // Unknown / already-WGSL types pass through unchanged.
    CHECK(mx::WgslRewrite::mapType("f32") == "f32");
    CHECK(mx::WgslRewrite::mapType("ClosureData") == "ClosureData");
}

TEST_CASE("GlslToWgsl: variable and const declarations", "[genglsl][wgsl]")
{
    CHECK(mx::WgslRewrite::rewriteAll("vec3 v = vec3(1.0);") == "var v: vec3f = vec3(1.0);");
    CHECK(mx::WgslRewrite::rewriteAll("float x = 2.0;") == "var x: f32 = 2.0;");
    CHECK(mx::WgslRewrite::rewriteAll("const float PI = 3.14;") == "const PI: f32 = 3.14;");
}

TEST_CASE("GlslToWgsl: ternary, inc/dec, float cast", "[genglsl][wgsl]")
{
    // MaterialX emits parenthesized ternary conditions.
    CHECK(mx::WgslRewrite::rewriteAll("int i = (a > b) ? c : d;") == "var i: i32 = select(d, c, (a > b));");
    // Unparenthesized multi-token comparison condition must be captured whole.
    CHECK(mx::WgslRewrite::rewriteAll("y = cosTheta < cosB ? 0.0 : M_PI;") == "y = select(M_PI, 0.0, cosTheta < cosB);");
    CHECK(mx::WgslRewrite::rewriteAll("i++;") == "i += 1;");
    CHECK(mx::WgslRewrite::rewriteAll("++i;") == "i += 1;");
    CHECK(mx::WgslRewrite::rewriteAll("y = float(z);") == "y = f32(z);");
    CHECK(mx::WgslRewrite::rewriteAll("var hi: i32 = int(trunc(h));") == "var hi: i32 = i32(trunc(h));");
}

TEST_CASE("GlslToWgsl: combined sampler split and texture sampling", "[genglsl][wgsl]")
{
    CHECK(mx::WgslRewrite::rewriteSamplerParams("sampler2D tex") ==
          "tex_texture: texture_2d<f32>, tex_sampler: sampler");
    CHECK(mx::WgslRewrite::rewriteTextureSampling("color = texture(tex, uv);") ==
          "color = mx_texture_sample(tex_texture, tex_sampler, uv);");
    // A function name ending in `texture` must not be rewritten.
    CHECK(mx::WgslRewrite::rewriteTextureSampling("mytexture(a, b);") == "mytexture(a, b);");
}

TEST_CASE("GlslToWgsl: value-returning function signature", "[genglsl][wgsl]")
{
    const std::string block =
        "vec3 mx_bar(float x)\n"
        "{\n"
        "    return vec3(x);\n"
        "}\n";
    const std::string wgsl = mx::WgslRewrite::rewriteBlock(block);
    // Value parameter `x` is renamed and copied into a mutable local.
    CHECK(contains(wgsl, "fn mx_bar(x_arg: f32) -> vec3f"));
    CHECK(contains(wgsl, "var x: f32 = x_arg;"));
    CHECK(contains(wgsl, "return vec3(x);"));
}

TEST_CASE("GlslToWgsl: out parameter becomes pointer and is dereferenced", "[genglsl][wgsl]")
{
    const std::string block =
        "void mx_foo(float a, out vec3 r)\n"
        "{\n"
        "    r = vec3(a);\n"
        "}\n";
    const std::string wgsl = mx::WgslRewrite::rewriteBlock(block);
    CHECK(contains(wgsl, "fn mx_foo(a_arg: f32, r: ptr<function, vec3f>)"));
    // No return type when results flow through out parameters.
    CHECK_FALSE(contains(wgsl, "-> "));
    // Value parameter copied to a mutable local; out parameter is dereferenced.
    CHECK(contains(wgsl, "var a: f32 = a_arg;"));
    CHECK(contains(wgsl, "(*r) = vec3(a);"));
}

TEST_CASE("GlslToWgsl: multi-line signature with out parameter", "[genglsl][wgsl]")
{
    const std::string block =
        "void mx_split(\n"
        "    float a,\n"
        "    out float b)\n"
        "{\n"
        "    b = a;\n"
        "}\n";
    const std::string wgsl = mx::WgslRewrite::rewriteBlock(block);
    CHECK(contains(wgsl, "fn mx_split(a_arg: f32, b: ptr<function, f32>)"));
    CHECK(contains(wgsl, "var a: f32 = a_arg;"));
    CHECK(contains(wgsl, "(*b) = a;"));
}

TEST_CASE("GlslToWgsl: out parameter passed to callee stays a pointer", "[genglsl][wgsl]")
{
    const std::string block =
        "void mx_bar(float a, out vec3 r)\n"
        "{\n"
        "    mx_foo(a, r);\n"
        "}\n"
        "void mx_foo(float a, out vec3 r)\n"
        "{\n"
        "    r = vec3(a);\n"
        "}\n";
    const std::string wgsl = mx::WgslRewrite::rewriteBlock(block);
    CHECK(contains(wgsl, "mx_foo(a, r)"));
    CHECK_FALSE(contains(wgsl, "mx_foo(a, (*r))"));
}

TEST_CASE("GlslToWgsl: out parameter used on rhs of assignment is dereferenced", "[genglsl][wgsl]")
{
    const std::string block =
        "void mx_polarized(out vec3 Rp, out vec3 Rs)\n"
        "{\n"
        "    Rs = vec3(1.0);\n"
        "    Rp = Rs * vec3(2.0);\n"
        "}\n";
    const std::string wgsl = mx::WgslRewrite::rewriteBlock(block);
    CHECK(contains(wgsl, "(*Rp) = (*Rs) *"));
}

TEST_CASE("GlslToWgsl: mutable value parameter is copied to a local var", "[genglsl][wgsl]")
{
    // GLSL mutates the `uv` parameter; WGSL parameters are immutable, so the
    // parameter is renamed and copied into a mutable local.
    const std::string block =
        "vec2 mx_transform_uv(vec2 uv, vec2 uv_scale, vec2 uv_offset)\n"
        "{\n"
        "    uv = uv * uv_scale + uv_offset;\n"
        "    return uv;\n"
        "}\n";
    const std::string wgsl = mx::WgslRewrite::rewriteBlock(block);
    CHECK(contains(wgsl, "fn mx_transform_uv(uv_arg: vec2f, uv_scale_arg: vec2f, uv_offset_arg: vec2f) -> vec2f"));
    CHECK(contains(wgsl, "var uv: vec2f = uv_arg;"));
    CHECK(contains(wgsl, "var uv_scale: vec2f = uv_scale_arg;"));
    CHECK(contains(wgsl, "uv = uv * uv_scale + uv_offset;"));
}

TEST_CASE("GlslToWgsl: #define becomes const or alias", "[genglsl][wgsl]")
{
    CHECK(mx::WgslRewrite::rewriteAll("#define M_PI 3.1415926") == "const M_PI: f32 = 3.1415926;");
    CHECK(mx::WgslRewrite::rewriteAll("#define CLOSURE_TYPE_REFLECTION 1") == "const CLOSURE_TYPE_REFLECTION: i32 = 1;");
    CHECK(mx::WgslRewrite::rewriteAll("#define EDF vec3") == "alias EDF = vec3f;");
    CHECK(mx::WgslRewrite::rewriteAll("#define material surfaceshader") == "alias material = surfaceshader;");
}

TEST_CASE("GlslToWgsl: for-loop init declaration", "[genglsl][wgsl]")
{
    CHECK(mx::WgslRewrite::rewriteAll("for (int i = 0; i < n; i++)") == "for (var i: i32 = 0; i < n; i += 1)");
}

TEST_CASE("GlslToWgsl: struct member definitions", "[genglsl][wgsl]")
{
    const std::string block =
        "struct ClosureData {\n"
        "    int closureType;\n"
        "    vec3 L;\n"
        "    float occlusion;\n"
        "};\n";
    const std::string wgsl = mx::WgslRewrite::rewriteBlock(block);
    CHECK(contains(wgsl, "struct ClosureData {"));
    CHECK(contains(wgsl, "closureType: i32,"));
    CHECK(contains(wgsl, "L: vec3f,"));
    CHECK(contains(wgsl, "occlusion: f32,"));
    // The closing brace loses its GLSL semicolon.
    CHECK_FALSE(contains(wgsl, "};"));
}

TEST_CASE("GlslToWgsl: function overload resolution", "[genglsl][wgsl]")
{
    const std::string shader =
        "fn mx_f(a: f32, b: f32) -> f32 { return a + b; }\n"
        "fn mx_f(a: f32, b: vec3f) -> vec3f { return b; }\n"
        "fn caller() -> vec3f\n"
        "{\n"
        "    var s: f32 = 1.0;\n"
        "    var v: vec3f = vec3f(0.0);\n"
        "    var r1: f32 = mx_f(s, s);\n"
        "    var r2: vec3f = mx_f(s, v);\n"
        "    return r2;\n"
        "}\n";
    const std::string out = mx::WgslRewrite::resolveOverloads(shader);
    // Definitions are renamed by signature; no name is declared twice.
    CHECK(contains(out, "fn mx_f_f32_f32(a: f32, b: f32)"));
    CHECK(contains(out, "fn mx_f_f32_vec3f(a: f32, b: vec3f)"));
    // Calls resolve by argument type.
    CHECK(contains(out, "mx_f_f32_f32(s, s)"));
    CHECK(contains(out, "mx_f_f32_vec3f(s, v)"));
}

TEST_CASE("GlslToWgsl: integer arguments coerced to bool parameters", "[genglsl][wgsl]")
{
    // MaterialX booleans are i32, but library functions shared with the GLSL backends keep
    // native `bool` parameters. Integer arguments passed to those parameters must be wrapped
    // in an explicit `bool(...)`; genuine bool arguments are left untouched.
    const std::string shader =
        "fn mx_dielectric_bsdf(weight: f32, retroreflective: bool) -> f32 { return weight; }\n"
        "fn caller() -> f32\n"
        "{\n"
        "    var w: f32 = 1.0;\n"
        "    var flag: i32 = 0;\n"
        "    var r: f32 = mx_dielectric_bsdf(w, 0);\n"
        "    r = mx_dielectric_bsdf(w, flag);\n"
        "    r = mx_dielectric_bsdf(w, true);\n"
        "    return r;\n"
        "}\n";
    const std::string out = mx::WgslRewrite::resolveOverloads(shader);
    CHECK(contains(out, "mx_dielectric_bsdf(w, bool(0))"));
    CHECK(contains(out, "mx_dielectric_bsdf(w, bool(flag))"));
    // A real bool literal is not re-wrapped.
    CHECK(contains(out, "mx_dielectric_bsdf(w, true)"));
    CHECK(!contains(out, "bool(true)"));
}

TEST_CASE("GlslToWgsl: dedup duplicate definitions", "[genglsl][wgsl]")
{
    const std::string shader =
        "const M_PI: f32 = 3.14;\n"
        "struct Foo { a: f32, }\n"
        "const M_PI: f32 = 3.14;\n"
        "struct Foo { a: f32, }\n";
    const std::string out = mx::WgslRewrite::dedupDefinitions(shader);
    CHECK(countOccurrences(out, "const M_PI:") == 1);
    CHECK(countOccurrences(out, "struct Foo") == 1);
}

TEST_CASE("GlslToWgsl: multiple declaration split", "[genglsl][wgsl]")
{
    CHECK(mx::WgslRewrite::rewriteAll("vec3 Rp, Rs;") == "var Rp: vec3f; var Rs: vec3f;");
    CHECK(mx::WgslRewrite::rewriteAll("float a, b, c;") == "var a: f32; var b: f32; var c: f32;");
    CHECK(mx::WgslRewrite::rewriteAll("float h = hsv.x; float s = hsv.y; float v = hsv.z;") ==
          "var h: f32 = hsv.x; var s: f32 = hsv.y; var v: f32 = hsv.z;");
    CHECK(mx::WgslRewrite::rewriteAll("float r = c.x; float g = c.y; float b = c.z;") ==
          "var r: f32 = c.x; var g: f32 = c.y; var b: f32 = c.z;");
}

TEST_CASE("GlslToWgsl: braceless control and float literal suffix", "[genglsl][wgsl]")
{
    CHECK(mx::WgslRewrite::rewriteAll("else s = 0.0f;") == "else { s = 0.0; }");
    CHECK(mx::WgslRewrite::rewriteAll("else if (hi == 1) return vec3(q, v, p);") ==
          "else if (hi == 1) { return vec3(q, v, p); }");
    CHECK(mx::WgslRewrite::rewriteAll("if (hi == 0) return vec3(v, t, p);") ==
          "if (hi == 0) { return vec3(v, t, p); }");
    CHECK(mx::WgslRewrite::rewriteAll("h = 6.0f * (h - floor(h));") == "h = 6.0 * (h - floor(h));");
    // Native WGSL type spellings must survive float-literal suffix stripping.
    CHECK(mx::WgslRewrite::rewriteAll("direction: vec3f,") == "direction: vec3f,");
    CHECK(mx::WgslRewrite::rewriteAll("@group(0) @binding(12) var<uniform> foo: vec3f") ==
          "@group(0) @binding(12) var<uniform> foo: vec3f");
    CHECK(mx::WgslRewrite::rewriteAll("normalWorld: mat4x4f,") == "normalWorld: mat4x4f,");
    // `else // comment` with a braced body on the next line must not become an empty block.
    CHECK(mx::WgslRewrite::rewriteAll("else // FRESNEL_MODEL_SCHLICK") == "else // FRESNEL_MODEL_SCHLICK");
    const std::string fresnelBlock =
        "vec3 mx_compute_fresnel(float cosTheta, FresnelData fd)\n"
        "{\n"
        "    else // FRESNEL_MODEL_SCHLICK\n"
        "    {\n"
        "        return mx_fresnel_hoffman_schlick(cosTheta, fd);\n"
        "    }\n"
        "}\n";
    const std::string wgsl = mx::WgslRewrite::rewriteBlock(fresnelBlock);
    CHECK_FALSE(contains(wgsl, "else { // FRESNEL_MODEL_SCHLICK }"));
    CHECK(contains(wgsl, "else // FRESNEL_MODEL_SCHLICK"));
    CHECK(contains(wgsl, "return mx_fresnel_hoffman_schlick(cosTheta, fd);"));
    // Blank line between `else // comment` and `{` must not synthesize an empty block.
    const std::string elseBlankBlock =
        "vec3 f(float x)\n"
        "{\n"
        "    else // FRESNEL_MODEL_SCHLICK\n"
        "\n"
        "    {\n"
        "        return vec3(x);\n"
        "    }\n"
        "}\n";
    const std::string elseBlankWgsl = mx::WgslRewrite::rewriteBlock(elseBlankBlock);
    CHECK_FALSE(contains(elseBlankWgsl, "else { // FRESNEL_MODEL_SCHLICK }"));
    CHECK(contains(elseBlankWgsl, "else // FRESNEL_MODEL_SCHLICK"));
}

TEST_CASE("GlslToWgsl: repair empty else comment blocks", "[genglsl][wgsl]")
{
    const std::string broken =
        "fn a() -> f32 {\n"
        "    else { // FRESNEL_MODEL_SCHLICK }\n"
        "    {\n"
        "        return 1.0;\n"
        "    }\n"
        "}\n"
        "fn b() -> f32 { return 2.0; }\n";
    const std::string fixed = mx::WgslRewrite::repairEmptyElseCommentBlocks(broken);
    CHECK_FALSE(contains(fixed, "else { // FRESNEL_MODEL_SCHLICK }"));
    CHECK(contains(fixed, "else // FRESNEL_MODEL_SCHLICK"));
    CHECK(contains(fixed, "fn b() -> f32 { return 2.0; }"));
}

TEST_CASE("GlslToWgsl: mx_hsv library functions", "[genglsl][wgsl]")
{
    const std::string block =
        "vec3 mx_hsvtorgb(vec3 hsv)\n"
        "{\n"
        "    float h = hsv.x; float s = hsv.y; float v = hsv.z;\n"
        "    if (s < 0.0001f) {\n"
        "      return vec3 (v, v, v);\n"
        "    } else {\n"
        "        h = 6.0f * (h - floor(h));\n"
        "        int hi = int(trunc(h));\n"
        "        if (hi == 0)\n"
        "            return vec3 (v, t, p);\n"
        "        else if (hi == 1) return vec3 (q, v, p);\n"
        "    }\n"
        "}\n"
        "vec3 mx_rgbtohsv(vec3 c)\n"
        "{\n"
        "    float r = c.x; float g = c.y; float b = c.z;\n"
        "    if (maxcomp > 0.0f)\n"
        "        s = delta / maxcomp;\n"
        "    else s = 0.0f;\n"
        "}\n";
    const std::string wgsl = mx::WgslRewrite::rewriteBlock(block);
    CHECK(contains(wgsl, "var h: f32 = hsv.x; var s: f32 = hsv.y; var v: f32 = hsv.z;"));
    CHECK(contains(wgsl, "var r: f32 = c.x; var g: f32 = c.y; var b: f32 = c.z;"));
    CHECK(contains(wgsl, "else { s = 0.0; }"));
    CHECK(contains(wgsl, "else if (hi == 1) { return vec3 (q, v, p); }"));
    CHECK(contains(wgsl, "h = 6.0 * (h - floor(h));"));
    CHECK(contains(wgsl, "var hi: i32 = i32(trunc(h));"));
    CHECK_FALSE(contains(wgsl, "int(trunc"));
    CHECK_FALSE(contains(wgsl, "float s ="));
    CHECK_FALSE(contains(wgsl, "0.0f"));
}

TEST_CASE("GlslToWgsl: reserved-word identifier rename", "[genglsl][wgsl]")
{
    // A GLSL variable named `var` (a WGSL keyword) is renamed (library path only,
    // so the WGSL `var` keyword emitted elsewhere is never touched).
    CHECK(mx::WgslRewrite::rewriteLine("vec3 var = vec3(1.0);") == "var var_: vec3f = vec3(1.0);");
    CHECK(mx::WgslRewrite::rewriteLine("x = var * 2.0;") == "x = var_ * 2.0;");
}

TEST_CASE("GlslToWgsl: combined sampler signature and read", "[genglsl][wgsl]")
{
    const std::string block =
        "vec3 mx_sample(sampler2D tex, vec2 uv)\n"
        "{\n"
        "    return texture(sampler2D(tex_texture, tex_sampler), uv).rgb;\n"
        "}\n";
    const std::string wgsl = mx::WgslRewrite::rewriteBlock(block);
    // sampler2D parameter splits into a texture + sampler pair (handle types, not copied).
    CHECK(contains(wgsl, "tex_texture: texture_2d<f32>, tex_sampler: sampler"));
    // Combined-sampler read becomes WGSL textureSample.
    CHECK(contains(wgsl, "textureSample(tex_texture, tex_sampler, uv)"));
}

TEST_CASE("GlslToWgsl: CRLF line endings do not desync overload resolution", "[genglsl][wgsl]")
{
    // Regression: with CRLF input, std::getline leaves a trailing '\r', so a struct close
    // reads as "}\r". If trim() does not strip it, the overload scanner stays "inside" the
    // struct and silently skips the functions that follow (producing duplicate bare names).
    const std::string shader =
        "struct FresnelData {\r\n"
        "    ior: vec3f,\r\n"
        "}\r\n"
        "fn mx_f(a: f32, b: f32) -> f32 { return a + b; }\r\n"
        "fn mx_f(a: f32, b: vec3f) -> vec3f { return b; }\r\n";
    const std::string out = mx::WgslRewrite::resolveOverloads(shader);
    CHECK(contains(out, "fn mx_f_f32_f32("));
    CHECK(contains(out, "fn mx_f_f32_vec3f("));
}

TEST_CASE("GlslToWgsl: findResidualGlsl flags unconverted GLSL", "[genglsl][wgsl]")
{
    // Fully converted WGSL yields no findings.
    CHECK(mx::WgslRewrite::findResidualGlsl("fn material_main() -> vec4f { return vec4f(0.0); }").empty());
    // GLSL leftovers are reported.
    CHECK_FALSE(mx::WgslRewrite::findResidualGlsl("#version 450\nfn f() {}").empty());
    CHECK_FALSE(mx::WgslRewrite::findResidualGlsl("layout(binding=0) uniform Foo { };").empty());
    CHECK_FALSE(mx::WgslRewrite::findResidualGlsl("sampler2D tex;").empty());
    // Legitimate WGSL texture/sampler declarations must NOT be flagged.
    CHECK(mx::WgslRewrite::findResidualGlsl("var t: texture_2d<f32>;\nvar s: sampler;").empty());
}
