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


def add_additional_test_streams(mesh):
    """
    Add additional test streams required by MaterialX test suite.
    
    This mirrors the C++ addAdditionalTestStreams() in RenderUtil.cpp,
    adding geometry attributes needed by geompropvalue, streams, and
    struct_texcoord tests.
    """
    vertex_count = mesh.getVertexCount()
    if vertex_count < 1:
        return
    
    # Get existing UV data for generating test data
    texcoord0_name = f"i_{mx_render.MeshStream.TEXCOORD_ATTRIBUTE}_0"
    texcoord0_stream = mesh.getStream(texcoord0_name)
    if not texcoord0_stream:
        return
    uv_data = texcoord0_stream.getData()
    
    # Second UV set (texcoord1) - copy from texcoord0
    texcoord1_name = f"i_{mx_render.MeshStream.TEXCOORD_ATTRIBUTE}_1"
    if not mesh.getStream(texcoord1_name):
        stream = mx_render.MeshStream.create(
            texcoord1_name, mx_render.MeshStream.TEXCOORD_ATTRIBUTE, 1
        )
        stream.setStride(2)
        stream.resize(vertex_count * 2)
        data = stream.getData()
        for i in range(len(uv_data)):
            data[i] = uv_data[i]
        mesh.addStream(stream)
    
    # Vertex colors (color0) - gradient based on UV
    color0_name = f"i_{mx_render.MeshStream.COLOR_ATTRIBUTE}_0"
    if not mesh.getStream(color0_name):
        stream = mx_render.MeshStream.create(
            color0_name, mx_render.MeshStream.COLOR_ATTRIBUTE, 0
        )
        stream.setStride(4)
        stream.resize(vertex_count * 4)
        data = stream.getData()
        for i in range(vertex_count):
            data[i * 4 + 0] = uv_data[i * 2 + 0]      # R = U
            data[i * 4 + 1] = uv_data[i * 2 + 1]      # G = V
            data[i * 4 + 2] = 1.0                      # B = 1
            data[i * 4 + 3] = 1.0                      # A = 1
        mesh.addStream(stream)
    
    # Second vertex colors (color1)
    color1_name = f"i_{mx_render.MeshStream.COLOR_ATTRIBUTE}_1"
    if not mesh.getStream(color1_name):
        stream = mx_render.MeshStream.create(
            color1_name, mx_render.MeshStream.COLOR_ATTRIBUTE, 1
        )
        stream.setStride(4)
        stream.resize(vertex_count * 4)
        data = stream.getData()
        for i in range(vertex_count):
            data[i * 4 + 0] = 1.0 - uv_data[i * 2 + 0]  # R = 1-U
            data[i * 4 + 1] = 1.0 - uv_data[i * 2 + 1]  # G = 1-V
            data[i * 4 + 2] = 0.0                        # B = 0
            data[i * 4 + 3] = 1.0                        # A = 1
        mesh.addStream(stream)
    
    # Geometry property: integer
    geom_int_name = f"i_{mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE}_geompropvalue_integer"
    if not mesh.getStream(geom_int_name):
        stream = mx_render.MeshStream.create(
            geom_int_name, mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE, 0
        )
        stream.setStride(1)
        stream.resize(vertex_count)
        data = stream.getData()
        for i in range(vertex_count):
            # Store int as float bits (matches C++ reinterpret_cast)
            import struct
            int_val = i % 10  # 0-9 pattern
            data[i] = struct.unpack('f', struct.pack('i', int_val))[0]
        mesh.addStream(stream)
    
    # Geometry property: float
    geom_float_name = f"i_{mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE}_geompropvalue_float"
    if not mesh.getStream(geom_float_name):
        stream = mx_render.MeshStream.create(
            geom_float_name, mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE, 1
        )
        stream.setStride(1)
        stream.resize(vertex_count)
        data = stream.getData()
        for i in range(vertex_count):
            data[i] = uv_data[i * 2 + 0]  # U coordinate
        mesh.addStream(stream)
    
    # Geometry property: vector2
    geom_vec2_name = f"i_{mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE}_geompropvalue_vector2"
    if not mesh.getStream(geom_vec2_name):
        stream = mx_render.MeshStream.create(
            geom_vec2_name, mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE, 1
        )
        stream.setStride(2)
        stream.resize(vertex_count * 2)
        data = stream.getData()
        for i in range(len(uv_data)):
            data[i] = uv_data[i]
        mesh.addStream(stream)
    
    # Geometry property: vector3
    geom_vec3_name = f"i_{mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE}_geompropvalue_vector3"
    if not mesh.getStream(geom_vec3_name):
        stream = mx_render.MeshStream.create(
            geom_vec3_name, mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE, 1
        )
        stream.setStride(3)
        stream.resize(vertex_count * 3)
        data = stream.getData()
        for i in range(vertex_count):
            data[i * 3 + 0] = uv_data[i * 2 + 0]  # U
            data[i * 3 + 1] = uv_data[i * 2 + 1]  # V
            data[i * 3 + 2] = 0.0
        mesh.addStream(stream)
    
    # Geometry property: vector4
    geom_vec4_name = f"i_{mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE}_geompropvalue_vector4"
    if not mesh.getStream(geom_vec4_name):
        stream = mx_render.MeshStream.create(
            geom_vec4_name, mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE, 1
        )
        stream.setStride(4)
        stream.resize(vertex_count * 4)
        data = stream.getData()
        for i in range(vertex_count):
            data[i * 4 + 0] = uv_data[i * 2 + 0]  # U
            data[i * 4 + 1] = uv_data[i * 2 + 1]  # V
            data[i * 4 + 2] = 0.0
            data[i * 4 + 3] = 1.0
        mesh.addStream(stream)
    
    # Geometry property: color2 (not standard but in tests)
    geom_color2_name = f"i_{mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE}_geompropvalue_color2"
    if not mesh.getStream(geom_color2_name):
        stream = mx_render.MeshStream.create(
            geom_color2_name, mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE, 1
        )
        stream.setStride(2)
        stream.resize(vertex_count * 2)
        data = stream.getData()
        for i in range(len(uv_data)):
            data[i] = uv_data[i]
        mesh.addStream(stream)
    
    # Geometry property: color3
    geom_color3_name = f"i_{mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE}_geompropvalue_color3"
    if not mesh.getStream(geom_color3_name):
        stream = mx_render.MeshStream.create(
            geom_color3_name, mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE, 1
        )
        stream.setStride(3)
        stream.resize(vertex_count * 3)
        data = stream.getData()
        for i in range(vertex_count):
            data[i * 3 + 0] = uv_data[i * 2 + 0]  # U
            data[i * 3 + 1] = uv_data[i * 2 + 1]  # V
            data[i * 3 + 2] = 1.0
        mesh.addStream(stream)
    
    # Geometry property: color4
    geom_color4_name = f"i_{mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE}_geompropvalue_color4"
    if not mesh.getStream(geom_color4_name):
        stream = mx_render.MeshStream.create(
            geom_color4_name, mx_render.MeshStream.GEOMETRY_PROPERTY_ATTRIBUTE, 1
        )
        stream.setStride(4)
        stream.resize(vertex_count * 4)
        data = stream.getData()
        for i in range(vertex_count):
            data[i * 4 + 0] = uv_data[i * 2 + 0]  # U
            data[i * 4 + 1] = uv_data[i * 2 + 1]  # V
            data[i * 4 + 2] = 1.0
            data[i * 4 + 3] = 1.0
        mesh.addStream(stream)


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


# Tests requiring struct texcoord handling not yet supported
_STRUCT_TEXCOORD_TESTS = {
    "struct_texcoord",     # Struct texcoord tests need special handling
}

# Tests with known upgrade/compatibility issues
_UPGRADE_TESTS = {
    "upgrade",  # All upgrade tests (syntax_1_22, syntax_1_25, etc.)
}


def _get_stdlib_marks(rel_path: Path, elem_name: str):
    """Get pytest marks for stdlib tests that need special handling."""
    marks = []
    path_str = str(rel_path)
    
    # Tests requiring struct texcoord handling
    for pattern in _STRUCT_TEXCOORD_TESTS:
        if pattern in path_str:
            marks.append(pytest.mark.xfail(
                reason=f"Struct texcoord tests need special handling",
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

