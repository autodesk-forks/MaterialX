# rendertest Utilities

This directory contains utility modules for rendering MaterialX materials, which are used by our modern, parallelized test suite under `contrib/tests`.

## Attribution

These utility modules are derived from [MaterialX_Learn](https://github.com/kwokcb/MaterialX_Learn)
by [Bernard Kwok](https://www.linkedin.com/in/bernard-cb-kwok/), an active contributor to
[MaterialX](https://github.com/AcademySoftwareFoundation/MaterialX) at the Academy Software Foundation.

## Requirements

- MaterialX built with Python bindings (`MATERIALX_BUILD_PYTHON=ON`)
- Render support enabled (`MATERIALX_BUILD_RENDER=ON`, `MATERIALX_BUILD_GEN_GLSL=ON`)

## Layout

```
rendertest/
├── mtlxutils/
│   ├── mxbase.py           # Version utilities
│   ├── mxrenderer.py       # GLSL renderer wrapper
│   ├── mxshadergen.py      # Shader generation utilities
│   └── render_material.py  # Shared render logic
└── README.md
```
