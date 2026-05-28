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
                            # Normalize path separators and use : to separate file from element
                            file_part = str(rel_path).replace("\\", "/")
                            test_id = f"{file_part}:{elem.getName()}"
                            
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
                        # Normalize path separators and use : to separate file from element
                        file_part = str(rel_path).replace("\\", "/")
                        test_id = f"{file_part}:{elem_name}"
                        
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
                # Normalize path separators and use : to separate file from element
                file_part = str(rel_path).replace("\\", "/")
                test_id = f"{file_part}:{elem.getName()}"
                
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


def record_pytest_result(repo_root: Path, key: str, status: str):
    """Record a pytest test/subtest result to a worker-specific file."""
    import os
    worker_file = repo_root / "contrib" / f"pytest_coverage_worker_{os.getpid()}.txt"
    with open(worker_file, "a", encoding="utf-8") as f:
        f.write(f"{key}\t{status}\n")


def pytest_sessionfinish(session, exitstatus):
    """
    On session finish, if we are on the master process (or sequential run),
    consolidate all worker-specific coverage files into pytest_coverage.txt.
    """
    # Only run on master process (or if sequential)
    if not hasattr(session.config, "workerinput"):
        repo_root = get_repo_root()
        contrib_dir = repo_root / "contrib"
        
        # Gather all worker files
        worker_files = list(contrib_dir.glob("pytest_coverage_worker_*.txt"))
        if not worker_files:
            return
            
        entries = {}
        for wf in worker_files:
            try:
                for line in wf.read_text(encoding="utf-8").splitlines():
                    if line.strip():
                        parts = line.strip().split("\t")
                        if len(parts) == 2:
                            entries[parts[0]] = parts[1]
                wf.unlink() # Delete worker file
            except Exception as e:
                print(f"Warning: Failed to process worker file {wf}: {e}", file=sys.stderr)
                
        # Write consolidated manifest
        manifest_file = contrib_dir / "pytest_coverage.txt"
        lines = [f"{key}\t{status}" for key, status in sorted(entries.items())]
        manifest_file.write_text("\n".join(lines) + ("\n" if lines else ""), encoding="utf-8")
        print(f"\nConsolidated pytest coverage manifest written to: {manifest_file}")


