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
import MaterialX.PyMaterialXRender as mx_render
from rendertest.mtlxutils import mxrenderer


def get_repo_root() -> Path:
    """Get MaterialX repository root."""
    return Path(__file__).parent.parent.parent


@pytest.fixture(scope="session")
def repo_root() -> Path:
    """MaterialX repository root path."""
    return get_repo_root()


@pytest.fixture(scope="session")
def output_dir(repo_root) -> Path:
    """Derived output directory for rendered images, which is gitignored."""
    path = repo_root / "contrib" / "renders"
    path.mkdir(parents=True, exist_ok=True)
    return path


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


def _add_stream_if_missing(mesh, name, attr_type, index, stride, fill_func):
    """Helper to create and add a mesh stream if it doesn't exist."""
    if mesh.getStream(name):
        return
    stream = mx_render.MeshStream.create(name, attr_type, index)
    stream.setStride(stride)
    stream.resize(mesh.getVertexCount() * stride)
    fill_func(stream.getData())
    mesh.addStream(stream)


def add_additional_test_streams(mesh):
    """
    Add additional test streams required by MaterialX test suite.
    
    This mirrors the C++ addAdditionalTestStreams() in RenderUtil.cpp,
    adding geometry attributes needed by geompropvalue, streams, and
    struct_texcoord tests.
    """
    import struct as struct_module
    
    n = mesh.getVertexCount()
    if n < 1:
        return
    
    # Get existing UV data for generating test data
    uv_stream = mesh.getStream(f"i_{mx_render.MeshStream.TEXCOORD_ATTRIBUTE}_0")
    if not uv_stream:
        return
    uv = uv_stream.getData()
    
    TEXCOORD = mx_render.MeshStream.TEXCOORD_ATTRIBUTE
    COLOR = mx_render.MeshStream.COLOR_ATTRIBUTE
    GEOMPROP = mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE
    
    # Second UV set - copy from texcoord0
    _add_stream_if_missing(mesh, f"i_{TEXCOORD}_1", TEXCOORD, 1, 2,
        lambda d: [d.__setitem__(i, uv[i]) for i in range(len(uv))])
    
    # Vertex colors - RGBA from UV
    def fill_color0(d):
        for i in range(n):
            d[i*4], d[i*4+1], d[i*4+2], d[i*4+3] = uv[i*2], uv[i*2+1], 1.0, 1.0
    _add_stream_if_missing(mesh, f"i_{COLOR}_0", COLOR, 0, 4, fill_color0)
    
    def fill_color1(d):
        for i in range(n):
            d[i*4], d[i*4+1], d[i*4+2], d[i*4+3] = 1.0-uv[i*2], 1.0-uv[i*2+1], 0.0, 1.0
    _add_stream_if_missing(mesh, f"i_{COLOR}_1", COLOR, 1, 4, fill_color1)
    
    # Geometry properties for geompropvalue tests
    def fill_int(d):
        for i in range(n):
            d[i] = struct_module.unpack('f', struct_module.pack('i', i % 10))[0]
    _add_stream_if_missing(mesh, f"i_{GEOMPROP}_geompropvalue_integer", GEOMPROP, 0, 1, fill_int)
    
    _add_stream_if_missing(mesh, f"i_{GEOMPROP}_geompropvalue_float", GEOMPROP, 1, 1,
        lambda d: [d.__setitem__(i, uv[i*2]) for i in range(n)])
    
    _add_stream_if_missing(mesh, f"i_{GEOMPROP}_geompropvalue_vector2", GEOMPROP, 1, 2,
        lambda d: [d.__setitem__(i, uv[i]) for i in range(len(uv))])
    
    def fill_vec3(d):
        for i in range(n):
            d[i*3], d[i*3+1], d[i*3+2] = uv[i*2], uv[i*2+1], 0.0
    _add_stream_if_missing(mesh, f"i_{GEOMPROP}_geompropvalue_vector3", GEOMPROP, 1, 3, fill_vec3)
    
    def fill_vec4(d):
        for i in range(n):
            d[i*4], d[i*4+1], d[i*4+2], d[i*4+3] = uv[i*2], uv[i*2+1], 0.0, 1.0
    _add_stream_if_missing(mesh, f"i_{GEOMPROP}_geompropvalue_vector4", GEOMPROP, 1, 4, fill_vec4)
    
    _add_stream_if_missing(mesh, f"i_{GEOMPROP}_geompropvalue_color2", GEOMPROP, 1, 2,
        lambda d: [d.__setitem__(i, uv[i]) for i in range(len(uv))])
    
    def fill_color3(d):
        for i in range(n):
            d[i*3], d[i*3+1], d[i*3+2] = uv[i*2], uv[i*2+1], 1.0
    _add_stream_if_missing(mesh, f"i_{GEOMPROP}_geompropvalue_color3", GEOMPROP, 1, 3, fill_color3)
    
    def fill_color4(d):
        for i in range(n):
            d[i*4], d[i*4+1], d[i*4+2], d[i*4+3] = uv[i*2], uv[i*2+1], 1.0, 1.0
    _add_stream_if_missing(mesh, f"i_{GEOMPROP}_geompropvalue_color4", GEOMPROP, 1, 4, fill_color4)


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
    
    # Add test geometry streams for geompropvalue, streams, and struct_texcoord tests
    geom_handler = renderer.renderer.getGeometryHandler()
    for mesh in geom_handler.getMeshes():
        add_additional_test_streams(mesh)
    
    return renderer


@pytest.fixture(scope="session")
def renderer(glsl_renderer):
    """
    Session-scoped renderer fixture.
    
    Provides a generic renderer interface for tests. Currently maps to the
    GLSL renderer, but can be parameterized or extended in the future to support
    other rendering backends (e.g. MSL, OSL, Slang).
    """
    return glsl_renderer


# Element skip patterns
_SKIP_PATTERNS = {
    "struct_texcoord": "Struct texcoord tests need special handling",
    "upgrade": "Syntax upgrade test - may have compatibility issues",
}


def should_skip_element(rel_path: Path, elem_name: str) -> bool:
    """Check if an element should be skipped based on path patterns."""
    path_str = str(rel_path)
    for pattern in _SKIP_PATTERNS:
        if pattern in path_str:
            return True
    return False


def get_element_skip_reason(rel_path: Path, elem_name: str) -> str:
    """Get the skip reason for an element."""
    path_str = str(rel_path)
    for pattern, reason in _SKIP_PATTERNS.items():
        if pattern in path_str:
            return reason
    return "Unknown"


def get_stdlib_files():
    """
    Get list of stdlib .mtlx files for parametrization.
    
    Fast collection - just globs for files, no parsing.
    """
    repo_root = get_repo_root()
    materials_root = repo_root / "resources" / "Materials"
    
    test_dirs = [
        materials_root / "TestSuite",
        materials_root / "Examples",
    ]
    
    files = []
    for test_dir in test_dirs:
        if test_dir.exists():
            for mtlx_file in sorted(test_dir.rglob("*.mtlx")):
                if not mtlx_file.name.startswith("_"):
                    rel_path = mtlx_file.relative_to(materials_root)
                    file_id = str(rel_path).replace("\\", "/")
                    files.append(pytest.param(mtlx_file, id=file_id))
    
    return files


def get_adsk_files():
    """
    Get list of adsk .mtlx files for parametrization.
    
    Fast collection - just globs for files, no parsing.
    """
    repo_root = get_repo_root()
    materials_dir = repo_root / "contrib" / "adsk" / "resources" / "Materials"
    
    if not materials_dir.exists():
        return []
    
    files = []
    for mtlx_file in sorted(materials_dir.rglob("*.mtlx")):
        rel_path = mtlx_file.relative_to(materials_dir)
        file_id = str(rel_path).replace("\\", "/")
        files.append(pytest.param(mtlx_file, id=file_id))
    
    return files

