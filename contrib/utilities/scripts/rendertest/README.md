# renderDocuments

Renders all MaterialX renderable elements found in a given file or folder using GLSL.
Output images are written as PNG files named after each renderable element.

## Attribution

This script is derived from [MaterialX_Learn](https://github.com/kwokcb/MaterialX_Learn)
by [Bernard Kwok](https://www.linkedin.com/in/bernard-cb-kwok/), an active contributor to
[MaterialX](https://github.com/AcademySoftwareFoundation/MaterialX) at the Academy Software Foundation.

## Requirements

- MaterialX built with Python bindings (`MATERIALX_BUILD_PYTHON=ON`)
- Render support enabled (`MATERIALX_BUILD_RENDER=ON`, `MATERIALX_BUILD_GEN_GLSL=ON`)

## Layout

```
rendertest/
├── renderDocuments.py           # Main script
├── extract_render_coverage.py   # Coverage manifest from render logs
├── mtlxutils/
│   ├── mxbase.py           # Version utilities
│   ├── mxrenderer.py       # GLSL renderer wrapper
│   └── mxshadergen.py      # Shader generation utilities
└── README.md
```

## Usage

```bash
python renderDocuments.py [options] <inputFileName>
```

`<inputFileName>` may be a single `.mtlx` file or a folder (all `.mtlx` files will be rendered).

### Arguments

| Argument | Description |
|---|---|
| `inputFileName` | Input `.mtlx` file or folder |
| `--outputPath` | Directory to write rendered PNG images (default: same directory as input file) |
| `--radiancePath` | Path to radiance IBL `.hdr` file |
| `--irradiancePath` | Path to irradiance IBL `.hdr` file |
| `--geometryPath` | Path to geometry `.obj` file (default: `sphere.obj`) |
| `--size` | Render resolution in pixels, e.g. `512` (default: 512) |
| `--path` | Additional search path(s) for MaterialX resources |
| `--library` | Additional relative library folder(s) to load |

### Example: Single Fusion material

From the **root** of the `MaterialX-adsk-fork` repository:

```base
mkdir ./renders
```

```bash
python contrib/utilities/scripts/rendertest/renderDocuments.py --radiancePath resources/Lights/san_giuseppe_bridge.hdr --irradiancePath resources/Lights/irradiance/san_giuseppe_bridge.hdr --geometryPath resources/Geometry/sphere.obj --outputPath ./renders --size 512  contrib/adsk/resources/Materials/Examples/Fusion/metal.mtlx
```

### Example: All Fusion materials

```bash
python contrib/utilities/scripts/rendertest/renderDocuments.py --radiancePath resources/Lights/san_giuseppe_bridge.hdr --irradiancePath resources/Lights/irradiance/san_giuseppe_bridge.hdr --geometryPath resources/Geometry/sphere.obj --outputPath ./renders --size 512  contrib/adsk/resources/Materials/Examples/Fusion/
```

### Example: All Revit materials

```bash
python contrib/utilities/scripts/rendertest/renderDocuments.py --radiancePath resources/Lights/san_giuseppe_bridge.hdr --irradiancePath resources/Lights/irradiance/san_giuseppe_bridge.hdr --geometryPath resources/Geometry/sphere.obj --outputPath ./renders --size 512  contrib/adsk/resources/Materials/Examples/Revit 
```

## Output

- One PNG per renderable element, named `<element_name>_genglsl.png`
- A `render_log.txt` written to the working directory

## Coverage analysis

After a render run, extract a sorted manifest of `file.mtlx:element` keys and outcomes:

```bash
python contrib/utilities/scripts/rendertest/extract_render_coverage.py render_log.txt \
  -o legacy_coverage.txt
```

Each line is `relative/path/file.mtlx:element_name<TAB>STATUS` where `STATUS` is `PASS`, `FAIL`, or `SKIP`.

Compare two manifests (for example legacy vs pytest):

```bash
python contrib/utilities/scripts/rendertest/extract_render_coverage.py legacy_coverage.txt \
  --manifest legacy_coverage.txt --compare pytest_coverage.txt
```

The script also accepts MaterialXTest C++ logs (`genglsl_render_log.txt`) with `--format materialxtest`
(elements are marked `RENDER` because those logs do not record pass/fail).
