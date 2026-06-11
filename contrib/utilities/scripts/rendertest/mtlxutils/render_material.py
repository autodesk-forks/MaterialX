"""
Shared render logic for pytest test suite.

This module encapsulates the per-material render logic so it can be called from
pytest test cases (parametrized tests).
"""
import MaterialX as mx
from pathlib import Path
from dataclasses import dataclass
from typing import Optional, List


@dataclass
class RenderResult:
    """Result of rendering a single material."""
    success: bool
    material_name: str
    output_path: Optional[Path] = None
    error: Optional[str] = None
    shader_errors: Optional[str] = None


def find_renderable_materials(doc) -> List:
    """Find all renderable elements in a document."""
    from mtlxutils import mxshadergen
    gen = mxshadergen.MtlxShaderGen(doc)
    gen.setup()
    return gen.findRenderableElements(doc)


def render_material(
    renderer,
    doc,
    render_node,
    output_path: Optional[Path] = None,
    search_path=None,
    target_colorspace: str = 'lin_rec709',
    target_distance_unit: str = 'centimeter'
) -> RenderResult:
    """
    Render a single material node.
    
    Args:
        renderer: Initialized GlslRenderer instance
        doc: MaterialX document containing the material
        render_node: The renderable node to render
        output_path: Directory to save output image (optional)
        search_path: MaterialX search path for source code 
            and images (optional)
        target_colorspace: Target colorspace override
        target_distance_unit: Target distance unit
        
    Returns:
        RenderResult with success status and any errors
    """
    material_name = render_node.getNamePath()
    
    # Register search path for source code includes and images
    # (mirrors performRender in mxrenderer.py)
    if search_path is not None:
        generator = renderer.getCodeGenerator()
        generator.registerSourceCodeSearchPath(search_path)
        image_handler = renderer.getImageHandler()
        image_handler.setSearchPath(search_path)
    
    # Handle material nodes that wrap surface shaders
    # getShaderNodes only works on Node objects, not Outputs
    if isinstance(render_node, mx.Node) and render_node.getType() == 'material':
        shader_nodes = mx.getShaderNodes(render_node)
        if not shader_nodes:
            return RenderResult(
                success=False,
                material_name=material_name,
                error=f"No surface shader found in material: {material_name}"
            )
    
    # Generate shader
    shader = renderer.generateShader(render_node, target_colorspace, target_distance_unit)
    if not shader:
        return RenderResult(
            success=False,
            material_name=material_name,
            shader_errors=renderer.getActiveShaderErrors()
        )
    
    # Create program
    if not renderer.createProgram():
        return RenderResult(
            success=False,
            material_name=material_name,
            error="Failed to create GPU program"
        )
    
    # Render
    rendered, errors = renderer.render()
    if not rendered:
        return RenderResult(
            success=False,
            material_name=material_name,
            error=str(errors)
        )
    
    # Capture and optionally save
    renderer.captureImage()
    
    result = RenderResult(
        success=True,
        material_name=material_name
    )
    
    if output_path:
        context = renderer.getCodeGenerator().getContext()
        target = context.getShaderGenerator().getTarget()
        # Map target generator names to match ASWF MaterialXTest's suffix conventions (e.g., genglsl -> glsl)
        suffix = target.removeprefix("gen") if target else target
        output_file = output_path / f"{mx.createValidName(material_name)}_{suffix}.png"
        renderer.saveCapture(str(output_file), True)
        result.output_path = output_file
    
    return result
