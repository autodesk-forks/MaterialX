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
├── run_legacy_benchmark.py      # Benchmark runner and coverage analyzer
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
| `--outputPath` | Directory to write rendered PNG images (created if missing) |
| `--logPath` | Render log file (default: `<outputPath>/render_log.txt`, else `./render_log.txt`) |
| `--radiancePath` | Path to radiance IBL `.hdr` file |
| `--irradiancePath` | Path to irradiance IBL `.hdr` file |
| `--geometryPath` | Path to geometry `.obj` file (default: `sphere.obj`) |
| `--size` | Render resolution in pixels, e.g. `512` (default: 512) |
| `--path` | Additional search path(s) for MaterialX resources |
| `--library` | Additional relative library folder(s) to load |

### Example: Single Fusion material

From the **root** of the `MaterialX-adsk-fork` repository:

```base
mkdir -p ./renders
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
- A render log (default: `render_log.txt` under `--outputPath` if set)

## Legacy Benchmark & Coverage Analysis

We provide a convenient wrapper script `run_legacy_benchmark.py` that handles directory creation, executes the rendering test suite, times it with high precision, parses the logs, and extracts/analyzes the element-level coverage.

### Run Legacy ADSK Benchmark

To run, time, and generate a sorted coverage manifest:
```bash
python contrib/utilities/scripts/rendertest/run_legacy_benchmark.py
```

This will:
- Auto-create `contrib/renders`
- Run the full suite under `contrib/adsk/resources/Materials`
- Save the overall wall-clock timing report to the terminal
- Generate a sorted element-level coverage manifest at `contrib/renders/legacy_coverage.txt`

### Compare manifests (e.g. legacy vs pytest)

```bash
python contrib/utilities/scripts/rendertest/run_legacy_benchmark.py --compare contrib/renders/pytest_coverage.txt
```
