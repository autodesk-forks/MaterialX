"""
GLSL render tests for MaterialX materials.

Each material becomes a separate test case, enabling:
- Individual pass/fail reporting
- Parallel execution with pytest-xdist
- Clear identification of which materials have issues
"""
import pytest
import MaterialX as mx
import MaterialX.PyMaterialXGenShader as mx_gen_shader
from pathlib import Path
from typing import List

from conftest import discover_stdlib_materials, discover_adsk_materials

# Import shared render logic
from rendertest.mtlxutils.render_material import render_material


def find_render_element(doc, element_name: str):
    """
    Find a renderable element by name in a document.
    
    Searches material nodes first, then falls back to findRenderableElements
    for output nodes used in TestSuite.
    """
    # First try material nodes (most common case)
    for elem in doc.getMaterialNodes():
        if elem.getName() == element_name:
            return elem
    
    # Fall back to findRenderableElements for outputs
    for elem in mx_gen_shader.findRenderableElements(doc, False):
        if elem.getNamePath() == element_name:
            return elem
    
    return None


def render_and_validate(
    glsl_renderer,
    mtlx_file: Path,
    element_name: str,
    libs: List,
    search_path
):
    """
    Common test logic: load document, find element, render, validate.
    
    Args:
        glsl_renderer: Initialized GLSL renderer
        mtlx_file: Path to .mtlx file
        element_name: Name of material/output to render
        libs: List of library documents to import
        search_path: Base search path for resources
    """
    # Create document with libraries
    doc = mx.createDocument()
    for lib in libs:
        doc.importLibrary(lib)
    
    # Load the material file
    mx.readFromXmlFile(doc, str(mtlx_file))
    valid, msg = doc.validate()
    assert valid, f"Document validation failed: {msg}"
    
    # Set up search path for textures and includes
    file_search_path = mx.FileSearchPath(search_path.asString())
    file_search_path.append(str(mtlx_file.parent.resolve()))
    
    # Find the renderable element
    render_node = find_render_element(doc, element_name)
    assert render_node is not None, f"Element '{element_name}' not found in {mtlx_file}"
    
    # Render using shared logic
    result = render_material(
        glsl_renderer,
        doc,
        render_node,
        output_path=None,  # Don't save images during test
        search_path=file_search_path
    )
    
    # Assert success
    if not result.success:
        error_msg = result.error or result.shader_errors or "Unknown error"
        pytest.fail(f"Render failed for {element_name}: {error_msg}")


class TestRenderStdlibMaterials:
    """
    Test rendering of standard MaterialX library materials.
    
    Covers resources/Materials/TestSuite and resources/Materials/Examples,
    matching the same test cases run by MaterialXTest/Catch2/CTest.
    """
    
    @pytest.mark.parametrize("mtlx_file,element_name", discover_stdlib_materials())
    def test_render_material(
        self,
        mtlx_file: Path,
        element_name: str,
        glsl_renderer,
        stdlib,
        search_path
    ):
        """Test that a stdlib material/output renders successfully."""
        render_and_validate(
            glsl_renderer, mtlx_file, element_name, [stdlib], search_path
        )


class TestRenderAdskMaterials:
    """Test rendering of Autodesk contributed materials."""
    
    @pytest.mark.parametrize("mtlx_file,material_name", discover_adsk_materials())
    def test_render_material(
        self,
        mtlx_file: Path,
        material_name: str,
        glsl_renderer,
        libraries,
        search_path
    ):
        """Test that an Autodesk material renders successfully."""
        render_and_validate(
            glsl_renderer, mtlx_file, material_name, libraries, search_path
        )
