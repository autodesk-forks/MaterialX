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


def pytest_addoption(parser):
    """Register custom command-line options for MaterialX render tests."""
    parser.addoption(
        "--baseline-dir",
        action="store",
        default=None,
        help="Path to directory containing baseline images for comparison."
    )
    parser.addoption(
        "--flip-threshold",
        action="store",
        type=float,
        default=0.05,
        help="Mean FLIP error threshold above which a comparison fails."
    )
    parser.addoption(
        "--output-dir",
        action="store",
        default=None,
        help="Path to directory where rendered images will be saved."
    )


@pytest.fixture(scope="session")
def baseline_dir(request) -> Path:
    """Path to the baseline directory, or None if not specified."""
    opt = request.config.getoption("--baseline-dir")
    return Path(opt) if opt else None


@pytest.fixture(scope="session")
def flip_threshold(request) -> float:
    """Mean FLIP error threshold for comparison gating."""
    return request.config.getoption("--flip-threshold")


@pytest.fixture(scope="session")
def output_dir(request, repo_root) -> Path:
    """Derived output directory for rendered images, which is gitignored."""
    opt = request.config.getoption("--output-dir")
    if opt:
        path = Path(opt)
    else:
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


@pytest.fixture(scope="session")
def data_library(stdlib, adsklib):
    """Combined data library (stdlib + adsklib) as a single document.

    Mirrors the C++ tests' single ``dependLib`` document. Test documents
    reference it via ``Document.setDataLibrary`` rather than merging libraries
    in with ``importLibrary`` -- merging before upgrading old-syntax documents
    can produce spurious "too many bindings" validation errors.
    """
    lib = mx.createDocument()
    lib.importLibrary(stdlib)
    lib.importLibrary(adsklib)
    return lib


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


def get_output_path_for_file(mtlx_file: Path, output_dir: Path) -> Path:
    """Derive the output directory path for a given material file."""
    repo_root = get_repo_root()
    materials_root = repo_root / "resources" / "Materials"
    materials_dir = repo_root / "contrib" / "adsk" / "resources" / "Materials"
    
    if mtlx_file.is_relative_to(materials_root):
        rel_path = mtlx_file.relative_to(materials_root)
    elif mtlx_file.is_relative_to(materials_dir):
        rel_path = mtlx_file.relative_to(materials_dir)
    else:
        return None
        
    return output_dir / rel_path.parent / mtlx_file.stem


from collections import defaultdict

_node_funcargs = {}
_subtest_html_extras = defaultdict(list)


@pytest.hookimpl(hookwrapper=True)
def pytest_runtest_makereport(item, call):
    """Hook to capture the funcargs for each test item so we can access them in logreport."""
    outcome = yield
    report = outcome.get_result()
    _node_funcargs[item.nodeid] = item.funcargs


@pytest.hookimpl(tryfirst=True)
def pytest_runtest_logreport(report):
    """Hook to capture subtest failures and append their visual comparisons to the main test report."""
    if type(report).__name__ == "SubtestReport" and report.failed:
        try:
            from pytest_html import extras
        except ImportError:
            return
            
        funcargs = _node_funcargs.get(report.nodeid)
        if not funcargs:
            return
            
        mtlx_file = funcargs.get("mtlx_file")
        output_dir = funcargs.get("output_dir")
        baseline_dir = funcargs.get("baseline_dir")
        
        if not mtlx_file or not output_dir:
            return
            
        # Extract subtest name from report context
        context = getattr(report, "context", None)
        subtest_name = context.msg if context else None
        if not subtest_name:
            return
            
        output_path = get_output_path_for_file(mtlx_file, output_dir)
        if not output_path or not output_path.exists():
            return
            
        # Find the rendered file
        import MaterialX as mx
        valid_elem_name = mx.createValidName(subtest_name)
        rendered_files = list(output_path.glob(f"{valid_elem_name}_*.png"))
        rendered_files = [f for f in rendered_files if not f.name.endswith("_diff.png")]
        
        if not rendered_files:
            return
            
        rendered_file = rendered_files[0]
        
        # Derive baseline and heatmap paths
        rel_rendered = rendered_file.relative_to(output_dir)
        baseline_file = baseline_dir / rel_rendered if baseline_dir else None
        heatmap_file = rendered_file.parent / f"{rendered_file.stem}_diff.png"
        
        # Determine HTML report directory to compute relative paths for images
        import os
        htmlpath_str = report.config.getoption("htmlpath")
        html_dir = Path(htmlpath_str).parent.resolve() if htmlpath_str else None
        
        # Fall back to base64 encoding only if we are generating a self-contained HTML report
        is_self_contained = report.config.getoption("self_contained_html") if htmlpath_str else False
        
        def get_image_src(path: Path) -> str:
            if not path or not path.exists():
                return ""
            if is_self_contained:
                import base64
                try:
                    with open(path, "rb") as f:
                        encoded = base64.b64encode(f.read()).decode("utf-8")
                        return f"data:image/png;base64,{encoded}"
                except Exception:
                    pass
            elif html_dir:
                try:
                    # Compute relative path from the HTML report to the image file
                    return os.path.relpath(path.resolve(), html_dir).replace("\\", "/")
                except ValueError:
                    return path.resolve().as_uri()
            return path.resolve().as_uri()
            
        rendered_src = get_image_src(rendered_file)
        baseline_src = get_image_src(baseline_file)
        heatmap_src = get_image_src(heatmap_file)
        
        if not rendered_src:
            return
            
        if baseline_src:
            baseline_img_tag = f'<img src="{baseline_src}" style="max-width: 100%; height: auto; border: 1px solid #ccc; border-radius: 4px;" />'
        else:
            baseline_img_tag = '<div style="padding: 50px 10px; background: #eee; border: 1px dashed #ccc; border-radius: 4px; color: #666; font-size: 12px;">Baseline image missing</div>'
            
        if heatmap_src:
            heatmap_img_tag = f'<img src="{heatmap_src}" style="max-width: 100%; height: auto; border: 1px solid #ccc; border-radius: 4px;" />'
        else:
            heatmap_img_tag = '<div style="padding: 50px 10px; background: #eee; border: 1px dashed #ccc; border-radius: 4px; color: #666; font-size: 12px;">No heatmap (comparison passed or skipped)</div>'
            
        html_content = f"""
        <div style="margin-top: 15px; padding: 15px; border: 1px solid #e74c3c; border-radius: 6px; background: #fdf2f2; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;">
            <h4 style="margin: 0 0 12px 0; color: #c0392b; font-size: 14px;">Visual Comparison for {subtest_name}</h4>
            <div style="display: flex; gap: 15px; flex-wrap: wrap;">
                <div style="flex: 1; min-width: 220px; text-align: center;">
                    <div style="font-weight: bold; margin-bottom: 6px; font-size: 12px; color: #555;">Baseline (Reference)</div>
                    {baseline_img_tag}
                </div>
                <div style="flex: 1; min-width: 220px; text-align: center;">
                    <div style="font-weight: bold; margin-bottom: 6px; font-size: 12px; color: #555;">Rendered (Optimized)</div>
                    <img src="{rendered_src}" style="max-width: 100%; height: auto; border: 1px solid #ccc; border-radius: 4px;" />
                </div>
                <div style="flex: 1; min-width: 220px; text-align: center;">
                    <div style="font-weight: bold; margin-bottom: 6px; font-size: 12px; color: #555;">FLIP Heatmap</div>
                    {heatmap_img_tag}
                </div>
            </div>
        </div>
        """
        _subtest_html_extras[report.nodeid].append(extras.html(html_content))

    elif type(report).__name__ == "TestReport" and report.when == "teardown":
        if report.nodeid in _subtest_html_extras:
            try:
                from pytest_html import extras
            except ImportError:
                return
            extra = getattr(report, "extras", [])
            extra.extend(_subtest_html_extras[report.nodeid])
            report.extras = extra
            del _subtest_html_extras[report.nodeid]


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


def compare_rendered_image(rendered_path: Path, baseline_path: Path, heatmap_path: Path = None) -> dict:
    """
    Compare a rendered image against a baseline image using NVIDIA FLIP.
    
    Returns a dictionary with comparison results:
    {
        'success': bool,
        'mean_flip': float,
        'max_flip': float,
        'pct_diff_pixels': float,
        'error': str or None,
        'heatmap_path': Path or None
    }
    """
    if not rendered_path.exists():
        return {'success': False, 'error': f"Rendered image not found: {rendered_path}"}
    if not baseline_path.exists():
        return {'success': False, 'error': f"Baseline image not found: {baseline_path}"}
        
    try:
        import flip_evaluator as flip
        import numpy as np
    except ImportError as e:
        return {'success': False, 'error': f"Required packages missing: {e}"}

    try:
        # Run FLIP evaluation (LDR mode, sRGB input)
        flip_map, mean_flip, _ = flip.evaluate(
            str(baseline_path),
            str(rendered_path),
            "LDR",
            inputsRGB=True,
            applyMagma=False,
            computeMeanError=True,
            parameters={"ppd": 70.0}
        )
    except Exception as e:
        return {'success': False, 'error': f"FLIP evaluation failed: {e}"}

    flip_map = np.array(flip_map)
    max_flip = float(flip_map.max())

    # Percentage of pixels with perceptible difference (FLIP > 0.01)
    diff_pixels = flip_map > 0.01
    pct_diff_pixels = 100.0 * diff_pixels.sum() / diff_pixels.size

    result = {
        'success': True,
        'mean_flip': float(mean_flip),
        'max_flip': max_flip,
        'pct_diff_pixels': pct_diff_pixels,
        'error': None,
        'heatmap_path': None
    }

    # Save heatmap if requested
    if heatmap_path:
        try:
            heatmap_img, _, _ = flip.evaluate(
                str(baseline_path),
                str(rendered_path),
                "LDR",
                inputsRGB=True,
                applyMagma=True,
                computeMeanError=False,
                parameters={"ppd": 70.0}
            )
            from PIL import Image
            heatmap_arr = np.array(heatmap_img)
            if heatmap_arr.max() <= 1.0:
                heatmap_arr = (heatmap_arr * 255).astype(np.uint8)
            Image.fromarray(heatmap_arr).save(heatmap_path)
            result['heatmap_path'] = heatmap_path
        except Exception as e:
            result['error'] = f"Failed to save heatmap: {e}"

    return result

