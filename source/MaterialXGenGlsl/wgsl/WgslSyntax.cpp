//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/wgsl/WgslSyntax.h>

#include <MaterialXGenShader/Syntax.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// WGSL uniform buffers cannot hold `bool`, and WGSL has no implicit bool<->int conversion.
// MaterialX booleans are therefore represented as `i32` throughout this backend (uniforms,
// node-function parameters, and locals) so the type is consistent on both sides of every
// call. Boolean literals must consequently be emitted as integer `0`/`1` rather than
// `false`/`true`.
class WgslBooleanTypeSyntax : public ScalarTypeSyntax
{
  public:
    explicit WgslBooleanTypeSyntax(const Syntax* parent) :
        ScalarTypeSyntax(parent, "i32", "0", "0")
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        return value.getValueString() == "true" ? "1" : "0";
    }
};

} // anonymous namespace

WgslSyntax::WgslSyntax(TypeSystemPtr typeSystem) :
    VkSyntax(typeSystem)
{
    // Promote the GLSL type syntaxes inherited from GlslSyntax/VkSyntax to their
    // WGSL spellings, so this generator emits native WGSL types (f32, vec3f, ...)
    // and WGSL value constructors. Declaration order (`var name: type`) is handled
    // by the generator's emit overrides; this class controls type names + literals.
    registerTypeSyntax(
        Type::FLOAT,
        std::make_shared<ScalarTypeSyntax>(this, "f32", "0.0", "0.0"));
    registerTypeSyntax(
        Type::INTEGER,
        std::make_shared<ScalarTypeSyntax>(this, "i32", "0", "0"));
    registerTypeSyntax(
        Type::STRING,
        std::make_shared<ScalarTypeSyntax>(this, "i32", "0", "0"));
    // Booleans map to i32 (see WgslBooleanTypeSyntax): WGSL uniform buffers cannot hold bool,
    // so this keeps the type consistent between uniforms, function parameters and arguments.
    registerTypeSyntax(
        Type::BOOLEAN,
        std::make_shared<WgslBooleanTypeSyntax>(this));
    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<AggregateTypeSyntax>(
            this, "vec2f", "vec2f(0.0)", "vec2f(0.0)",
            EMPTY_STRING, EMPTY_STRING, GlslSyntax::VEC2_MEMBERS));
    registerTypeSyntax(
        Type::VECTOR3,
        std::make_shared<AggregateTypeSyntax>(
            this, "vec3f", "vec3f(0.0)", "vec3f(0.0)",
            EMPTY_STRING, EMPTY_STRING, GlslSyntax::VEC3_MEMBERS));
    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<AggregateTypeSyntax>(
            this, "vec4f", "vec4f(0.0)", "vec4f(0.0)",
            EMPTY_STRING, EMPTY_STRING, GlslSyntax::VEC4_MEMBERS));
    registerTypeSyntax(
        Type::COLOR3,
        std::make_shared<AggregateTypeSyntax>(
            this, "vec3f", "vec3f(0.0)", "vec3f(0.0)",
            EMPTY_STRING, EMPTY_STRING, GlslSyntax::VEC3_MEMBERS));
    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<AggregateTypeSyntax>(
            this, "vec4f", "vec4f(0.0)", "vec4f(0.0)",
            EMPTY_STRING, EMPTY_STRING, GlslSyntax::VEC4_MEMBERS));
    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<AggregateTypeSyntax>(this, "mat3x3f", "mat3x3f(1.0)", "mat3x3f(1.0)"));
    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<AggregateTypeSyntax>(this, "mat4x4f", "mat4x4f(1.0)", "mat4x4f(1.0)"));

    // Add in WGSL specific keywords
    registerReservedWords({ // Keywords (https://www.w3.org/TR/WGSL/#keyword-summary)
                            "alias",
                            "break",
                            "case",
                            "const",
                            "const_assert",
                            "continue",
                            "continuing",
                            "default",
                            "diagnostic",
                            "discard",
                            "else",
                            "enable",
                            "false",
                            "fn",
                            "for",
                            "if",
                            "let",
                            "loop",
                            "override",
                            "requires",
                            "return",
                            "struct",
                            "switch",
                            "true",
                            "var",
                            "while",
                            // Reserved Words (https://www.w3.org/TR/WGSL/#reserved-words)
                            "NULL",
                            "Self",
                            "abstract",
                            "active",
                            "alignas",
                            "alignof",
                            "as",
                            "asm",
                            "asm_fragment",
                            "async",
                            "attribute",
                            "auto",
                            "await",
                            "become",
                            "cast",
                            "catch",
                            "class",
                            "co_await",
                            "co_return",
                            "co_yield",
                            "coherent",
                            "column_major",
                            "common",
                            "compile",
                            "compile_fragment",
                            "concept",
                            "const_cast",
                            "consteval",
                            "constexpr",
                            "constinit",
                            "crate",
                            "debugger",
                            "decltype",
                            "delete",
                            "demote",
                            "demote_to_helper",
                            "do",
                            "dynamic_cast",
                            "enum",
                            "explicit",
                            "export",
                            "extends",
                            "extern",
                            "external",
                            "fallthrough",
                            "filter",
                            "final",
                            "finally",
                            "friend",
                            "from",
                            "fxgroup",
                            "get",
                            "goto",
                            "groupshared",
                            "highp",
                            "impl",
                            "implements",
                            "import",
                            "inline",
                            "instanceof",
                            "interface",
                            "layout",
                            "lowp",
                            "macro",
                            "macro_rules",
                            "match",
                            "mediump",
                            "meta",
                            "mod",
                            "module",
                            "move",
                            "mut",
                            "mutable",
                            "namespace",
                            "new",
                            "nil",
                            "noexcept",
                            "noinline",
                            "nointerpolation",
                            "non_coherent",
                            "noncoherent",
                            "noperspective",
                            "null",
                            "nullptr",
                            "of",
                            "operator",
                            "package",
                            "packoffset",
                            "partition",
                            "pass",
                            "patch",
                            "pixelfragment",
                            "precise",
                            "precision",
                            "premerge",
                            "priv",
                            "protected",
                            "pub",
                            "public",
                            "readonly",
                            "ref",
                            "regardless",
                            "register",
                            "reinterpret_cast",
                            "require",
                            "resource",
                            "restrict",
                            "self",
                            "set",
                            "shared",
                            "sizeof",
                            "smooth",
                            "snorm",
                            "static",
                            "static_assert",
                            "static_cast",
                            "std",
                            "subroutine",
                            "super",
                            "target",
                            "template",
                            "this",
                            "thread_local",
                            "throw",
                            "trait",
                            "try",
                            "type",
                            "typedef",
                            "typeid",
                            "typename",
                            "typeof",
                            "union",
                            "unless",
                            "unorm",
                            "unsafe",
                            "unsized",
                            "use",
                            "using",
                            "varying",
                            "virtual",
                            "volatile",
                            "wgsl",
                            "where",
                            "with",
                            "writeonly",
                            "yield" });
}

MATERIALX_NAMESPACE_END
