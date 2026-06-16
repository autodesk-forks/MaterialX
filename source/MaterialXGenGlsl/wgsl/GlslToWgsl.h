//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GLSLTOWGSL_H
#define MATERIALX_GLSLTOWGSL_H

/// @file
/// GLSL-to-WGSL string rewriting utilities for the WGSL shader generator.
///
/// The MaterialX GLSL code path (HwSurfaceNode, CompoundNode, SourceCodeNode and
/// the genglsl `.glsl` library sources) emits GLSL-flavoured text. These helpers
/// convert the common GLSL patterns to valid WGSL at emit time, so that the WGSL
/// generator can reuse the genglsl node implementations and library functions
/// without maintaining a parallel set of `.wgsl` sources:
///   - types:               `float`/`vec3`/`mat4`        -> `f32`/`vec3f`/`mat4x4f`
///   - const declarations:  `const float x = 1.0`        -> `const x: f32 = 1.0`
///   - variable decls:      `vec3 v = ...`               -> `var v: vec3f = ...`
///   - ternary operators:   `a ? b : c`                  -> `select(c, b, a)`
///   - function signatures: `void f(float x, out int y)` -> `fn f(x: f32) -> i32`
///   - out parameters:      body refs to `y` become `(*y)` with `y: ptr<function,T>`
///   - increment/decrement: `i++`                        -> `i += 1`
///   - combined samplers:   `sampler2D tex`              -> `tex_texture: texture_2d<f32>, tex_sampler: sampler`
///   - texture sampling:    `texture(tex, uv)`           -> `mx_texture_sample(tex_texture, tex_sampler, uv)`
///
/// This is an independent implementation; it shares no code with the LMV
/// `contrib/adsk/lmv` generators.
///
/// Scope and assumptions: these helpers are a *targeted* converter for the constrained,
/// machine-generated GLSL produced by the MaterialX genglsl node implementations and library
/// sources -- not a general-purpose GLSL parser. They assume single-statement-per-line output,
/// the signature/expression shapes the generator emits, and that input is well formed. Comments
/// and string literals are not specially protected, and the minimal `#if` evaluator recognizes
/// only the macros the standard libraries use. As a safety net, the generator runs
/// `findResidualGlsl()` on its output and warns if any unconverted GLSL remains, so an
/// incomplete rewrite is observable rather than silently producing invalid WGSL.

#include <MaterialXGenGlsl/Export.h>

#include <MaterialXCore/Library.h>

#include <string>
#include <utility>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

namespace WgslRewrite
{

/// Map a GLSL type keyword to its WGSL equivalent.
/// Returns the input unchanged if it is already a WGSL type or unknown.
MX_GENGLSL_API string mapType(const string& glslType);

/// Apply the stateless, single-line GLSL->WGSL syntax conversions (float cast,
/// const/var declarations, ternaries, increment/decrement, function signatures).
/// Does not handle out-parameter body dereferencing -- use LineRewriter for that.
MX_GENGLSL_API string rewriteAll(string line);

/// Split GLSL combined-sampler parameters in a line:
/// `sampler2D tex` -> `tex_texture: texture_2d<f32>, tex_sampler: sampler`.
MX_GENGLSL_API string rewriteSamplerParams(string line);

/// Rewrite GLSL texture sampling calls to the WGSL helper:
/// `texture(tex, uv)` -> `mx_texture_sample(tex_texture, tex_sampler, uv)`.
MX_GENGLSL_API string rewriteTextureSampling(string line);

/// Rewrite a single GLSL line to WGSL with no surrounding function context.
MX_GENGLSL_API string rewriteLine(const string& line);

/// Rewrite a whole multi-line GLSL block to WGSL. `#include` directives are
/// preserved verbatim so the caller can resolve and rewrite them with path context.
MX_GENGLSL_API string rewriteBlock(const string& block);

/// Convert HwNumLightsNode / HwLightSamplerNode GLSL function stubs still present
/// after per-line rewriting. Does not touch native WGSL module bindings.
MX_GENGLSL_API string rewriteResidualGlslFunctions(const string& shader);

/// Drop duplicate module-scope definitions (the same `const`/`alias`/`struct`/`fn`
/// emitted by more than one library file), keeping the first. WGSL forbids
/// redeclaration; genglsl headers are include-guarded per file but the same symbol
/// is often defined across several files.
MX_GENGLSL_API string dedupDefinitions(const string& shader);

/// Dereference `ptr<function,T>` (out) parameters in *compound* function bodies,
/// which assign to them by bare name (`out1 = ...`). Library functions already use
/// `(*out1)` and are left untouched (gated on the absence of `(*` in the body).
MX_GENGLSL_API string derefPointerParams(const string& shader);

/// Resolve GLSL function overloading (which WGSL forbids) across a whole WGSL
/// shader: functions defined more than once under the same name get a unique
/// type-suffixed name (e.g. `mx_fresnel_schlick_f32_f32`), and every call site is
/// rewritten to the matching overload via argument-type inference. Assumes the
/// single-line function signatures and call expressions this generator emits.
MX_GENGLSL_API string resolveOverloads(const string& shader);

/// Wrap integer MaterialX boolean values in `bool(...)` at call sites whose callee
/// declares a native WGSL `bool` parameter (shared GLSL library functions such as
/// `mx_dielectric_bsdf`). Always run after `resolveOverloads`.
MX_GENGLSL_API string coerceBoolCallSites(const string& shader);

/// Fix `else { // comment }` empty blocks that orphan the real `{ ... }` body and
/// break brace balance in generated WGSL.
MX_GENGLSL_API string repairEmptyElseCommentBlocks(const string& shader);

/// Scan a generated WGSL shader for residual GLSL tokens that indicate an incomplete
/// GLSL->WGSL rewrite -- e.g. `#version`, `layout(`, combined `sampler2D`/`texture2D`
/// declarations, or `gl_*` builtins. Returns a (deduplicated) list of human-readable
/// findings; empty when the output looks fully converted. The generator logs these as
/// warnings so a failed rewrite surfaces instead of silently emitting invalid WGSL.
/// @param wgsl The generated WGSL source to scan.
/// @return One entry per distinct residual-GLSL token found (empty if none).
MX_GENGLSL_API StringVec findResidualGlsl(const string& wgsl);

/// Stateful, function-scope-aware GLSL-to-WGSL line rewriter.
///
/// Feed it one line at a time in source order. It tracks the current function
/// body (via brace depth) so that:
///   - a function signature `void f(..., out T r)` becomes
///     `fn f(..., r: ptr<function, T>)` (no `-> T` return), and
///   - every reference to an `out` parameter `r` inside the body becomes `(*r)`.
/// Value-returning signatures (`vec3 g(...)`) gain a `-> vec3f` return type.
class MX_GENGLSL_API LineRewriter
{
  public:
    /// Rewrite one line (without trailing newline). Updates internal scope state.
    string rewrite(const string& line);

  private:
    /// Enter a function body: record out-parameter names, the mutable `var` copies
    /// to inject for value parameters, and brace state; return the (already-rewritten)
    /// signature line for emission.
    string beginFunction(const string& sig, std::vector<string>&& outNames,
                         const std::vector<std::pair<string, string>>& valueParams);

    bool _funcActive = false;       ///< inside a function body (or its signature)
    bool _seenOpenBrace = false;    ///< the function's opening brace has been seen
    int _braceDepth = 0;            ///< net brace nesting within the current function
    std::vector<string> _outParams; ///< out-parameter names to dereference

    std::vector<string> _paramInjections; ///< `var name: T = name_arg;` lines to inject at body start
    bool _injected = false;               ///< whether the injections have been emitted

    bool _accumulatingSig = false; ///< buffering a signature whose params span lines
    string _sigBuffer;             ///< accumulated multi-line signature text
    int _sigParenDepth = 0;        ///< open-paren nesting while accumulating

    bool _accumTernary = false; ///< buffering a ternary expression that spans lines
    string _ternaryBuffer;      ///< accumulated multi-line ternary text

    bool _pendingHeader = false; ///< a braceless control header awaiting its body
    string _headerLine;          ///< the buffered `if(...)`/`for(...)`/`else` header

    bool _inStruct = false; ///< inside a `struct { ... }` definition body

    /// A `#if/#elif/#else/#endif` conditional frame for the minimal preprocessor.
    struct PPFrame
    {
        bool active;
        bool taken;
        bool parentActive;
    };
    std::vector<PPFrame> _ppStack; ///< active preprocessor conditional frames

    /// True when all enclosing preprocessor conditionals select the current line.
    bool ppActive() const;
    /// Handle a `#if/#ifdef/#ifndef/#elif/#else/#endif` directive; updates _ppStack.
    void updatePreprocessor(const string& directive);
};

} // namespace WgslRewrite

MATERIALX_NAMESPACE_END

#endif // MATERIALX_GLSLTOWGSL_H
