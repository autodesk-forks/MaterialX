# MaterialX Contrib Test Framework

## Overview

This directory contains pytest-based tests for MaterialX contrib materials, 
built on top of the existing `rendertest` utilities from Bernard Kwok's 
[MaterialX_Learn](https://github.com/kwokcb/MaterialX_Learn).

## Goals

1. **Per-material test cases**: Each material is a separate pytest test case with 
   individual pass/fail reporting
2. **Parallel execution**: Tests run in parallel using pytest-xdist for faster CI
3. **Shared setup**: Expensive renderer initialization is session-scoped, amortized 
   across tests within each worker process
4. **Reusable logic**: Render logic is encapsulated as clean, modular helpers in `render_material.py`

## Directory Structure

```
contrib/
├── utilities/
│   └── scripts/
│       └── rendertest/              # Existing utilities (minimal changes)
│           ├── mtlxutils/
│           │   ├── mxrenderer.py    # GLSL renderer wrapper
│           │   ├── mxshadergen.py   # Shader generation
│           │   └── render_material.py  # Shared render logic
└── tests/                           # pytest tests
    ├── conftest.py                  # Fixtures
    ├── pyproject.toml               # pytest config
    ├── test_render_glsl.py          # Parametrized render tests
    └── DESIGN.md                    # This file
```

## Usage

### Running tests

From the `contrib/tests` directory:

```bash
# Run all tests
pytest

# Run with parallel workers
pytest -n auto

# Run specific test
pytest -k "metal"

# Verbose output
pytest -v
```

### Prerequisites

- MaterialX built with Python bindings (`MATERIALX_BUILD_PYTHON=ON`)
- Render support enabled (`MATERIALX_BUILD_RENDER=ON`, `MATERIALX_BUILD_GEN_GLSL=ON`)
- Python path configured to find MaterialX modules

## Architecture

### Fixtures (conftest.py)

| Fixture | Scope | Purpose |
|---------|-------|---------|
| `stdlib` | session | Loaded MaterialX standard library |
| `adsklib` | session | Loaded Autodesk library |
| `libraries` | session | Combined libraries |
| `glsl_renderer` | session | Initialized GLSL renderer |
| `search_path` | session | MaterialX file search path |

### Test Parametrization

Test files are discovered at collection time via `get_stdlib_files()` and `get_adsk_files()`:
- Fast collection: simply globs `.mtlx` file paths without loading or parsing documents.
- Granular subtests: each file-level test dynamically discovers and runs its renderable elements in parallel-safe `pytest-subtests` loops.

### Shared Render Logic

`rendertest/mtlxutils/render_material.py` provides:
- `render_material()`: Render a single material node or renderable output.
- `RenderResult`: Dataclass for render outcomes.

This is used by `test_render_glsl.py` (pytest tests) to perform unified rendering validation.

## Future Work

1. **Baseline images**: Generate and store baseline rendered images
2. **Image comparison**: Compare renders against baselines using FLIP or similar
3. **Shader comparison**: Compare generated GLSL against baselines
4. **CI integration**: GitHub Actions workflow with software GL rendering

## Related

- Original rendertest: `contrib/utilities/scripts/rendertest/README.md`
- MaterialX_Learn: https://github.com/kwokcb/MaterialX_Learn

