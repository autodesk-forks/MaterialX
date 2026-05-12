"""
pytest configuration and fixtures for MaterialX rendering tests.

Fixtures are session-scoped to amortize setup cost across test cases.
Each pytest-xdist worker process gets its own fixture instances.
"""
import sys
from pathlib import Path

import pytest

# Add rendertest and mtlxutils to import path
# Note: mxrenderer.py uses `from mtlxutils import ...` so we need both paths
_rendertest_path = Path(__file__).parent.parent / "utilities" / "scripts"
_mtlxutils_path = _rendertest_path / "rendertest"
sys.path.insert(0, str(_rendertest_path))
sys.path.insert(0, str(_mtlxutils_path))

import MaterialX as mx
import MaterialX.PyMaterialXGenShader as mx_gen_shader
from rendertest.mtlxutils import mxrenderer


def get_repo_root() -> Path:
    """Get MaterialX repository root."""
    return Path(__file__).parent.parent.parent


@pytest.fixture(scope="session")
def repo_root() -> Path:
    """MaterialX repository root path."""
    return get_repo_root()


@pytest.fixture(scope="session")
def search_path(repo_root) -> mx.FileSearchPath:
    """MaterialX search path including adsk libraries."""
    sp = mx.getDefaultDataSearchPath()
    adsk_path = repo_root / "contrib" / "adsk" / "libraries"
    if adsk_path.exists():
        sp.append(str(adsk_path))
    return sp


@pytest.fixture(scope="session")
def stdlib(search_path):
    """Load MaterialX standard library once per worker."""
    lib = mx.createDocument()
    library_folders = mx.getDefaultDataLibraryFolders()
    mx.loadLibraries(library_folders, search_path, lib)
    return lib


@pytest.fixture(scope="session")
def adsklib(search_path, repo_root):
    """Load Autodesk library once per worker."""
    lib = mx.createDocument()
    adsk_path = repo_root / "contrib" / "adsk" / "libraries"
    if adsk_path.exists():
        adsk_search = mx.FileSearchPath(str(adsk_path))
        mx.loadLibraries(["adsklib"], adsk_search, lib)
    return lib


@pytest.fixture(scope="session")
def libraries(stdlib, adsklib):
    """Combined libraries for document creation."""
    return [stdlib, adsklib]


@pytest.fixture(scope="session")
def glsl_renderer(stdlib, search_path, repo_root):
    """
    Initialize GLSL renderer once per worker process.
    
    This is the expensive setup that we want to share across tests.
    """
    # IBL paths
    lights_path = repo_root / "resources" / "Lights"
    radiance_path = lights_path / "san_giuseppe_bridge.hdr"
    irradiance_path = lights_path / "irradiance" / "san_giuseppe_bridge.hdr"
    
    # Geometry
    geometry_path = repo_root / "resources" / "Geometry" / "sphere.obj"
    
    # Render size
    width = height = 512
    
    renderer = mxrenderer.initializeRenderer(
        stdlib,
        search_path,
        str(radiance_path),
        str(irradiance_path),
        width,
        height,
        str(geometry_path)
    )
    
    return renderer


# Tests requiring custom geometry properties (not in sphere.obj)
_GEOM_PROP_TESTS = {
    "geompropvalue",       # Custom geometry properties
    "struct_texcoord",     # Struct texcoord tests
}

# Specific element names that require geometry attributes we don't have
_GEOM_ATTR_ELEMENTS = {
    "texcoord1_output",    # Requires second UV set
    "color_float_output",  # Requires vertex colors
    "color_vec3_output",   # Requires vertex colors
    "color_vec4_output",   # Requires vertex colors
}

# Tests with known upgrade/compatibility issues
_UPGRADE_TESTS = {
    "upgrade",  # All upgrade tests (syntax_1_22, syntax_1_25, etc.)
}


def _get_stdlib_marks(rel_path: Path, elem_name: str):
    """Get pytest marks for stdlib tests that need special handling."""
    marks = []
    path_str = str(rel_path)
    
    # Tests requiring custom geometry properties
    for pattern in _GEOM_PROP_TESTS:
        if pattern in path_str:
            marks.append(pytest.mark.xfail(
                reason=f"Requires geometry with {pattern} attributes",
                strict=False
            ))
            return marks
    
    # Specific elements requiring geometry attributes
    if elem_name in _GEOM_ATTR_ELEMENTS:
        marks.append(pytest.mark.xfail(
            reason=f"Requires geometry attribute: {elem_name}",
            strict=False
        ))
        return marks
    
    # Syntax upgrade tests
    for pattern in _UPGRADE_TESTS:
        if pattern in path_str:
            marks.append(pytest.mark.xfail(
                reason="Syntax upgrade test - may have compatibility issues",
                strict=False
            ))
            return marks
    
    return marks


def discover_stdlib_materials():
    """
    Discover all renderable materials in resources/Materials at collection time.
    
    This covers TestSuite (stdlib, pbrlib, nprlib) and Examples directories,
    matching the same test cases run by MaterialXTest/Catch2.
    
    Yields (mtlx_file, material_name) pairs for parametrization.
    """
    repo_root = get_repo_root()
    materials_root = repo_root / "resources" / "Materials"
    
    # Match MaterialXTest: TestSuite + Examples
    test_dirs = [
        materials_root / "TestSuite",
        materials_root / "Examples",
    ]
    
    # Load stdlib for material discovery
    stdlib = mx.createDocument()
    search_path = mx.getDefaultDataSearchPath()
    mx.loadLibraries(mx.getDefaultDataLibraryFolders(), search_path, stdlib)
    
    for test_dir in test_dirs:
        if not test_dir.exists():
            continue
            
        for mtlx_file in sorted(test_dir.rglob("*.mtlx")):
            # Skip options files
            if mtlx_file.name.startswith("_"):
                continue
                
            try:
                doc = mx.createDocument()
                doc.importLibrary(stdlib)
                mx.readFromXmlFile(doc, str(mtlx_file))
                
                # Find renderable material nodes
                material_nodes = list(doc.getMaterialNodes())
                
                if material_nodes:
                    for elem in material_nodes:
                        # Skip materials without shader nodes (matches findRenderableElements)
                        if mx.getShaderNodes(elem):
                            rel_path = mtlx_file.relative_to(materials_root)
                            test_id = f"{rel_path.parent}/{rel_path.stem}/{elem.getName()}"
                            
                            marks = _get_stdlib_marks(rel_path, elem.getName())
                            yield pytest.param(
                                mtlx_file,
                                elem.getName(),
                                id=test_id,
                                marks=marks
                            )
                else:
                    # Some TestSuite files have renderable outputs but no materials
                    # Use findRenderableElements to catch these
                    renderables = mx_gen_shader.findRenderableElements(doc, False)
                    for elem in renderables:
                        rel_path = mtlx_file.relative_to(materials_root)
                        elem_name = elem.getNamePath()
                        test_id = f"{rel_path.parent}/{rel_path.stem}/{elem_name}"
                        
                        marks = _get_stdlib_marks(rel_path, elem_name)
                        yield pytest.param(
                            mtlx_file,
                            elem_name,
                            id=test_id,
                            marks=marks
                        )
                        
            except mx.Exception:
                # Skip files that fail to load during discovery
                continue


def discover_adsk_materials():
    """
    Discover all renderable materials in contrib/adsk at collection time.
    
    Yields (mtlx_file, material_name) pairs for parametrization.
    """
    repo_root = get_repo_root()
    materials_dir = repo_root / "contrib" / "adsk" / "resources" / "Materials"
    
    if not materials_dir.exists():
        return
    
    # Load stdlib for material discovery
    stdlib = mx.createDocument()
    search_path = mx.getDefaultDataSearchPath()
    mx.loadLibraries(mx.getDefaultDataLibraryFolders(), search_path, stdlib)
    
    for mtlx_file in sorted(materials_dir.rglob("*.mtlx")):
        try:
            doc = mx.createDocument()
            doc.importLibrary(stdlib)
            mx.readFromXmlFile(doc, str(mtlx_file))
            
            # Find renderable elements (only materials with shader nodes)
            for elem in doc.getMaterialNodes():
                if not mx.getShaderNodes(elem):
                    continue
                    
                rel_path = mtlx_file.relative_to(materials_dir)
                test_id = f"{rel_path.parent}/{rel_path.stem}/{elem.getName()}"
                
                # Proceduralwood uses relative includes
                # (../../stdlib/...) that require a source
                # build layout
                marks = []
                if "Proceduralwood" in str(rel_path):
                    marks.append(pytest.mark.xfail(
                        reason="adsklib relative includes "
                               "require source build layout",
                        strict=False
                    ))
                
                yield pytest.param(
                    mtlx_file,
                    elem.getName(),
                    id=test_id,
                    marks=marks
                )
        except mx.Exception:
            # Skip files that fail to load during discovery
            continue

