"""
Shared render logic for both script and pytest usage.

This module encapsulates the per-material render logic so it can be called from:
1. renderDocuments.py (batch script)
2. pytest test cases (parametrized tests)
"""
import MaterialX as mx
from pathlib import Path
from dataclasses import dataclass
from typing import Optional, List, Tuple


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
        target_colorspace: Target colorspace override
        target_distance_unit: Target distance unit
        
    Returns:
        RenderResult with success status and any errors
    """
    material_name = render_node.getNamePath()
    
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
        output_file = output_path / f"{mx.createValidName(material_name)}_{target}.png"
        renderer.saveCapture(str(output_file), True)
        result.output_path = output_file
    
    return result


def render_file(
    renderer,
    mtlx_file: Path,
    libraries: List,
    search_path,
    output_path: Optional[Path] = None
) -> List[RenderResult]:
    """
    Render all materials in a single .mtlx file.
    
    Args:
        renderer: Initialized GlslRenderer instance
        mtlx_file: Path to the .mtlx file
        libraries: List of loaded library documents
        search_path: MaterialX search path
        output_path: Directory to save output images (optional)
        
    Returns:
        List of RenderResult for each material in the file
    """
    results = []
    
    # Create working document with libraries
    doc = mx.createDocument()
    for lib in libraries:
        doc.importLibrary(lib)
    
    # Load the material file
    try:
        mx.readFromXmlFile(doc, str(mtlx_file))
        valid, msg = doc.validate()
        if not valid:
            return [RenderResult(
                success=False,
                material_name=str(mtlx_file),
                error=f"Validation failed: {msg}"
            )]
    except mx.Exception as err:
        return [RenderResult(
            success=False,
            material_name=str(mtlx_file),
            error=str(err)
        )]
    
    # Update search path with file's directory
    file_dir = mtlx_file.parent.resolve()
    full_search_path = mx.FileSearchPath(search_path)
    full_search_path.append(str(file_dir))
    
    # Update image handler search path
    image_handler = renderer.getImageHandler()
    image_handler.setSearchPath(full_search_path)
    
    # Find and render all materials
    render_nodes = renderer.findRenderableElements(doc)
    for node in render_nodes:
        result = render_material(renderer, doc, node, output_path)
        results.append(result)
    
    return results
