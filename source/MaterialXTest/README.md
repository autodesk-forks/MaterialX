# Unit tests

## Core Tests

The names of the files reflect the Element type being tested for the following:

- Document.cpp
- Element.cpp
- Geom.cpp
- Look.cpp
- Material.cpp
- Node.cpp
- Types.cpp
- Value.cpp

## Document utilties
- Observer.cpp : Document observation.
- Traversal.cpp : Document traversal.
- Util.cpp : Basic utilities.

## I/O Tests

- File.cpp : Basic file path tests.
- XmlIo.cpp : XML document I/O tests.

## Shader Generation Tests

- ShaderGen.cpp :

Core framework tests.

- ShaderValid.cpp :

Tests for all GLSL and OSL implementations. If rendering tests are enabled via the build options then each Element tested will be rendered if the appropriate backend support is available.
- GLSL:
    - Will execute on a Windows machine which supports OpenGL 4.0 or above.
- OSL: Uses the utilities "oslc" and "testrender" utilities from the
    [OSL distribution](https://github.com/imageworks/OpenShadingLanguage).
    - The test suite has been tested with version 1.9.10.
    - The following build options are required to be set:
        - MATERIALX_OSLC_EXECUTABLE: Full path to the "oslc" binary.
        - MATERIALX_TESTRENDER_EXECUTABLE: Full path to the "testrender" binary.
        - MATERIALX_OSL_INCLUDE_PATH: Full path to osl include paths (i.e. location of "stdosl.h").

The [test suite folder (documents/TestSuite)](../../documents/TestSuite) contains a set of input MaterialX documents. Each file is parsed for renderable elements. Each such element is compiled, and/or rendered as part of the validation test. It is possible add additional tests by simply adding new MaterialX documents under the TestSuite folder.

Note that the sub-folders group documents based on Element group or category as they are related to the stdlib and pbrlib libraries.

The [options file (documents/TestSuite/_options.mtlx)](../../documents/TestSuite/_options.mtlx) is a MaterialX document with a set of test execution options. The values may be edited locally as desired.

The [Geometry](../../documents/TestSuite/Geometry) and [Images]((../../documents/TestSuite/Geometry) folders provide stock input geometry and images for usage by the test suite.

The [Utilities folder](../../documents/TestSuite/Geometry) provides utilities used for rendering with testrender as well as the light configuration used for hardware rendering.
