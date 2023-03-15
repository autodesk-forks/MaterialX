# MaterialX build step for Autodesk

- Need debug filename qualifiers
- Need to define custom layout to meet Package Config guidelines.

# Generate steps:
```
cmake -S . -B_mtlxbuild -G "Visual Studio 14 2015 Win64" 
-DCMAKE_INSTALL_PREFIX=%CD%\install 
-DMATERIALX_BUILD_PYTHON=OFF 
-DMATERIALX_TEST_RENDER=OFF 
-DMATERIALX_BUILD_RENDER=ON
-DMATERIALX_BUILD_TESTS=OFF 
-DMATERIALX_DEBUG_POSTFIX=d 
-DMATERIALX_INSTALL_INCLUDE_PATH=inc 
-DMATERIALX_INSTALL_LIB_PATH=libs 
-DMATERIALX_INSTALL_STDLIB_PATH=libraries
```

# Package Types:
We have packages following the Autodesk package guidelines but these are not compatible with standard Open Source Install SDK packages that use CMAKE for configuration.
Therefore, a *sdk_{os}_{platform} is also available.

# Versioning
Package version is based on MaterialX tags and current Autodesk guidance
For example https://github.com/materialx/MaterialX/releases/tag/v1.38.1, and that its current release is 1.38.1, 
then guidance suggests we implement our own MAJOR version, say "1", use the concatenated MaterialX version "1381" as MINOR, and CI assigns the PATCH {patchid}. 
This produces a version would be 1.1381.1. We have control of the MAJOR version and need to increment it when we know backward binary compatibility is lost.
