# Library Structure

## Standard Library
-   **stdlib**
    - stdlib_defs.mtlx : Single node nodedef definitions file.
|   - stdlib_ng.mtlx :  Node graph definitions file.
    -   genglsl: GLSL support
        -   lib : Utilities
        - stdlib_genglsl_impl.mtlx : Mapping from definitions to implementations
        - cm_genglsl_impl.mtlx : Color management implementations
        -   < implementation files >
        -   ogsfx: OGSFX support
            - stdlib_genglsl_ogsfx_impl.mtlx : Mapping from definitions to implementations
            -   < implementation files >
    -   genosl: OSL support
        -   lib: Utilities
        - stdlib_genosl_impl.mtlx : Mapping from definitions to implementations
        - cm_genosl_impl.mtlx : Color management implementations
        -   < implementation files >
    -   Osl: Reference support
        -   < implementation files >

## PBR library
-   **pbrlib**
    - pbrlib_defs.mtlx : Single node definitions file.
    - pbrlib_ng.mtlx : Node graph definitions file.
    -   genglsl : GLSL support
        -   lib : Utilities
        - pbrlib_genglsl_impl.mtlx : Mapping from definitions to implementations
        -   < implementation files >
        -   ogsfx : OGSFX support
            - < implementation files >
    -   genosl : OSL support
        -   lib : Utilities
        - pbrlib_genosl_impl.mtlx : Mapping from definitions to implementations
        -   < implementation files >
