# MaterialXView WebGL / WebGPU Backend Parity

MaterialXView ships two renderer backends as separate webpack bundles. Use the **Renderer** toggle (bottom-center) to switch between `index.html` (WebGL) and `index-webgpu.html` (WebGPU). Query parameters (`?file`, `?geom`) are preserved so you can compare the same material and geometry.

> **Status:** WebGPU reaches visual parity with WebGL for standard lit surfaces (direct lights + IBL,
> sRGB output, raw texture sampling). Custom MaterialX vertex displacements remain WebGL-only; see
> [Vertex transforms](#vertex-transforms).

## Building and running

The WebGPU backend generates WGSL in the browser via the MaterialX WASM module, so that module must
include the `WgslShaderGenerator` binding (built when `MATERIALX_BUILD_GEN_GLSL` is enabled).

1. Build the MaterialX JavaScript/WASM target with Emscripten. This produces
   `JsMaterialXGenShader.{js,wasm,data}` (the `.data` file bundles the data libraries).
2. Copy those three artifacts into `javascript/build/bin/` (the folder the viewer's webpack config
   copies from); the WebGL backend additionally uses `JsMaterialXCore.{js,wasm}` from there. These
   are build outputs and are intentionally **not** committed to the repository.
3. From `javascript/` run `npm ci` (workspace install), then from `javascript/MaterialXView`:

   ```bash
   npm run start   # webpack dev server; open /index.html (WebGL) or /index-webgpu.html (WebGPU)
   npm run build   # static bundle in dist/
   ```

## Shader generation

| Backend | MaterialX generator | Three.js material | Shader output |
|---------|---------------------|-------------------|---------------|
| WebGL | `EsslShaderGenerator` | `RawShaderMaterial` | GLSL ES 3.0 vertex + pixel |
| WebGPU | `WgslShaderGenerator` | `MeshBasicNodeMaterial` | WGSL pixel → TSL via `mxtsladapter` |

## Parity policy

### Lighting

- **WebGL** and **WebGPU** both register the full light rig from `Lights/san_giuseppe_bridge_split.mtlx`
  (`u_numActiveLightSources`, `u_lightData`) plus FIS image-based lighting.
- WebGPU binds `u_lightData` through TSL `uniformArray` (struct array `LightData`); the adapter packs
  `registerLights()` output into the fixed-size array the generator emits.

### Color space (output)

- **WebGL** sets `hwSrgbEncodeOutput = true`; `RawShaderMaterial` writes that display-referred sRGB
  directly to the framebuffer.
- **WebGPU** sets `hwSrgbEncodeOutput = false`; the WGSL returns linear surface color and
  `WebGPURenderer` applies `outputColorSpace = SRGBColorSpace` when presenting to the canvas.
- Applying `mx_srgb_encode()` on WebGPU as well would double-encode (washed-out, low-contrast output).

### Lighting (WebGPU node material)

- Shading uses **MaterialX lights only** (`registerLights()` → `u_lightData` / `u_numActiveLightSources`
  in generated WGSL). `createMxWgslMaterial` sets `material.lights = false` so Three.js does not add
  its own `BasicLightingModel` pass on top of the MaterialX `colorNode`.

### Color space (input textures)

**Policy: parity-with-WebGL-today (raw sampling).** MaterialXView registers no color-management
system override in the viewer, so the generated shader performs no sRGB→linear input transform on
8-bit material textures. To match the WebGL `RawShaderMaterial` reference:

- Material textures: `NoColorSpace` on the WebGPU TSL `texture()` nodes (`buildTextureMap`).
- HDR environment maps: `NoColorSpace` on radiance/irradiance (`buildEnvironment` / `mxtsladapter`).

Physically-correct CMS (native MaterialXView style: sRGB textures + in-shader linearization via
`mx_srgb_texture_to_lin_rec709`) would require changing **both** backends together and would alter
the current WebGL appearance.

### Vertex transforms

- **WebGL** uses MaterialX-generated vertex shaders with explicit matrix uniforms (displacements,
  custom projections).
- **WebGPU** uses Three.js TSL builtins (`positionWorld`, `normalWorld`, `cameraPosition`). The
  `WgslShaderGenerator` still emits a `vs_main` vertex stage, but it is not wired into Three.js
  today.

**Deferred integration (Phase 4b):** reach displacement parity by either (a) feeding generated WGSL
vertex logic into a custom TSL `positionNode` / vertex graph, or (b) porting supported displacement
node types into TSL vertex nodes. Option (a) preserves MaterialX codegen fidelity; option (b) is
narrower but stays entirely in the TSL layer.

### Tangents

- **WebGL** uses geometry tangent attributes when present (computed for indexed meshes in
  `updateScene()`).
- **WebGPU** sets `useGeometryTangent: true` when indexed geometry is used, binding TSL
  `tangentWorld` for anisotropic materials (`standard_surface_metal_brushed.mtlx`). Otherwise the
  adapter derives an orthonormal tangent from the normal (isotropic-safe).

## Recommended comparison materials

Use the same `?file` and `?geom` on both pages:

| Purpose | Material URL param |
|---------|-------------------|
| Calibration / exposure | `?file=Materials/Examples/StandardSurface/standard_surface_greysphere.mtlx` |
| Coat + specular + lighting | `?file=Materials/Examples/StandardSurface/standard_surface_default.mtlx` |
| Tiled textures | `?file=Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx` |
| Anisotropic / brushed metal | `?file=Materials/Examples/StandardSurface/standard_surface_metal_brushed.mtlx` |

Example side-by-side URLs:

```
index.html?file=Materials/Examples/StandardSurface/standard_surface_greysphere.mtlx
index-webgpu.html?file=Materials/Examples/StandardSurface/standard_surface_greysphere.mtlx
```

## WebGPU requirements

WebGPU requires a browser with `navigator.gpu` support. If unavailable, the WebGPU page shows a fallback banner with a link to the WebGL view, and the WebGPU toggle is disabled on the WebGL page.

## Regression tests

JavaScript (no GPU/WASM required; run in CI after the viewer build):

```bash
npm run verify:wgsl          # WGSL → TSL converter (parameter mapping, entry parsing)
npm run verify:wgsl-assembly # Full NodeMaterial assembly via real TSL
```

C++ (`MaterialXTest`, run by CTest in CI):

- `[wgslgen]` — generates WGSL for example materials and asserts structural invariants: no residual
  GLSL, exactly one entry function, balanced scopes, well-formed manifest, `mx_srgb_encode` when
  `hwSrgbEncodeOutput=true`, and `LightData` / `sampleLightSource` when directional lights are bound.
- `[genglsl][wgsl]` — unit tests for the `GlslToWgsl` rewriter.

Set `MATERIALX_WGSL_DUMP_DIR` when running `[wgslgen]` to dump the generated `*.wgsl` and manifests
for inspection.

**Recommended CI enhancement:** validate the dumped WGSL with a parser such as
[`naga`](https://github.com/gfx-rs/wgpu/tree/trunk/naga) to catch syntax errors that the structural
checks above cannot.
