"""
GLSL render tests for Autodesk MaterialX materials.

Each material becomes a separate test case, enabling:
- Individual pass/fail reporting
- Parallel execution with pytest-xdist
- Clear identification of which materials have issues
"""
import pytest
import MaterialX as mx
from pathlib import Path

from conftest import discover_adsk_materials

# Import shared render logic
from rendertest.mtlxutils.render_material import render_material, find_renderable_materials


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
        """
        Test that a single material renders successfully.
        
        This test verifies:
        1. Shader generation succeeds
        2. GPU program creation succeeds
        3. Render completes without errors
        
        Future: Add baseline image comparison.
        """
        # Create document with libraries
        doc = mx.createDocument()
        for lib in libraries:
            doc.importLibrary(lib)
        
        # Load the material file
        mx.readFromXmlFile(doc, str(mtlx_file))
        valid, msg = doc.validate()
        assert valid, f"Document validation failed: {msg}"
        
        # Update search path for textures (copy and extend)
        file_search_path = mx.FileSearchPath(search_path.asString())
        file_search_path.append(str(mtlx_file.parent.resolve()))
        
        image_handler = glsl_renderer.getImageHandler()
        image_handler.setSearchPath(file_search_path)
        
        # Find the specific material node
        render_node = None
        for elem in doc.getMaterialNodes():
            if elem.getName() == material_name:
                render_node = elem
                break
        
        assert render_node is not None, f"Material '{material_name}' not found in {mtlx_file}"
        
        # Render using shared logic
        result = render_material(
            glsl_renderer,
            doc,
            render_node,
            output_path=None  # Don't save images during test
        )
        
        # Assert success
        if not result.success:
            error_msg = result.error or result.shader_errors or "Unknown error"
            pytest.fail(f"Render failed for {material_name}: {error_msg}")


class TestRenderStdlibMaterials:
    """Test rendering of standard MaterialX library materials."""
    
    @pytest.mark.skip(reason="Stdlib material discovery not yet implemented")
    def test_render_stdlib_material(self):
        """Placeholder for stdlib material tests."""
        pass
