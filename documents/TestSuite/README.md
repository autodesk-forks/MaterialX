# Render Test Suite

The test suite sib-folders contains a set of input MaterialX documents. Each file is parsed for renderable elements. Each such element is then compiled, and/or rendered as part of the validation test.

## Folder layout

- The [options file (_options.mtlx)](_options.mtlx) is a MaterialX document with a set of test execution options. The values may be edited locally as desired.
- The main grouping of files is per library ([stdlib](stdlib) and [pbrlib](pbrlib)).
- Sub-folders then group documents based on Element group or category. For example math tests
are found in [stdlib/math](stdlib/math)), with:
    - `math.mtlx`
    - `math_operators.mtlx`
    - `transform.mtlx`
    - `trig.mtlx`, and
    - `vector_math.mtlx`
  documents containing the various Elements to test.
- It is possible add additional tests by simply adding new MaterialX documents under a TestSuite sub-folder.
- The [Geometry](Geometry) and [Images](Images) folders provide stock input geometry and images for usage by the test suite.
- The [Utilities folder](Utilities) provides utilities used for rendering with `testrender` as well as the light configuration used for hardware rendering.

For details on how to build the unit test module to use this test suite see the [source documentation](source/MaterialXTest/README.md).
