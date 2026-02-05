# Python Code Examples

This folder contains example Python scripts that generate, process, and validate material content using the MaterialX API.

## Scripts

| Script | Description |
|--------|-------------|
| `baketextures.py` | Bake procedural textures to image files |
| `comparenodedefs.py` | Compare node definitions between spec and library |
| `creatematerial.py` | Create MaterialX materials programmatically |
| `diff_test_results.py` | Compare test runs (traces and rendered images) |
| `generateshader.py` | Generate shader code from MaterialX documents |
| `genmdl.py` | Generate MDL from MaterialX |
| `mxdoc.py` | Document MaterialX files |
| `mxformat.py` | Format MaterialX XML files |
| `mxvalidate.py` | Validate MaterialX documents |
| `translateshader.py` | Translate shaders between formats |

## Test Result Comparison

The `diff_test_results.py` script compares baseline vs optimized MaterialX test runs,
analyzing both performance traces and rendered images.

### Features

- **Performance traces**: Loads Perfetto `.perfetto-trace` files (requires `MATERIALX_BUILD_TRACING=ON`),
  computes timing deltas, identifies improvements and regressions
- **Rendered images**: Pixel-wise RMSE comparison to detect visual differences
- **Charts**: Generates bar charts showing before/after timing comparison

### Installation

```bash
# Core dependencies
pip install perfetto pandas matplotlib

# For image comparison
pip install pillow numpy
```

### Usage

```bash
# Compare both traces and images (default)
python diff_test_results.py baseline/ optimized/

# Compare traces only
python diff_test_results.py baseline/ optimized/ --traces

# Compare images only
python diff_test_results.py baseline/ optimized/ --images

# Filter noise and generate chart
python diff_test_results.py baseline/ optimized/ --min-delta-ms 1 --chart chart.png

# Export trace results to CSV
python diff_test_results.py baseline/ optimized/ --csv results.csv
```

### Options

| Option | Description |
|--------|-------------|
| `--traces` | Compare performance traces only |
| `--images` | Compare rendered images only |
| `--track`, `-t` | Filter traces by track name (e.g., `GPU`, `ShaderGen`) |
| `--min-delta-ms` | Minimum absolute time difference to include (filters noise) |
| `--chart`, `-c` | Output path for chart image (requires matplotlib) |
| `--csv` | Export trace results to CSV file (requires pandas) |
| `--image-threshold` | RMSE threshold for image differences (default: 0.001) |
| `--show-opt OPT_NAME` | Highlight materials affected by optimization |

### Optimization Tracking

When using `--show-opt`, the script finds optimization pass events in the trace and
identifies affected materials from the parent shader generation slice. Affected materials
are marked with `*` in the table and `★` in chart labels (bold purple text).

The script also validates that the baseline trace does NOT have the optimization enabled.