# Add a native Vulkan renderer module (`MaterialXRenderVk`)

## Context

MaterialX ships hardware render backends for OpenGL (`MaterialXRenderGlsl`) and Metal
(`MaterialXRenderMsl`), plus a Slang/slang-rhi backend in this fork
(`MaterialXRenderSlang`). There is no native Vulkan renderer.

The gap is narrower than it looks, and lopsided:

- **Vulkan shader generation is already done and shipping.**
  `source/MaterialXGenGlsl/VkShaderGenerator.{h,cpp}`, `VkSyntax.{h,cpp}` and
  `VkResourceBindingContext.{h,cpp}` all exist and are current. Critically,
  `VkShaderGenerator::TARGET == "genglsl"` (`VkShaderGenerator.cpp:12`), so Vulkan reuses the
  entire `genglsl` data library — **no** new `libraries/targets/*.mtlx`, **no** new
  `libraries/*/genvulkan/` implementations, **no** `_options.mtlx` change.
- **Nothing consumes that output at runtime.** CI validates the generated Vulkan GLSL with
  `glslangValidator` (`.github/workflows/main.yml:292`), but no MaterialX code compiles it to
  SPIR-V or renders with it. The Vulkan codegen has no render-level regression coverage.

Prior art exists but is not usable as-is. `origin/vulkan/vkRenderer` (Aug 2025, 323 commits
behind — archived to `vulkan/vkRenderer_old` as step 1 below) has a ~5,700-line
`source/MaterialXRenderVk` in one WIP commit. Its Vulkan plumbing is
sound; its MaterialX integration is absent — both `createProgram()` overloads are
commented-out stubs, `captureImage()` returns `nullptr`, `render()` draws hardcoded NVIDIA
sample shaders against a hardcoded `F:/source/MaterialX/.../teapot.obj`, and it presents to a
visible Win32 swapchain. `VkProgram.h` is a copy of `MslProgram.h` with the Metal bodies
commented out plus a stray `private:` that makes its entire public API unreachable, and
`VulkanDevice` sits outside `MATERIALX_NAMESPACE` while exposing `glm::mat4x4`.
`origin/adsk_contrib/vulkanRender` (2023) is a strict subset with no SPIR-V path at all.

**Outcome:** a first-class `MaterialXRenderVk` that mirrors the `GlslRenderer`/`MslRenderer`
API contracts, renders the test suite offscreen under Vulkan, bakes textures, and drives
MaterialXView — giving the already-shipping Vulkan shader generator real render coverage.

## Decisions (settled)

| Decision | Choice |
|---|---|
| Approach | Fresh native rewrite (`vulkan.hpp`); `vulkan/vkRenderer_old` referenced for plumbing only; keep current repo coding style and API contracts |
| Branch | Archive `origin/vulkan/vkRenderer` → `vulkan/vkRenderer_old`; cut a new `vulkan/vkRenderer` from `adsk_contrib/dev` |
| Plan location | Committed to `C:\source\MaterialX-adsk\plans\vulkan-renderer.md` |
| Scope | Renderer + `TextureBakerVk` + render tests + MaterialXView backend |
| Platforms | Windows and Linux only |
| Build | `find_package(Vulkan)` non-`REQUIRED`; `MATERIALX_BUILD_RENDER_VK` auto-ON when found; glslang from the Vulkan SDK |
| Viewer | `VulkanRenderPipeline` renders offscreen, blitted into the existing GL-backed NanoGUI window; NanoGUI **not** forked |
| Clip space | Add `MatrixConvention::Vulkan` to `source/MaterialXRender/ShaderRenderer.h` |
| Naming | `source/MaterialXRenderVk`, `MATERIALX_BUILD_RENDER_VK`, `MX_RENDERVK_API` / `MATERIALX_RENDERVK_EXPORTS`; classes `VkContext`, `VkRenderer`, `VkProgram`, `VkMaterial`, `VkFramebuffer`, `VkTextureHandler`, `TextureBakerVk` |

---

# Before you start

## 1. Branch setup — archive the old WIP, start a fresh `vulkan/vkRenderer`

`origin/vulkan/vkRenderer` holds the Aug 2025 WIP described in Context. We keep it for
reference but do the new work on a clean branch of the same name, cut from
`adsk_contrib/dev`.

> **This deletes a remote branch.** Create the archive copy and verify it *before* deleting
> anything. Anyone with `vulkan/vkRenderer` checked out will need to re-point afterwards, so
> announce it. GitHub's branch-rename API (`gh api -X POST
> repos/autodesk-forks/MaterialX/branches/vulkan%2FvkRenderer/rename -f
> new_name='vulkan/vkRenderer_old'`) is atomic and preserves any open PR, so prefer it if `gh`
> is available — it is **not** installed on this machine, hence the plain-git sequence below.

```powershell
git fetch origin

# Record the current tip so the archive can be verified (expect c8869283).
git rev-parse origin/vulkan/vkRenderer

# 1. Create the archive branch from the existing remote tip.
git push origin refs/remotes/origin/vulkan/vkRenderer:refs/heads/vulkan/vkRenderer_old

# 2. Verify the archive exists and matches before deleting anything.
git fetch origin
git rev-parse origin/vulkan/vkRenderer_old   # must equal the SHA from above

# 3. Only now delete the original.
git push origin --delete vulkan/vkRenderer

# 4. Create the new branch from adsk_contrib/dev and publish it.
git checkout -b vulkan/vkRenderer origin/adsk_contrib/dev
git push -u origin vulkan/vkRenderer
```

Note the starting point matters: `git rev-parse --abbrev-ref HEAD` currently reports `main`,
but PRs target `adsk_contrib/dev`. The two differ by ~1,800 lines across 61 files, *including
`source/MaterialXTest/MaterialXRender/RenderUtil.h`* — a file this plan modifies.

Throughout the plan, reference the old code as
`git show origin/vulkan/vkRenderer_old:source/MaterialXRenderVk/<file>`.

## 2. Install the LunarG Vulkan SDK (≥ 1.3.2xx). `$env:VULKAN_SDK` is currently empty and
`C:\VulkanSDK` does not exist on this machine. You need the SDK rather than just
`libvulkan-dev` because you need the `glslang` **libraries and headers**, not just the
`glslangValidator` executable that CI already uses.

- Windows: <https://vulkan.lunarg.com/sdk/home#windows> — install with the default components.
- Linux: `sudo apt install vulkan-sdk` from the LunarG apt repo (the distro `libglslang-dev`
  package often does **not** register CMake's `Vulkan::glslang` component).

Verify: `echo $env:VULKAN_SDK` is non-empty and `vulkaninfo --summary` lists a device.

## 3. CMake version is fine

`cmake_minimum_required(VERSION 3.26)` (`CMakeLists.txt:8`) is
above the 3.24 needed for the `Vulkan::glslang` imported target, so you can rely on it.

---

# Background: the three things a junior dev needs to understand first

## A. How MaterialX renderers are structured

Every backend follows the same five-class shape. Learn one and you know them all:

| Role | OpenGL | Metal | Yours |
|---|---|---|---|
| Device/context | `GLContext` | `MetalState` (global) | `VkContext` |
| The renderer (`ShaderRenderer` subclass) | `GlslRenderer` | `MslRenderer` | `VkRenderer` |
| Compiled program + introspection + binding | `GlslProgram` | `MslProgram` | `VkProgram` |
| Offscreen target | `GLFramebuffer` | `MetalFramebuffer` | `VkFramebuffer` |
| Texture upload (`ImageHandler` subclass) | `GLTextureHandler` | `MetalTextureHandler` | `VkTextureHandler` |
| Material (`ShaderMaterial` subclass) | `GlslMaterial` | `MslMaterial` | `VkMaterial` |
| Baker | `TextureBakerGlsl` | `TextureBakerMsl` | `TextureBakerVk` |

**Read `source/MaterialXRenderGlsl/GlslProgram.cpp` end to end before writing any code.** It is
1,298 lines and it is the specification you are re-implementing. `VkProgram` is
`GlslProgram` with GL calls swapped for Vulkan ones.

## B. How the shader generator hands you a deterministic layout

`GlslShaderGenerator::generate()` (`source/MaterialXGenGlsl/GlslShaderGenerator.cpp:141-158`)
calls `resourceBindingCtx->initialize()` **once**, then emits the VERTEX stage, then the PIXEL
stage. `VkResourceBindingContext::initialize()` resets a counter to 0
(`VkResourceBindingContext.cpp:21-25`), and each emitted resource post-increments it
(`VkResourceBindingContext.cpp:65, 87, 173`).

Consequences you can rely on:

- The vertex UBO gets `binding=0`; pixel UBOs get `1, 2, …`; pixel samplers get their own
  bindings; `LightData_pixel` gets one.
- **No `set=` qualifier is emitted** → everything lives in descriptor set 0.
- Bindings are unique *across both stages* within one program → **one
  `vk::DescriptorSetLayout` with `vk::ShaderStageFlagBits::eAllGraphics` on every binding is
  correct and sufficient.** You do not need per-stage sets.
- UBOs are `layout(std140, ...)` (`VkResourceBindingContext.cpp:65`).

Latent hazard, worth a code comment: `VkShaderGenerator::_resourceBindingCtx` is a mutable
member, so two threads calling `generate()` on one generator instance would corrupt the
counter. Not a regression, but don't parallelize the Vk tester without cloning the generator.

## C. Clip space — the counter-intuitive part, get this right once

Vulkan differs from OpenGL in two ways. Only one of them needs code.

**Depth: yes, you need a change.** GL clip depth is `[-1, 1]`; Vulkan is `[0, 1]` — same as
Metal. `Camera::createPerspectiveMatrixZP` (`source/MaterialXRender/Camera.h:168`) already
exists for exactly this.

**Y orientation: no, you need nothing.** This trips people up, so reason it through:

- In GL, NDC `y = -1` maps to window `y = 0`, and `glReadPixels` returns row 0 from the
  bottom. So **GL buffer row 0 ⟷ NDC y = -1**.
- In Vulkan with a standard positive-height `VkViewport`, NDC `y = -1` maps to attachment row
  0 (the top). So **Vulkan buffer row 0 ⟷ NDC y = -1**.

They already agree. Therefore:

- Do **not** negate `projectionMatrix[1][1]`.
- Do **not** use a negative-height viewport (`VK_KHR_maintenance1`).
- Keep `vk::FrontFace::eCounterClockwise` — there is no handedness change, so winding is
  unaffected.
- Raw readback byte order matches `GLFramebuffer::getColorImage()`
  (`source/MaterialXRenderGlsl/GLFramebuffer.cpp:168`), so the Vk render test saves with
  `verticalFlip = true` exactly like GLSL (`RenderGlsl.cpp:372`) and Metal.

If your first rendered image comes out upside-down, the bug is somewhere else — resist the
urge to "fix" it with a flip.

---

# Key design decisions

## Uniform introspection: glslang reflection

You need, for every uniform: its descriptor binding, its byte offset inside a `std140` block,
and its MaterialX metadata (path, unit, colorspace, default value). Three ways to get the
first two:

| | SPIRV-Reflect / spirv-cross | **glslang `buildReflection()`** | Replay the binding counter yourself |
|---|---|---|---|
| New dependency | Yes (~9k LOC vendored; `FindVulkan` exposes no target for either) | **None** — `Vulkan::glslang` is already needed for GLSL→SPIR-V | None |
| Works for `createProgram(const StageMap&)` (raw source, no `Shader`) | Yes | **Yes** | **No** — breaks `loadSource()` and the viewer's shader-reload |
| std140 offsets | Yes | **Yes** (`TObjectReflection::offset`) | You'd re-derive std140 by hand |
| Survives genglsl emission changes | Yes | **Yes** | No — silently desyncs |

**Use glslang reflection** as the source of truth for layout, then decorate each entry with
MaterialX metadata from the `Shader`'s `VariableBlock`s. This is structurally the same
"introspect the API, then decorate from the Shader" merge that
`GlslProgram::updateUniformsList()` (`GlslProgram.cpp:900-1060`) already does — you are
following an existing repo pattern, not inventing one.

One important detail that makes this work cleanly: `VkResourceBindingContext.cpp:65-66` emits
UBOs **without an instance name** (`uniform PublicUniforms_pixel { ... };`), so glslang
reflects members under their bare names (`u_worldMatrix`, not
`PublicUniforms_pixel.u_worldMatrix`). That preserves the `getUniformsList()` key naming
contract with `GlslProgram` exactly, so downstream code needs no changes.

### Fix the branch's per-stage `TProgram` mistake

The old branch's `GLSLtoSPV()` builds a **fresh** `glslang::TProgram` per stage,
which prevents cross-stage reflection with the unified binding numbering the generator emits.
Add both `TShader`s to **one** `TProgram`:

```cpp
glslang::TShader vs(EShLangVertex), fs(EShLangFragment);
// setEnvInput / setEnvClient / setEnvTarget per stage:
//   EShSourceGlsl, EShClientVulkan, EShTargetVulkan_1_2, EShTargetSpv_1_5
// (lifted verbatim from origin/vulkan/vkRenderer:.../VkProgram.cpp:83-90)

glslang::TProgram program;
program.addShader(&vs);
program.addShader(&fs);
if (!program.link(EShMsgDefault))
    throw ExceptionRenderError("Vulkan GLSL link failed", { program.getInfoLog() });

program.buildReflection(EShReflectionSeparateBuffers |
                        EShReflectionAllBlockVariables |
                        EShReflectionAllIOVariables |
                        EShReflectionSharedStd140UBO);

glslang::GlslangToSpv(*program.getIntermediate(EShLangVertex),   _spirv[VERTEX_STAGE]);
glslang::GlslangToSpv(*program.getIntermediate(EShLangFragment), _spirv[PIXEL_STAGE]);
```

Also replace the branch's hand-rolled 90-line `TBuiltInResource` literal with
`GetDefaultResources()` from `<glslang/Public/ResourceLimits.h>`, overriding only what differs.

Then walk the reflection:

- `getNumUniformBlocks()` / `getUniformBlock(i)` → `{name, getBinding(), size}` → one
  `UniformBlock` record per UBO, each with a **zero-initialized** `std::vector<uint8_t>`
  shadow buffer and a host-visible + coherent `vk::Buffer`.
- `getNumUniformVariables()` / `getUniform(i)` → `index` (owning block, or `-1`), `offset`
  (std140 byte offset), `getType()`, `getBinding()` (for samplers).
- `getNumPipeInputs()` / `getPipeInput(i)` on the vertex stage → `i_position`, `i_normal`, …
  with `getType()->getQualifier().layoutLocation` → `getAttributesList()`.

## Three Vulkan-specific traps that produce wrong output, not crashes

These are the bugs that will cost you days if you don't plan for them.

**1. Uninitialized UBO memory.** `vk::DeviceMemory` contents are undefined at allocation.
GL has default uniform values and D3D11 zero-initializes constant buffers — Vulkan does
neither. If you don't seed every `std140` block from the `Shader`'s `VariableBlock` defaults
before the first draw, materials render as *garbage*, not as an obvious black. Do this inside
`VkProgram::build()`, before any external `bindUniform()` call. Model it on
`SlangProgram::bindUniformDefaults()` (`SlangProgram.h:225`).

**2. `mat3` in std140 is 48 bytes, not 36.** A `mat3` is laid out as 3 × `vec4`. GL's
`glUniformMatrix3fv(loc, 1, GL_FALSE, m.data())` (`GlslProgram.cpp:781`) takes tightly-packed
36 bytes. A naive port that `memcpy`s `Matrix33::data()` writes 36 bytes and **corrupts
everything after it in the block**. Write it row-expanded, 16 bytes per row.

**3. Every sampler binding must be written before `vkCmdBindDescriptorSets`.** GL tolerates an
unbound sampler; Vulkan calls it undefined behaviour and some drivers fault. Add a
`writeUnboundSamplersWithZeroImage()` sweep at the end of `VkProgram::bind()` that fills any
sampler binding not yet written with `ImageHandler::getZeroImage()`
(`source/MaterialXRender/ImageHandler.h:248`).

## Headless-first: no window, no swapchain

Unlike the branch, `VkRenderer` must not need a window. Vulkan offscreen rendering needs no
surface and no swapchain, so **do not link `MaterialXRenderHw`** — upstream PR #2897's
`MaterialXRenderHlsl` set the same precedent. This also means enabling Vulkan does not force
`MATERIALX_BUILD_RENDER_HW` on.

Do not request `VK_KHR_surface` / `VK_KHR_win32_surface` (the branch does, unconditionally).
Request only `VK_EXT_debug_utils`, and only in validation builds.

---

# Phases

Each phase builds and is verifiable on its own. Ordered so the riskiest unknown is settled
before any renderer code is written.

## Phase 0 — Reflection spike (½–1 day, throwaway code)

**Purpose: answer three questions that determine ~300 lines of `VkProgram.cpp`.** Do not skip
this; it is cheap insurance.

Write a scratch `main.cpp` **outside the repo** that:

1. Generates a real VS/PS pair with lights enabled, either by calling `VkShaderGenerator`
   directly or via
   `python python/Scripts/generateshader.py resources/Materials/Examples/StandardSurface --target vulkan`.
2. Feeds both stages into one `TProgram`, links, and calls `buildReflection()` with the flags
   above.
3. Dumps every uniform block (name / binding / size), every uniform (name / offset / type /
   binding / blockIndex), and every pipe input (name / location).

**Answer and write down:**

- **Q1.** Are `u_lightData[0].type`, `u_lightData[0].direction`, … reflected under composed
  names, or only `u_lightData`? *This is the highest-risk unknown in the project.* If only the
  latter, adopt the fallback in Risk R2 below before writing the light-binding code.
- **Q2.** Do reported offsets match std140 for a `mat4` in a UBO (16-byte base alignment,
  64-byte stride)? And for `mat3` (48 bytes)?
- **Q3.** Are sampler bindings reported via `getUniform(i).getBinding()`, and do they equal
  the counter values `VkResourceBindingContext` emitted?

**Deliverable:** a one-page note attached to the PR recording the answers.

## Phase 1 — Module skeleton, CMake, SPIR-V compile (2–3 days)

Goal: `MaterialXRenderVk` builds on Windows and Linux; a `VkRenderer` can be constructed,
initialized, and can compile a generated shader to SPIR-V. No rendering yet.

**Create:**

| File | Purpose | ~lines (h/cpp) |
|---|---|---|
| `source/MaterialXRenderVk/CMakeLists.txt` | Module build | 55 |
| `source/MaterialXRenderVk/Export.h` | `MX_RENDERVK_API` | 25 |
| `source/MaterialXRenderVk/VkUtil.{h,cpp}` | `VK_CHECK` throwing `ExceptionRenderError`, `vk::Result`→string, one-time command helper, `findMemoryType()`, `transitionImageLayout()`, debug-messenger callback | 70 / 230 |
| `source/MaterialXRenderVk/VkContext.{h,cpp}` | Instance, physical device, logical device, queue, command pool | 150 / 480 |
| `source/MaterialXRenderVk/VkProgram.{h,cpp}` | *First slice only:* stages + compile to SPIR-V + link + reflect | 180 / 450 |
| `source/MaterialXRenderVk/VkRenderer.{h,cpp}` | *Stub:* `initialize()`, `createProgram()`; everything else throws | 170 / 200 |

Copy the Apache-2.0 SPDX header from any current file — **not** the 2021 Lucasfilm header the
branch's `Export.h` carries.

**Modify:** root `CMakeLists.txt` (option + dispatch, see "CMake edits" below);
`documents/CMakeLists.txt:18`.

**Verify:**

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DMATERIALX_BUILD_TESTS=ON
cmake --build build --config Release --target MaterialXRenderVk -j
```

- Configure output contains `Vulkan found at ...; enabling MATERIALX_BUILD_RENDER_VK`.
- `MaterialXRenderVk.lib` exists.
- Reconfigure with `-DMATERIALX_BUILD_RENDER_VK=OFF` → full build still clean.
- Also verify `-DMATERIALX_BUILD_MONOLITHIC=ON -DMATERIALX_BUILD_RENDER_VK=ON` configures and
  builds (see Risk R4).
- Linux gate: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)`.

## Phase 2 — Reflection, descriptor layout, UBO staging (3–4 days)

Goal: `VkProgram::build()` produces a `vk::DescriptorSetLayout`, a `vk::PipelineLayout`,
per-block UBOs with seeded shadow buffers, and populated `getUniformsList()` /
`getAttributesList()`. Still no draw.

Complete `VkProgram` (grows to ~330 / ~1400):

- `build()` → glslang link + reflect + SPIR-V + two `vk::ShaderModule`s.
- `updateUniformsList()` — reflection first, then decorate from the `Shader`'s uniform blocks,
  mirroring `GlslProgram.cpp:908-1060` **including** the recursive struct-member expansion at
  `:1010-1030`.
- `bindUniformDefaults()` — see trap #1 above. Mandatory.
- `bindUniform(name, value, errorIfMissing)` — find in `_uniformList`, write into
  `_blocks[input->blockIndex].shadow` at `input->offset`, mark dirty. Type dispatch mirrors
  `GlslProgram::bindUniformLocation` (`GlslProgram.cpp:729-793`) **except `Matrix33`** — see
  trap #2.
- `flushUniforms()` — `memcpy` dirty shadows into mapped coherent memory.

**Verify** with a temporary `TEST_CASE("Render: Vulkan Reflection", "[rendervk]")` that
generates `standard_surface_default` with `VkShaderGenerator`, calls `createProgram(shader)`,
and `CHECK`s:

- `getUniformsList()` contains `u_worldMatrix`, `u_viewPosition`, `u_envRadiance`,
  `u_numActiveLightSources`.
- `getAttributesList()` contains `i_position` at location 0.
- `printUniforms(std::cout)` output is sane.
- A known `Matrix33` bound via `bindUniform` reads back correctly from the shadow buffer
  (this is your regression test for trap #2).

```powershell
ctest --test-dir build -C Release -R MaterialXTest_Render_Vulkan_Reflection -V
```

Fold this into the main suite at the end of Phase 3.

## Phase 3 — Offscreen render path (4–6 days) — the milestone that proves the design

Goal: `VkRenderer::render()` + `captureImage()` produce a correct PNG for a textured, lit, IBL
material at 512×512.

**Create:** `VkFramebuffer.{h,cpp}` (~115 / 380), `VkTextureHandler.{h,cpp}` (~130 / 560).
**Complete:** `VkProgram` pipeline creation, mesh/partition buffers, descriptor writes, and the
five `bind*` routines. `VkRenderer::render()`, `renderTextureSpace()`, `captureImage()`,
`setSize()`, `validateInputs()`.

**Do it in this order — each step is visually verifiable, so a regression is always one step
back:**

1. **Clear only.** `captureImage()` returns a solid `_screenColor` PNG. Proves framebuffer +
   readback + row order.
2. **Mesh + constant-output shader.** Silhouette of `sphere.obj`. Proves vertex/index buffers,
   pipeline, viewport, depth, front face.
3. **`bindUniformDefaults()` + `bindViewInformation()`.** Correctly-shaded unlit surface.
   *This is where you confirm the ZP projection* — compare against the GLSL renderer's PNG for
   the same element.
4. **`bindTextures()`.** Textured surface.
5. **`bindEnvironmentImages()` + `bindLightingScalars()`.** IBL.
6. **`bindLightSources()`.** Direct lighting.

**Gate for step 6:** enable `VK_LAYER_KHRONOS_validation` (`VkContext` turns it on when
`NDEBUG` is undefined) and require **zero validation errors**.

## Phase 4 — `VkMaterial` and `TextureBakerVk` (2 days)

**Create:** `VkMaterial.{h,cpp}` (~135 / 480) implementing the full `ShaderMaterial` override
set (`source/MaterialXRender/ShaderMaterial.h:70-142`); `TextureBaker.{h,cpp}` (~48 / 15).

The baker needs **zero template changes**. `TextureBaker<Renderer, ShaderGen>`
(`source/MaterialXRender/TextureBaker.inl:52-101, 229-237`) requires exactly:
`Renderer(w,h,baseType)`, `initialize()`, `createImageHandler(ImageLoaderPtr)`,
`getFramebuffer()->setEncodeSrgb(bool)`, `createProgram(ShaderPtr)`,
`renderTextureSpace(min,max)`, `captureImage(ImagePtr)`, `_imageHandler`, `_baseType` — all
delivered by Phases 2–3. `_generator` is declared `ShaderGeneratorPtr` (`TextureBaker.h:288`),
so `VkShaderGenerator::create()` fits.

**Verify:** bake `resources/Materials/Examples/StandardSurface/standard_surface_marble_solid.mtlx`
and compare the emitted textures against `TextureBakerGlsl` output.

## Phase 5 — Render test harness (2 days)

**Create:** `source/MaterialXTest/MaterialXRenderVk/CMakeLists.txt` (7 lines, byte-identical
to the RenderGlsl one) and `RenderVk.cpp` (~420, a copy of `RenderSlang.cpp` with the
substitutions below).

**Modify:** `RenderUtil.{h,cpp}` (the `getTestSuiteName()` virtual),
`source/MaterialXTest/CMakeLists.txt`, root `CMakeLists.txt` (`MATERIALX_TEST_RENDER_VK`).

### Resolving the collision with the GLSL tester

Because `VkShaderGenerator::TARGET == "genglsl"`, the Vk tester already passes the
`runTest()` gate (`RenderUtil.h:224`) with no `_options.mtlx` change — but it would clobber the
GLSL tester's logs. `RenderUtil.cpp:76` reads the target and passes it to
`logger.start(target, …)`, which writes `<target>_render_log.txt`,
`<target>_render_doc_validation_log.txt` and `<target>_render_profiling_log.txt`
(`RenderUtil.cpp:614-632`).

Add to `ShaderRenderTester`'s protected section (`RenderUtil.h`, near line 221):

```cpp
// Name used to disambiguate this tester's log files from other testers.
// Defaults to the shader generator's target. Backends that share a target with
// another backend (Vulkan GLSL and OpenGL GLSL both report "genglsl") must
// override this to avoid clobbering each other's logs.
virtual std::string getTestSuiteName() const { return _shaderGenerator->getTarget(); }
```

Change `RenderUtil.cpp:76` to `const std::string target = getTestSuiteName();` (note: drop the
`&`, it now returns by value). Leave `runTest()` using `getTarget()` so the `_options.mtlx`
`targets` gate keeps working unchanged.

In `RenderVk.cpp`: `std::string getTestSuiteName() const override { return "genglsl_vulkan"; }`
— consistent with `GenGlsl.cpp:180`'s existing `"glsl_vulkan"` convention.

Image files do **not** collide: `RenderUtil.cpp:583-593` gives both testers the same
per-material directory (which is what you want for side-by-side comparison) and filenames are
per-tester literals — `_glsl.png` at `RenderGlsl.cpp:364`, `_slang.png` at
`RenderSlang.cpp:354`. Use `_vk.png`. Dump generated code as `_vs.vk.glsl` / `_ps.vk.glsl`
(vs `_vs.glsl` / `_ps.glsl` at `RenderGlsl.cpp:234-237`).

### Headless skip

`ShaderRenderTester::loadOptions` calls `runTest(options)` (`RenderUtil.cpp:399`) **before**
`initializeGeneratorContext` → `createRenderer` (`:472`), so overriding `runTest()`
short-circuits before any `REQUIRE(initialized)` can fail:

```cpp
bool runTest(const GenShaderUtil::TestSuiteOptions& testOptions) override
{
    if (!RenderUtil::ShaderRenderTester::runTest(testOptions))
        return false;
    if (!mx::VkContext::isDeviceAvailable())
    {
        std::cerr << "No Vulkan device available. Skip Vulkan render test." << std::endl;
        return false;
    }
    return true;
}
```

This is cleaner than PR #2897's approach of returning null from inside `createRenderer` — but
keep `VkContext::tryCreate()` too, for applications.

**Verify:**

```powershell
cmake --build build --config Release --target MaterialXTest -j
ctest --test-dir build -C Release -R MaterialXTest_Render_Vulkan_TestSuite -V
```

The ctest name comes from `add_tests()` (`source/MaterialXTest/CMakeLists.txt:9-31`) grepping
for `TEST_CASE("Render: Vulkan TestSuite", "[rendervk]")` at configure time.

Then check the output directory for:

- `genglsl_vulkan_render_log.txt` **alongside an untouched** `genglsl_render_log.txt`.
- `*_vk.png` alongside `*_glsl.png`.
- Headless path: `VK_ICD_FILENAMES=/nonexistent ctest -R MaterialXTest_Render_Vulkan_TestSuite -V`
  must print the skip message and **pass**, not fail.

## Phase 6 — Viewer backend (3–5 days)

This is much smaller than the raw `#ifdef` count suggests. The 23
`MATERIALXVIEW_METAL_BACKEND` sites in `Viewer.cpp` are four separate concerns, and only one
is the render pipeline:

| Category | Sites | Vulkan action |
|---|---|---|
| **A. GL windowing** (`clear()`, `checkGlErrors`, `glDisable(GL_FRAMEBUFFER_SRGB)`) | `Viewer.cpp:2226, 2282, 2328` | **No change.** NanoGUI stays on OpenGL; `MATERIALXVIEW_OPENGL_BACKEND` stays defined; these `#ifndef METAL` blocks remain correct. |
| **B. Essl secondary generator** | `Viewer.cpp:505, 816, 828, 936, 1020, 1338, 1468, 1884, 1958, 2629`; `Viewer.h:424` | **No change.** Orthogonal to the render backend. |
| **C. Metal-only quirks** (`metal_texture()`, mouse-Y flip, `[1][1]` flip) | `Viewer.cpp:380, 2152, 2379, 2471` | **No change.** Vulkan needs none of these (see Background C). |
| **D. Genuinely backend-dependent** | `Viewer.cpp:9, 59, 202, 259, 2448`; `RenderPipeline.h:23`; `Editor.cpp:649` | **7 edits.** |

**So: do not touch the existing macro.** Add `MATERIALXVIEW_VULKAN_BACKEND` as an *additional*
define alongside `MATERIALXVIEW_OPENGL_BACKEND`. OpenGL keeps owning the window; Vulkan owns
only the 3D scene. Seven surgical edits, no refactor risk.

### The seven edits

1. **`RenderPipeline.h:23-27`** — three-way `TextureBakerPtr` alias:
   ```cpp
   #if defined(MATERIALXVIEW_METAL_BACKEND)
   using TextureBakerPtr = shared_ptr<class TextureBakerMsl>;
   #elif defined(MATERIALXVIEW_VULKAN_BACKEND)
   using TextureBakerPtr = shared_ptr<class TextureBakerVk>;
   #else
   using TextureBakerPtr = shared_ptr<class TextureBakerGlsl>;
   #endif
   ```
2. **`Viewer.cpp:9-17`** — add a block *after* the existing one (the GL includes are still
   needed for the window):
   ```cpp
   #ifdef MATERIALXVIEW_VULKAN_BACKEND
   #include <MaterialXView/RenderPipelineVulkan.h>
   #include <MaterialXGenGlsl/VkShaderGenerator.h>
   #endif
   ```
3. **`Viewer.cpp:59`** — leave `USE_FLOAT_BUFFER` false for Vulkan in pass 1 (matches GL).
4. **`Viewer.cpp:202-210`** — generator selection, nesting inside the existing `#ifndef METAL`:
   ```cpp
   #ifndef MATERIALXVIEW_METAL_BACKEND
     #ifdef MATERIALXVIEW_VULKAN_BACKEND
       _genContext(mx::VkShaderGenerator::create(_typeSystem)),
     #else
       _genContext(mx::GlslShaderGenerator::create(_typeSystem)),
     #endif
       _genContextEssl(mx::EsslShaderGenerator::create(_typeSystem)),
   #else
       _genContext(mx::MslShaderGenerator::create(_typeSystem)),
   #endif
   ```
5. **`Viewer.cpp:259-264`** — pipeline construction, same nesting; Vulkan calls
   `VulkanRenderPipeline::create(this)` then `initialize(nullptr, nullptr)`. Keep the Essl
   option lines that follow.
6. **`Viewer.cpp:2448`** — projection convention becomes
   `#if defined(MATERIALXVIEW_METAL_BACKEND) || defined(MATERIALXVIEW_VULKAN_BACKEND)` →
   `createPerspectiveMatrixZP` / `createOrthographicMatrixZP`. Leave `Viewer.cpp:2471`
   (the `[1][1]` flip and `metal_texture()`) **untouched**.
7. **`Editor.cpp:649`** — comment only, documenting that `VkMaterial::bindShader()` must be
   safe to call outside a render pass.

`Viewer.cpp:1651`'s `getTarget() == mx::GlslShaderGenerator::TARGET` is **already true** under
Vulkan (both are `"genglsl"`), so shader-source export works unchanged. No edit.

### `source/MaterialXView/CMakeLists.txt`

Replace the implicit Apple-only selection with an explicit tri-state, keeping
`NANOGUI_PREFERRED_BACKEND` on `OpenGL` for Vulkan:

```cmake
set(MATERIALXVIEW_BACKEND "Auto" CACHE STRING "Viewer render backend: Auto, OpenGL, Metal, or Vulkan.")
set_property(CACHE MATERIALXVIEW_BACKEND PROPERTY STRINGS Auto OpenGL Metal Vulkan)
...
if(MATERIALXVIEW_BACKEND STREQUAL "Vulkan")
    if(NOT MATERIALX_BUILD_RENDER_VK)
        message(FATAL_ERROR "MATERIALXVIEW_BACKEND=Vulkan requires MATERIALX_BUILD_RENDER_VK=ON.")
    endif()
    # NanoGUI keeps its OpenGL backend: the window and UI are GL; only the 3D
    # scene is rendered offscreen with Vulkan and blitted in.
    set(NANOGUI_PREFERRED_BACKEND OpenGL)
    set(MATERIALXVIEW_RENDER_BACKEND_DEFINITIONS
        "-DMATERIALXVIEW_OPENGL_BACKEND=1;-DMATERIALXVIEW_VULKAN_BACKEND=1")
    LIST(APPEND MATERIALX_LIBRARIES MaterialXRenderVk)
    LIST(REMOVE_ITEM materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/RenderPipelineGL.cpp")
endif()
```

Keep `MaterialXRenderGlsl` in the library list — you still need `GLUtil.h`/`checkGlErrors` for
the category-A sites and for the blit. `RenderPipelineVulkan.cpp` is picked up automatically by
the existing `file(GLOB ...)`; `RenderPipelineGL.cpp` must be explicitly removed so there is
only one `RenderPipeline` implementation (exactly what the Metal branch does today at
`CMakeLists.txt:113-114`).

### The Vulkan → GL transfer

`VulkanRenderPipeline` implements all 12 virtuals of `RenderPipeline.h:48-69`. The critical
one, `renderFrame()`, ignores its `color_texture` argument (as `RenderPipelineGL.cpp:308`
already does), records the env / opaque / transparent / wireframe passes structurally
identically to `RenderPipelineGL::renderFrame` (`RenderPipelineGL.cpp:308-478`) with GL state
calls replaced by per-pass pipeline variants, submits, waits, then transfers:

```
vkCmdCopyImageToBuffer(colorImage -> host-visible staging VkBuffer)
vkQueueWaitIdle
glBindTexture(GL_TEXTURE_2D, _blitTexture)
glTexSubImage2D(...)              // reuse the texture; never reallocate per frame
draw a fullscreen quad with a trivial GL blit program
```

**Keep GL interop out of scope.** `VK_KHR_external_memory_win32` / `_fd` +
`GL_EXT_memory_object` requires matching device UUIDs between the GL driver's GPU and the
chosen `VkPhysicalDevice` — which **fails outright on hybrid-graphics laptops**, a large
fraction of the user base — plus a `VkSemaphore`↔`GL_EXT_semaphore` handshake, dedicated
allocations, separate HANDLE/fd paths, and tiling negotiation. Several hundred lines of
platform-forked code to save a few milliseconds in a tool that is not frame-rate-critical.
Isolate the transfer behind a single `presentToGL()` virtual so the fast path can be dropped
in later behind a runtime capability check.

Cost estimate: ~8 MB read + 8 MB upload per frame at 1080p RGBA8, roughly 3–5 ms round trip on
a discrete GPU. Comparable to what `GLRenderPipeline::getFrameImage()` already does on capture.

`getFrameImage()` becomes trivial — return `_framebuffer->getColorImage()` directly, skipping
GL entirely. That is actually *better* than the GL pipeline's `glReadPixels` path.

**Verify:**

```powershell
cmake -S . -B build -DMATERIALX_BUILD_VIEWER=ON -DMATERIALX_BUILD_RENDER_VK=ON -DMATERIALXVIEW_BACKEND=Vulkan
cmake --build build --config Release --target MaterialXView -j
./build/bin/Release/MaterialXView.exe --material resources/Materials/Examples/StandardSurface/standard_surface_carpaint.mtlx
```

Gate: matches the GL viewer visually; orbit/zoom respond; "Save Image" writes a correct PNG;
UI texture baking matches the GL viewer's output.

## Phase 7 — Packaging, CI, docs (1–2 days)

- `cmake/modules/MaterialXConfig.cmake.in` — a `RenderVk` component block (below).
- `documents/CMakeLists.txt:18` — add the module to `MATERIALX_DOXYGEN_SOURCE_FOLDERS`.
- `.github/workflows/main.yml` — a **build-only** Vulkan job on Windows and Linux.
- `CHANGELOG.md` and a note in `documents/DeveloperGuide/` about the SDK requirement.

**Python bindings: defer to a follow-up PR.** `MaterialXRenderSlang` — the most recent backend
and the closest analogue in maturity — ships none, and `source/PyMaterialX/` has only
`PyMaterialXRenderGlsl`, `PyMaterialXRenderMsl`, `PyMaterialXRenderOsl`. Adding
`PyMaterialXRenderVk` here means ~5 files plus a `source/PyMaterialX/CMakeLists.txt:60-67, 90-91`
edit purely to expose `TextureBakerVk`, with no consumer yet. Ship it separately once the
renderer is proven. (Noting the divergence: PR #2897 *did* ship bindings for HLSL.)

---

# Class design

Each class mirrors a specific existing counterpart. Where the Vulkan shape must diverge, it is
called out.

## `VkContext` — mirrors `SlangContext`, **not** `MetalState`

Decision: **per-instance, `shared_ptr`-owned, no global singleton.** `MetalState` is global
because `MTLDevice` is process-wide and NanoGUI hands it to you. Vulkan has no such
constraint, and a global would make `TextureBakerVk` (which constructs a second renderer) and
the viewer fight over one device.

```cpp
using VkContextPtr = std::shared_ptr<class VkContext>;

/// @class VkContext
/// Owns the Vulkan instance, physical device, logical device, queue and command
/// pool used by a renderer. Headless: no surface, no swapchain.
class MX_RENDERVK_API VkContext
{
  public:
    /// Create a context, throwing ExceptionRenderError on failure.
    static VkContextPtr create(bool enableValidation = MX_VK_DEFAULT_VALIDATION);

    /// Non-throwing probe for applications that must degrade gracefully.
    static VkContextPtr tryCreate(std::ostream* log = nullptr) noexcept;

    /// Cheap cached query: does this machine expose a Vulkan device?
    /// Safe to call before create(). Used by the render tests to skip on headless CI.
    static bool isDeviceAvailable();

    /// Adopt an externally owned device, e.g. from a host application passing a
    /// RenderContextHandle to VkRenderer::initialize().
    static VkContextPtr createShared(vk::Instance, vk::PhysicalDevice, vk::Device,
                                     uint32_t queueFamilyIndex, vk::Queue);
    ~VkContext();

    vk::Instance       getInstance()         const { return _instance; }
    vk::PhysicalDevice getPhysicalDevice()   const { return _physicalDevice; }
    vk::Device         getDevice()           const { return _device; }
    vk::Queue          getQueue()            const { return _queue; }
    uint32_t           getQueueFamilyIndex() const { return _queueFamilyIndex; }
    vk::CommandPool    getCommandPool()      const { return _commandPool; }

    /// Allocate, begin, run, submit and wait on a single-use command buffer.
    void submitOneTimeCommands(const std::function<void(vk::CommandBuffer)>& recorder);
    void waitIdle();

    uint32_t findMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags props) const;
    vk::Format findSupportedDepthFormat() const;

  protected:
    VkContext(bool enableValidation);

    vk::Instance _instance;
    vk::DebugUtilsMessengerEXT _debugMessenger;
    vk::PhysicalDevice _physicalDevice;
    vk::Device _device;
    vk::Queue _queue;
    uint32_t _queueFamilyIndex = 0;
    vk::CommandPool _commandPool;
    bool _ownsDevice = true;
};
```

Implementation notes:

- Use `vulkan.hpp` with `VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1`. Put
  `VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE` in **exactly one** `.cpp`
  (`VkContext.cpp`) and initialize `VULKAN_HPP_DEFAULT_DISPATCHER` once in `create()`.
- **Everything inside `MATERIALX_NAMESPACE`.** The branch's `VulkanDevice` is declared outside
  it; don't repeat that.
- **No `glm` anywhere.** The branch exposes `glm::mat4x4 lookAt/perspective/…` from
  `VkDevice.h`; delete all of it — `mx::Camera` already provides these.
- `isDeviceAvailable()` is a cached static: try `vk::detail::DynamicLoader`, create a minimal
  instance with no extensions or layers, `vkEnumeratePhysicalDevices`, require count > 0,
  catch everything and return false.

## `VkFramebuffer` — mirrors `GLFramebuffer`

Note the branch spells it `VkFrameBuffer` (capital B). Use **`VkFramebuffer`**, matching
`GLFramebuffer` / `MetalFramebuffer`.

```cpp
using VkFramebufferPtr = std::shared_ptr<class VkFramebuffer>;

class MX_RENDERVK_API VkFramebuffer
{
  public:
    static VkFramebufferPtr create(VkContextPtr context, unsigned int width, unsigned int height,
                                   unsigned int channelCount, Image::BaseType baseType);
    virtual ~VkFramebuffer();

    unsigned int getWidth()  const { return _width; }
    unsigned int getHeight() const { return _height; }

    void setEncodeSrgb(bool encode);   // recreates render pass/attachment if the format changes
    bool getEncodeSrgb() const { return _encodeSrgb; }
    void resize(unsigned int width, unsigned int height);

    /// Begin/end the render pass. Unlike GLFramebuffer::bind(), Vulkan needs the
    /// command buffer, so the signature necessarily differs -- same as
    /// MetalFramebuffer::bind(MTLRenderPassDescriptor*).
    void bind(vk::CommandBuffer cmd, const Color3& clearColor);
    void unbind(vk::CommandBuffer cmd);

    vk::RenderPass  getRenderPass()      const { return _renderPass; }
    vk::Framebuffer getFramebuffer()     const { return _framebuffer; }
    vk::Format      getColorFormat()     const { return _colorFormat; }
    vk::Image       getColorImage()      const { return _colorImage; }
    vk::ImageView   getColorImageView()  const { return _colorView; }

    /// Read the color attachment back into an mx::Image via a host-visible staging
    /// buffer. Row order matches GLFramebuffer::getColorImage() -- see Background C.
    ImagePtr getColorImage(ImagePtr image = nullptr);

  protected:
    VkFramebuffer(VkContextPtr, unsigned, unsigned, unsigned, Image::BaseType);
    void createAttachments();
    void destroyAttachments();

    VkContextPtr _context;
    unsigned int _width, _height, _channelCount;
    Image::BaseType _baseType;
    bool _encodeSrgb = false;
    vk::Format _colorFormat, _depthFormat;
    vk::Image _colorImage, _depthImage;
    vk::DeviceMemory _colorMemory, _depthMemory;
    vk::ImageView _colorView, _depthView;
    vk::RenderPass _renderPass;
    vk::Framebuffer _framebuffer;
};
```

Create the color attachment with `eColorAttachment | eTransferSrc | eSampled` — transfer-src
for readback and the viewer blit, sampled for a future interop path.

## `VkProgram` — mirrors `GlslProgram`

Keep the `GlslProgram` public surface verbatim. `Input` gains Vulkan fields and drops `gltype`.

```cpp
class MX_RENDERVK_API VkProgram
{
  public:
    static VkProgramPtr create(VkContextPtr context);
    virtual ~VkProgram();

    // --- Shader code setup (identical to GlslProgram) ---
    void setStages(ShaderPtr shader);
    void addStage(const string& stage, const string& sourceCode);
    const string& getStageSourceCode(const string& stage) const;
    void clearStages();
    ShaderPtr getShader() const { return _shader; }

    // --- Program building ---
    /// glslang compile of both stages -> single TProgram -> link -> buildReflection ->
    /// GlslangToSpv -> 2x vk::ShaderModule -> descriptor set layout -> pipeline layout ->
    /// UBOs -> bindUniformDefaults(). Throws ExceptionRenderError with the glslang log.
    void build(const VkFramebufferPtr& framebuffer);
    bool hasBuiltData();
    void clearBuiltData();

    struct MX_RENDERVK_API Input
    {
        static int INVALID_VK_TYPE;
        int   location   = -1;   // vertex attribute location, or -1
        int   binding    = -1;   // descriptor binding for samplers/blocks, or -1
        int   blockIndex = -1;   // index into _blocks for UBO members, or -1
        int   offset     = -1;   // std140 byte offset within the block
        int   size       = 0;    // element count (matches GlslProgram::Input::size)
        bool  isSampler  = false;
        bool  isConstant = false;
        string typeString;                 // from the Shader, if present
        MaterialX::ConstValuePtr value;    // from the Shader, if present
        string path, unit, colorspace;     // from the Shader, if present
        Input(int loc, int bind, int sz, const string& p);
    };
    using InputPtr = std::shared_ptr<Input>;
    using InputMap = std::unordered_map<string, InputPtr>;

    const InputMap& getUniformsList();     // same contract as GlslProgram
    const InputMap& getAttributesList();   // same contract as GlslProgram
    void findInputs(const string& variable, const InputMap& variableList,
                    InputMap& foundList, bool exactMatch);

    bool bind(vk::CommandBuffer cmd);   // binds pipeline + descriptor set; flushes dirty UBOs
    void unbind() const;
    bool hasActiveAttributes() const;
    bool hasUniform(const string& name);
    void bindUniform(const string& name, ConstValuePtr value, bool errorIfMissing = true);

    void bindMesh(MeshPtr mesh);
    void bindPartition(MeshPartitionPtr partition);
    void drawPartition(vk::CommandBuffer cmd, MeshPartitionPtr partition);
    void unbindGeometry();

    void bindTextures(ImageHandlerPtr imageHandler);
    void bindLighting(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler);
    void bindViewInformation(CameraPtr camera);
    void bindTimeAndFrame(float time = 0.0f, float frame = 1.0f);

    bool isTransparent() const { return _alphaBlendingEnabled; }
    void printUniforms(std::ostream& outputStream);
    void printAttributes(std::ostream& outputStream);

  protected:
    VkProgram(VkContextPtr context);
    const InputMap& updateUniformsList();
    const InputMap& updateAttributesList();
    ConstValuePtr findUniformValue(const string& name, const InputMap& list);

    /// Seed every UBO shadow buffer from the Shader's VariableBlocks.
    /// MANDATORY under Vulkan: device memory is uninitialized, unlike D3D11
    /// constant buffers (zeroed) or GL default uniform values.
    void bindUniformDefaults();
    void flushUniforms();

    /// Fill any sampler binding not yet written with ImageHandler::getZeroImage().
    /// Sampling an unwritten Vulkan descriptor is undefined behaviour.
    void writeUnboundSamplersWithZeroImage(ImageHandlerPtr imageHandler);

    bool compileToSpirv(StringVec& errors);
    void createDescriptorLayout();
    void createPipeline(const VkFramebufferPtr& framebuffer);

    struct UniformBlock
    {
        string name;
        uint32_t binding = 0;
        size_t   size    = 0;
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        void* mapped = nullptr;
        std::vector<uint8_t> shadow;   // zero-initialized; seeded by bindUniformDefaults()
        bool dirty = true;
    };

    struct VertexBinding   // one interleaved buffer per mesh
    {
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        std::vector<string> attributeOrder;
        uint32_t stride = 0;
    };

    VkContextPtr _context;
    StringMap _stages;
    ShaderPtr _shader;
    std::map<string, std::vector<uint32_t>> _spirv;
    vk::ShaderModule _vertexModule, _fragmentModule;
    vk::DescriptorSetLayout _descriptorSetLayout;
    vk::PipelineLayout _pipelineLayout;
    vk::DescriptorPool _descriptorPool;
    vk::DescriptorSet _descriptorSet;
    vk::Pipeline _pipeline;
    InputMap _uniformList, _attributeList;
    std::vector<UniformBlock> _blocks;
    std::map<Mesh*, VertexBinding> _vertexBuffers;
    std::map<MeshPartitionPtr, std::pair<vk::Buffer, vk::DeviceMemory>> _indexBuffers;
    MeshPtr _boundMesh;
    bool _alphaBlendingEnabled = false;
};
```

**Vertex buffers differ from GL by necessity.** GL binds one buffer per attribute via
`glVertexAttribPointer`. Vulkan fixes strides at pipeline-creation time in
`VkPipelineVertexInputStateCreateInfo`, so build **one interleaved buffer per `Mesh*`** whose
layout is dictated by `getAttributesList()` locations. Cache on the raw `Mesh*` (matching
`_boundMesh` semantics in `GlslProgram`) and on `MeshPartitionPtr` for indices (matching
`_indexBufferIds`, `GlslProgram.h:259`).

**Pipeline caching.** One pipeline per program suffices for the render tests and the baker. For
the viewer, add the `PipelineKind`/format-keyed map that `SlangProgram` uses
(`SlangProgram.h:185-196, 235-244`) in Phase 6 — you need it for the transparent (alpha-blend,
cull-front-then-back) and wireframe passes that `RenderPipelineGL::renderFrame`
(`RenderPipelineGL.cpp:424-477`) gets from mutable GL state.

### The five binding routines

| Routine | Source of truth | Vulkan-specific hazard |
|---|---|---|
| `bindUniformDefaults()` | `_shader->getStage(*)->getUniformBlocks()` → `ShaderPort::getValue()` | **Trap #1.** Without it, materials render as garbage, not black. Call from `build()`. |
| `bindTextures()` | Iterate `getUniformsList()` for `isSampler`, skip `HW::ENV_RADIANCE`/`HW::ENV_IRRADIANCE`; `ImageSamplingProperties::setProperties(name, publicUniforms)`; `imageHandler->acquireImage(...)` | Mirrors `GlslProgram.cpp:528-564`, but writes a `vk::DescriptorImageInfo` instead of `glUniform1i(loc, unit)`. |
| `bindLighting()` env images | `lightHandler->getEnvRadianceMap()` / `getEnvPrefilteredMap()` / `getEnvIrradianceMap()`, else `imageHandler->getZeroImage()` | **Trap #3.** Follow with `writeUnboundSamplersWithZeroImage()`. |
| `bindLighting()` scalars | `HW::ENV_MATRIX`, `ENV_RADIANCE_SAMPLES`, `ENV_LIGHT_INTENSITY`, `ENV_RADIANCE_MIPS`, `REFRACTION_TWO_SIDED`, `NUM_ACTIVE_LIGHT_SOURCES`, `ALBEDO_TABLE` | Straight port of `GlslProgram.cpp:581-630, 684-701`. |
| `bindLighting()` light sources | `HW::LIGHT_DATA_INSTANCE + "[i]." + inputName`; direction transformed by `lightHandler->getLightTransform()` | Port of `GlslProgram.cpp:637-681`. **Depends on Phase 0 Q1** — see Risk R2. |

## `VkTextureHandler` — mirrors `GLTextureHandler`

```cpp
class MX_RENDERVK_API VkTextureHandler : public ImageHandler
{
    friend class VkProgram;
  public:
    static ImageHandlerPtr create(VkContextPtr context, ImageLoaderPtr imageLoader);
    ~VkTextureHandler();

    bool bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties) override;
    bool unbindImage(ImagePtr image) override;
    bool createRenderResources(ImagePtr image, bool generateMipMaps,
                               bool useAsRenderTarget = false) override;
    void releaseRenderResources(ImagePtr image = nullptr) override;

    vk::DescriptorImageInfo getDescriptorInfo(ImagePtr image, const ImageSamplingProperties&);
    vk::Sampler getSampler(const ImageSamplingProperties& samplingProperties);

    static vk::SamplerAddressMode mapAddressModeToVk(ImageSamplingProperties::AddressMode);
    static vk::Filter             mapFilterTypeToVk(ImageSamplingProperties::FilterType, bool mip);
    static vk::SamplerMipmapMode  mapMipModeToVk(ImageSamplingProperties::FilterType);
    static vk::Format             mapTextureFormatToVk(Image::BaseType, unsigned channelCount, bool srgb);

  protected:
    VkTextureHandler(VkContextPtr, ImageLoaderPtr);
    struct VkTextureResource { vk::Image image; vk::DeviceMemory memory;
                               vk::ImageView view; uint32_t mipLevels; vk::Format format; };
    void generateMipmaps(vk::CommandBuffer, VkTextureResource&, unsigned w, unsigned h);

    VkContextPtr _context;
    std::map<unsigned int, VkTextureResource> _textures;   // keyed by Image::getResourceId()
    std::unordered_map<ImageSamplingProperties, vk::Sampler, ImageSamplingKeyHasher> _samplers;
};
```

- Reuse `ImageSamplingKeyHasher` from `MaterialXRender/ImageHandler.h` — `SlangTextureHandler.h:91`
  already does.
- `bindImage()` has no "texture unit" to occupy in Vulkan; it just ensures the resource exists
  and records sampling properties. **Do not add `getBoundTextureLocation()`** — it has no
  Vulkan analogue. (`RenderPipelineGL.cpp:169, 281` use it; the Vulkan pipeline uses
  `getDescriptorInfo()`.)
- **Expand 3-channel images to RGBA on upload.** `VK_FORMAT_R8G8B8_UNORM` and
  `R32G32B32_SFLOAT` are optional and usually unsupported for sampled images on desktop
  drivers. This is a real GL divergence and a likely cause of mysteriously black textures.

## `VkRenderer` — mirrors `GlslRenderer` and `SlangRenderer`

```cpp
class MX_RENDERVK_API VkRenderer : public ShaderRenderer
{
  public:
    static VkRendererPtr create(unsigned int width = 512, unsigned int height = 512,
                                Image::BaseType baseType = Image::BaseType::UINT8);
    virtual ~VkRenderer();

    /// Create a texture handler for Vulkan textures. Requires initialize() first.
    ImageHandlerPtr createImageHandler(ImageLoaderPtr imageLoader);

    /// @param renderContextHandle optional VkContext* for sharing an existing device.
    void initialize(RenderContextHandle renderContextHandle = nullptr) override;
    void createProgram(ShaderPtr shader) override;
    void createProgram(const StageMap& stages) override;
    void validateInputs() override;
    void updateUniform(const string& name, ConstValuePtr value) override;
    void setSize(unsigned int width, unsigned int height) override;
    void render() override;
    void renderTextureSpace(const Vector2& uvMin, const Vector2& uvMax);
    ImagePtr captureImage(ImagePtr image = nullptr) override;

    VkProgramPtr     getProgram()     const { return _program; }
    VkFramebufferPtr getFramebuffer() const { return _framebuffer; }
    VkContextPtr     getContext()     const { return _context; }
    void   setScreenColor(const Color3& c) { _screenColor = c; }
    Color3 getScreenColor() const { return _screenColor; }

  protected:
    VkRenderer(unsigned int width, unsigned int height, Image::BaseType baseType);
    void createFrameBuffer(bool encodeSrgb);

  private:
    bool _initialized = false;
    VkContextPtr _context;
    VkProgramPtr _program;
    VkFramebufferPtr _framebuffer;
    Color3 _screenColor;
};
```

Constructor delegates to
`ShaderRenderer(width, height, baseType, MatrixConvention::Vulkan)`, matching
`GlslRenderer.cpp:28`.

Two deliberate divergences from `GlslRenderer`:

- `createImageHandler()` is **not** inline (unlike `GlslRenderer.h:49-52`) because it needs
  `_context`, so `initialize()` must run first. `TextureBaker.inl:90-93` already calls them in
  that order, so this is safe.
- **No `SimpleWindowPtr _window`.** `GlslRenderer.h:139` keeps one for GL context creation;
  Vulkan offscreen needs none.

## `VkMaterial` — mirrors `GlslMaterial`

Identical `ShaderMaterial` override set; only the program type and `copyShader` change.

The one genuine divergence: `bindShader() const` (`ShaderMaterial.h:97`) takes no command
buffer, and a Vulkan pipeline cannot be bound without one. Resolve by making
`VkMaterial::bindShader()` a **validation / lazy-build** step (ensure
`_vkProgram->hasBuiltData()`, build if not) and having `VulkanRenderPipeline` set the current
command buffer on the material before the pass. `MslMaterial` solves this the same way via the
`MetalState` command-buffer stack.

## `TextureBakerVk` — mirrors `TextureBakerGlsl`

```cpp
using TextureBakerVkPtr = shared_ptr<class TextureBakerVk>;

class MX_RENDERVK_API TextureBakerVk : public TextureBaker<VkRenderer, VkShaderGenerator>
{
  public:
    static TextureBakerVkPtr create(unsigned int width = 1024, unsigned int height = 1024,
                                    Image::BaseType baseType = Image::BaseType::UINT8)
    {
        return TextureBakerVkPtr(new TextureBakerVk(width, height, baseType));
    }
    TextureBakerVk(unsigned int width, unsigned int height, Image::BaseType baseType);
};
```

The `.cpp` is ~12 lines delegating to
`TextureBaker<VkRenderer, VkShaderGenerator>(w, h, baseType, true)`.

---

# CMake edits

## Root `CMakeLists.txt`

**After line 47** (`MATERIALX_BUILD_RENDER_PLATFORMS`):

```cmake
# Vulkan render back-end. Auto-enabled when a Vulkan SDK is found.
# Windows and Linux only; MoltenVK is not supported.
set(__build_render_vk OFF)
if(MATERIALX_BUILD_RENDER AND MATERIALX_BUILD_RENDER_PLATFORMS AND
   MATERIALX_BUILD_GEN_GLSL AND (WIN32 OR (UNIX AND NOT APPLE)))
    find_package(Vulkan QUIET COMPONENTS glslang)
    if(Vulkan_FOUND AND TARGET Vulkan::glslang)
        set(__build_render_vk ON)
        message(STATUS "Vulkan found at ${Vulkan_LIBRARY}; enabling MATERIALX_BUILD_RENDER_VK.")
    elseif(Vulkan_FOUND)
        message(STATUS "Vulkan found but the glslang component is missing; "
                       "MATERIALX_BUILD_RENDER_VK is off. Install the LunarG SDK to enable it.")
    endif()
endif()
option(MATERIALX_BUILD_RENDER_VK "Build the native Vulkan render back-end." ${__build_render_vk})
```

`find_package` is called at the root rather than in the module because the option's default
must be known before the option is declared.

**After line 63:**
```cmake
option(MATERIALX_TEST_RENDER_VK "Include Vulkan in the render test suite." ON)
```

**Near lines 184/198:** `mark_as_advanced` for both new options.

**In the render dispatch block, after line 581** (inside `if(MATERIALX_BUILD_RENDER_PLATFORMS)`):
```cmake
        if(MATERIALX_BUILD_RENDER_VK)
            add_subdirectory(source/MaterialXRenderVk)
        endif()
```
Do **not** set `MATERIALX_BUILD_RENDER_HW ON` here.

## `source/MaterialXRenderVk/CMakeLists.txt` (new)

```cmake
file(GLOB materialx_source  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

find_package(Vulkan REQUIRED COMPONENTS glslang)

mx_add_library(MaterialXRenderVk
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXRender
        MaterialXGenGlsl
    EXPORT_DEFINE
        MATERIALX_RENDERVK_EXPORTS)

target_link_libraries(${TARGET_NAME}
    PUBLIC  Vulkan::Vulkan
    PRIVATE Vulkan::glslang)

target_compile_definitions(${TARGET_NAME}
    PUBLIC  VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    PRIVATE $<$<BOOL:${WIN32}>:NOMINMAX>
            $<$<BOOL:${WIN32}>:WIN32_LEAN_AND_MEAN>)
```

Portability decisions vs the branch, and why:

- `Vulkan::glslang` is an imported interface target from CMake's `FindVulkan` that already
  propagates `SPIRV`, `SPIRV-Tools`, `SPIRV-Tools-opt`, `MachineIndependent`, `OSDependent`,
  `GenericCodeGen` and `glslang-default-resource-limits`. **Do not** replicate the branch's
  `debug ${Vulkan_LIB_PATH}/SPIRV-Tools-optd.lib / optimized ...opt.lib` hack — that is
  MSVC + LunarG-layout specific and will not link on Linux.
- No `find_package(OpenGL)`, no `find_package(X11)`, no `Opengl32`, no
  `-DVK_USE_PLATFORM_WIN32_KHR`. The branch has all four and needs none.
- `VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1` must be `PUBLIC` because headers are installed and
  consumers (`MaterialXView`, `MaterialXTest`) include them.
- No explicit `install()` / `set_target_properties()` / `target_include_directories()` —
  `mx_add_library` (`CMakeLists.txt:397-506`) handles all of it, including monolithic aliasing
  and the `mxHeaders` FILE_SET.
- Do **not** pass `SKIP_INSTALL`. Slang does because slang-rhi isn't installable; Vulkan is a
  system dependency and this module should install normally.

## `source/MaterialXTest/CMakeLists.txt` — after line 82

```cmake
  if(MATERIALX_BUILD_RENDER_VK AND MATERIALX_TEST_RENDER_VK)
    add_subdirectory(MaterialXRenderVk)
    target_link_libraries(MaterialXTest MaterialXRenderVk)
  endif()
```

## `cmake/modules/MaterialXConfig.cmake.in`

Insert before `check_required_components(...)` and — importantly — **outside** the existing
`if(UNIX AND NOT APPLE)` block that handles `RenderGlsl` at line 44, since Vulkan is
Windows + Linux:

```cmake
if(@MATERIALX_BUILD_RENDER_VK@ AND "RenderVk" IN_LIST MaterialX_FIND_COMPONENTS)
    find_dependency(Vulkan)
    set(MaterialX_RenderVk_FOUND TRUE)
endif()
```

## `documents/CMakeLists.txt:18`

```cmake
    ${PROJECT_SOURCE_DIR}/source/MaterialXRenderOsl
    ${PROJECT_SOURCE_DIR}/source/MaterialXRenderVk)
```

## `.github/workflows/main.yml`

The existing Vulkan touchpoints (lines 277, 288-293) are **shader-generation validation only**
and need no change.

Add a **build-only** job on Windows and Linux: install the SDK
(`humbletim/setup-vulkan-sdk@v1.2.0`, or `apt install vulkan-sdk`), configure with
`-DMATERIALX_BUILD_RENDER_VK=ON`, build the `MaterialXRenderVk` target. Cheap, catches compile
breaks.

**Do not run the Vulkan render tests in CI initially.** GitHub-hosted runners have no GPU. A
software ICD (lavapipe / SwiftShader) is possible on Linux via `VK_ICD_FILENAMES`, but it is
slow across hundreds of materials and produces pixel differences that make golden comparison
useless. The `isDeviceAvailable()` skip means the tests simply skip cleanly if a job does run
them, which is the safe default. Optionally add one lavapipe smoke test rendering a *single*
material.

---

# `MatrixConvention::Vulkan`

`source/MaterialXRender/ShaderRenderer.h:34-38`:

```cpp
enum class MatrixConvention
{
    OpenGL = 0,
    Metal  = 1,
    Vulkan = 2
};
```

`source/MaterialXRender/ShaderRenderer.cpp:40` — extend the condition, don't add a branch:

```cpp
// Metal and Vulkan both use a [0,1] clip-space depth range.
if (_matrixConvention == ShaderRenderer::MatrixConvention::Metal ||
    _matrixConvention == ShaderRenderer::MatrixConvention::Vulkan)
{
    _camera->setProjectionMatrix(Camera::createPerspectiveMatrixZP(-fW, fW, -fH, fH, DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE));
}
else // MatrixConvention::OpenGL (default)
{
    _camera->setProjectionMatrix(Camera::createPerspectiveMatrix(-fW, fW, -fH, fH, DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE));
}
```

Per Background C, no Y handling is needed here or in `Camera`.

---

# File manifest

## New (24 files, ~6,400 lines)

`source/MaterialXRenderVk/`: `CMakeLists.txt` (55), `Export.h` (25), `VkUtil.{h,cpp}` (70/230),
`VkContext.{h,cpp}` (150/480), `VkFramebuffer.{h,cpp}` (115/380),
`VkTextureHandler.{h,cpp}` (130/560), `VkProgram.{h,cpp}` (330/1400),
`VkRenderer.{h,cpp}` (170/450), `VkMaterial.{h,cpp}` (135/480), `TextureBaker.{h,cpp}` (48/15).

`source/MaterialXTest/MaterialXRenderVk/`: `CMakeLists.txt` (8), `RenderVk.cpp` (420).

`source/MaterialXView/`: `RenderPipelineVulkan.{h,cpp}` (60/700).

For scale: `MaterialXRenderGlsl` is ~3,000 lines and `MaterialXRenderSlang` ~3,700.

## Modified (13 files)

| Path | Change |
|---|---|
| `CMakeLists.txt` | 2 options + auto-detect (after :47, :63), 2 `mark_as_advanced`, `add_subdirectory` (after :581) |
| `source/MaterialXRender/ShaderRenderer.h` | `MatrixConvention::Vulkan = 2` at :34-38 |
| `source/MaterialXRender/ShaderRenderer.cpp` | `:40` condition → `Metal \|\| Vulkan` |
| `source/MaterialXTest/MaterialXRender/RenderUtil.h` | + `virtual std::string getTestSuiteName() const` |
| `source/MaterialXTest/MaterialXRender/RenderUtil.cpp` | `:76` (and optionally the `:401` skip message) → `getTestSuiteName()` |
| `source/MaterialXTest/CMakeLists.txt` | + Vk dispatch block after :82 |
| `source/MaterialXView/CMakeLists.txt` | `MATERIALXVIEW_BACKEND` tri-state; Vulkan defines, libs, source list |
| `source/MaterialXView/RenderPipeline.h` | `:23-27` three-way `TextureBakerPtr` alias |
| `source/MaterialXView/Viewer.cpp` | 6 edits (:9, :59, :202, :259, :2448; verify :1651) |
| `source/MaterialXView/Editor.cpp` | Comment at :649 documenting the `bindShader()` contract |
| `documents/CMakeLists.txt` | +1 line at :18 |
| `cmake/modules/MaterialXConfig.cmake.in` | + `RenderVk` component block |
| `.github/workflows/main.yml` | + build-only Vk job (Windows + Linux) |

---

# Reuse — do not reinvent

- **`mx_add_library()`** (root `CMakeLists.txt:397`) — do not hand-roll `add_library` +
  `install` the way the branch did.
- **`TextureBaker<Renderer, ShaderGen>`** (`source/MaterialXRender/TextureBaker.h:31` + `.inl`).
- **`Camera::createPerspectiveMatrixZP` / `createOrthographicMatrixZP`**
  (`source/MaterialXRender/Camera.h:168, 173`).
- **`ImageHandler::getZeroImage()`** (`ImageHandler.h:248`) and `acquireImage()` (`:194`).
- **`ImageSamplingKeyHasher`** (`ImageHandler.h`).
- **`ImageHandler`, `GeometryHandler`, `LightHandler`, `Camera`, `Image`, `Mesh`** — all
  API-agnostic, used unchanged.
- **`RenderUtil::ShaderRenderTester`** (`RenderUtil.h:211`).
- **`VkShaderGenerator` / `VkSyntax` / `VkResourceBindingContext`** — already current.
- **`glslang::GetDefaultResources()`** (`<glslang/Public/ResourceLimits.h>`) instead of the
  branch's hand-rolled table.
- From `origin/vulkan/vkRenderer_old`, via `git show <branch>:<path>` — **reference only**:
  `GLSLtoSPV` env setup in `VkProgram.cpp:83-90`, plus resource plumbing in `VkDevice.cpp`,
  `VkBuffer.cpp`, `VkTexture.cpp`, `VkUniformBuffer.cpp`, `VkFramebuffer.cpp`.
  Do **not** port `shaders.hpp` (verbatim NVIDIA sample code), the `ShowWindow` patch to
  `SimpleWindowWindows.cpp`, or `VkProgram.h`.

---

# Verification

```powershell
# Configure (Windows, Vulkan SDK present)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
      -DMATERIALX_BUILD_TESTS=ON -DMATERIALX_TEST_RENDER=ON `
      -DMATERIALX_BUILD_VIEWER=ON -DMATERIALXVIEW_BACKEND=Vulkan
# Confirm: "Vulkan found at ...; enabling MATERIALX_BUILD_RENDER_VK."

cmake --build build --config Release -j

# Vulkan render tests
ctest --test-dir build -C Release -R MaterialXTest_Render_Vulkan_TestSuite --output-on-failure

# Full render suite -- confirms no regression in the GLSL tester
ctest --test-dir build -C Release -R Render --output-on-failure

# Viewer
./build/bin/Release/MaterialXView.exe --material resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx
```

Also confirm:

- Configuring **without** the Vulkan SDK succeeds, `MATERIALX_BUILD_RENDER_VK` auto-OFF, no warnings.
- `-DMATERIALX_BUILD_MONOLITHIC=ON -DMATERIALX_BUILD_RENDER_VK=ON` configures and builds.
- `[rendervk]` tests **skip, not fail**, when no device is present
  (`VK_ICD_FILENAMES=/nonexistent`).
- Vulkan validation layers report **zero errors** across a full suite run.
- `genglsl_render_log.txt` and `genglsl_vulkan_render_log.txt` both exist and are distinct.
- `*_vk.png` images match `*_glsl.png` for the same documents, with correct Y orientation.
- Linux: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)`.

---

# Risks, ranked

**R1 — Descriptor completeness (highest; bites in Phase 3).** Vulkan requires *every* binding
in the layout to be written before `vkCmdBindDescriptorSets`; GL and D3D11 tolerate unbound
samplers. One unwritten `combined_image_sampler` is UB and faults on some drivers.
*Mitigation:* `writeUnboundSamplersWithZeroImage()`, plus zero validation errors as the Phase 3
gate.

**R2 — glslang reflection of `u_lightData[i].member` (high; resolved in Phase 0).** If
composed names aren't exposed, all direct lighting silently produces garbage. *Mitigation:*
Phase 0 Q1 answers this before code is written. Fallback: synthesize offsets from the
`LightData` `VariableBlock` using the sorted member order that
`VkResourceBindingContext.cpp:143-147` emits (largest alignment first) plus the trailing `padN`
floats — deterministic, but coupled to that function, so add a unit test asserting the computed
struct size equals the reflected block size / `MAX_LIGHT_SOURCES`.

**R3 — std140 `mat3` layout (high; easy to miss).** See trap #2. *Mitigation:* row-expanded
write plus the Phase 2 round-trip unit test.

**R4 — `MATERIALX_BUILD_MONOLITHIC` interaction (medium).** `mx_add_library` aliases into one
target under monolithic builds (`CMakeLists.txt:452-458`), so `PUBLIC Vulkan::Vulkan` attaches
Vulkan to the monolithic `MaterialX` target and every consumer links it transitively.
*Mitigation:* explicitly verify that configuration in Phase 1.

**R5 — sRGB parity with GLSL (medium).** `RenderPipelineGL::renderFrame` toggles
`GL_FRAMEBUFFER_SRGB` around passes (`RenderPipelineGL.cpp:333, 357, 458`). Vulkan encodes sRGB
via the attachment *format*, fixed at render-pass creation — you cannot toggle mid-pass.
*Mitigation:* `VkFramebuffer::setEncodeSrgb` recreates the render pass; for the viewer's mixed
passes, either split into two render passes or do the conversion in the blit shader. Decide in
Phase 6.

**R6 — 3-channel texture formats (medium).** `VK_FORMAT_R8G8B8_UNORM` / `R32G32B32_SFLOAT` are
optional and usually unsupported for sampled images. Silent black textures. *Mitigation:*
always expand to 4 channels on upload; assert with `vkGetPhysicalDeviceFormatProperties`.

**R7 — Viewer readback cost (low-medium).** ~3–5 ms/frame at 1080p. Acceptable; keep
`presentToGL()` virtual so interop can be dropped in later.

**R8 — CI has no GPU (low).** Vulkan render tests build but won't execute, so regressions are
caught locally only until a GPU runner exists. The same limitation already applies to the GLSL
render tests.

**R9 — Generator statefulness (low, latent).** `VkShaderGenerator::_resourceBindingCtx` is a
shared mutable counter; concurrent `generate()` on one instance corrupts bindings. Not a
regression, but note it in `RenderVk.cpp` next to any future parallelization of
`ShaderRenderTester`.

**R10 — `vulkan.hpp` in public headers (low).** Pulls a large header into anything including
`VkRenderer.h`. Keep `vk::` types out of the public surface where practical, as
`GlslRenderer.h` keeps GL types out of its own.

---

# Open questions

1. **Phase 0 Q1** (light-data reflection) — answer before committing to the light-binding
   design.
2. **Minimum Vulkan version.** The branch targets `EShTargetVulkan_1_2` / `EShTargetSpv_1_5`.
   Is 1.2 an acceptable floor, or must something run on 1.1?
3. **Viewer sRGB (R5).** Is a small pixel delta vs the GL viewer acceptable in pass 1, or must
   it be bit-comparable?
4. **CI.** Build-only, or do you also want a lavapipe smoke test on Linux?
