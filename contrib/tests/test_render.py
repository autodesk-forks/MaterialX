"""
Render tests for MaterialX materials.

Uses pytest-subtests for hierarchical test reporting:
- Fast collection: just glob for .mtlx files
- Granular reporting: each element is a subtest
- Clear identification of which materials have issues
"""
import pytest
import MaterialX as mx
import MaterialX.PyMaterialXGenShader as mx_gen_shader
from pathlib import Path
from typing import List

from conftest import (
    get_repo_root,
    get_stdlib_files,
    get_adsk_files,
    should_skip_element,
    get_element_skip_reason,
)

from rendertest.mtlxutils.render_material import render_material


def find_renderable_elements(doc):
    """
    Find all renderable elements in a document.
    
    Returns list of (element, name) tuples.
    Materials with shader nodes come first, then other renderables.
    """
    elements = []
    
    # Material nodes with shaders
    for elem in doc.getMaterialNodes():
        if mx.getShaderNodes(elem):
            elements.append((elem, elem.getName()))
    
    # If no materials, check for renderable outputs
    if not elements:
        for elem in mx_gen_shader.findRenderableElements(doc, False):
            elements.append((elem, elem.getNamePath()))
    
    return elements


def render_element(renderer, doc, elem, search_path, output_path=None):
    """Render a single element and return (success, error_msg)."""
    result = render_material(
        renderer,
        doc,
        elem,
        output_path=output_path,
        search_path=search_path
    )
    
    if result.success:
        return True, None
    else:
        return False, result.error or result.shader_errors or "Unknown error"


class TestRenderStdlibMaterials:
    """
    Test rendering of standard MaterialX library materials.
    
    Covers resources/Materials/TestSuite and resources/Materials/Examples,
    matching the same test cases run by MaterialXTest/Catch2/CTest.
    """
    
    @pytest.mark.parametrize("mtlx_file", get_stdlib_files())
    def test_render_file(
        self,
        mtlx_file: Path,
        subtests,
        renderer,
        stdlib,
        search_path,
        output_dir
    ):
        """Test all renderable elements in a stdlib material file."""
        # Load document
        doc = mx.createDocument()
        doc.importLibrary(stdlib)
        mx.readFromXmlFile(doc, str(mtlx_file))
        
        valid, msg = doc.validate()
        assert valid, f"Document validation failed: {msg}"
        
        # Set up search path
        file_search_path = mx.FileSearchPath(search_path.asString())
        file_search_path.append(str(mtlx_file.parent.resolve()))
        
        # Test each renderable element as a subtest
        elements = find_renderable_elements(doc)
        if not elements:
            pytest.skip("No renderable elements in file")
        
        repo_root = get_repo_root()
        materials_root = repo_root / "resources" / "Materials"
        rel_path = mtlx_file.relative_to(materials_root)
        
        # Construct output directory for this material file to match MaterialXTest layout
        output_path = output_dir / rel_path.parent / mtlx_file.stem
        output_path.mkdir(parents=True, exist_ok=True)
        
        for elem, elem_name in elements:
            with subtests.test(msg=elem_name):
                if should_skip_element(rel_path, elem_name):
                    pytest.skip(get_element_skip_reason(rel_path, elem_name))
                
                success, error = render_element(
                    renderer, doc, elem, file_search_path, output_path=output_path
                )
                assert success, f"Render failed: {error}"


class TestRenderAdskMaterials:
    """Test rendering of Autodesk contributed materials."""
    
    @pytest.mark.parametrize("mtlx_file", get_adsk_files())
    def test_render_file(
        self,
        mtlx_file: Path,
        subtests,
        renderer,
        libraries,
        search_path,
        output_dir
    ):
        """Test all renderable elements in an Autodesk material file."""
        # Load document
        doc = mx.createDocument()
        for lib in libraries:
            doc.importLibrary(lib)
        mx.readFromXmlFile(doc, str(mtlx_file))
        
        valid, msg = doc.validate()
        assert valid, f"Document validation failed: {msg}"
        
        # Set up search path
        file_search_path = mx.FileSearchPath(search_path.asString())
        file_search_path.append(str(mtlx_file.parent.resolve()))
        
        # Test each renderable element as a subtest
        elements = find_renderable_elements(doc)
        if not elements:
            pytest.skip("No renderable elements in file")
        
        repo_root = get_repo_root()
        materials_dir = repo_root / "contrib" / "adsk" / "resources" / "Materials"
        rel_path = mtlx_file.relative_to(materials_dir)
        
        # Construct output directory for this material file to match MaterialXTest layout
        output_path = output_dir / rel_path.parent / mtlx_file.stem
        output_path.mkdir(parents=True, exist_ok=True)
        
        for elem, elem_name in elements:
            with subtests.test(msg=elem_name):
                # Skip Proceduralwood due to relative include issues
                if "Proceduralwood" in str(rel_path):
                    pytest.skip("adsklib relative includes require source build layout")
                
                success, error = render_element(
                    renderer, doc, elem, file_search_path, output_path=output_path
                )
                assert success, f"Render failed: {error}"
