# Core

- [MaterialXCore](MaterialXCore): Core library  
- [MaterialXFormat](MaterialXFormat): XML support
- [MaterialXTest](MaterialXTest) : Unit tests
- [PyMaterialX](PyMaterialX) : Python API support

# Shader Generation and Usage

## Supported APIs

-   C++
-   Python

## Supported Backends

-   Core shader generation API: [MaterialXGenShader](MaterialXGenShader) module
    -  Refer to document in module folder.
-   Shader Generation languages supported:
    -   GLSL
        -   Version 4 or higher
        -   Core support in [MaterialXGenGLSL](MaterialXGenGLSL) module
        -   Autodesk OGSFX Effect support in [MaterialXGenOgsFx** module
    -   OSL:
        -   [MaterialXGenOsl](MaterialXGenOsl) module.
        -   Uses an implementation which differs from the reference OSL
            implementation

## Definition Libraries

-  Standard library ([stdlib](../documents/Libraries/stdlib)) implementations for supported languages.
-  PBR library ([pbrlib](../documents/Libraries/pbrlib)): node definitions and implementations for supported languages.
-   Basic "default" non-LUT color management transforms for OSL and
    GLSL:
    -   lin_rec709, gamma18, gamma22, gamm24, acescg, srgb_texture
-   Basic "light" definitions and implementations for GLSL:
    -   point, directional, spotlight, IBL

## Library Tree Structure
- Refer to documentation in the [Libraries](../documents/Libraries) folder.

## Unsupported

Nodes and implementations which are not currently supported:
-   ambientocclusion
-   arrayappend
-   curveadjust
-   displacementshader and volumeshader and associated operations (add,
    multiply, mix)
-   mix surfaceshader for GLSL
-   Matrix33 type operations for OSL.

## Rendering Utilities

- [MaterialXRender](MaterialXRender) module.
- Geometry handler with OBJ format support.
- Image handler with formats supported by the "stb" open source loader.
- Render test suite: Windows only.
- GLSL and OSL program validators

## Viewing Utilities

- [MaterialXView](MaterialXView) module
- Sample material viewer.
