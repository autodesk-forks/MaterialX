#!/usr/bin/env python3
"""
Convert GLSL files to WGSL format for MaterialX stdlib
"""
import os
import re
from pathlib import Path

def convert_glsl_to_wgsl(glsl_code):
    """Convert GLSL code to WGSL"""
    wgsl_code = glsl_code
    
    # Type conversions
    wgsl_code = re.sub(r'\bvec2\b', 'vec2f', wgsl_code)
    wgsl_code = re.sub(r'\bvec3\b', 'vec3f', wgsl_code)
    wgsl_code = re.sub(r'\bvec4\b', 'vec4f', wgsl_code)
    wgsl_code = re.sub(r'\bivec2\b', 'vec2i', wgsl_code)
    wgsl_code = re.sub(r'\bivec3\b', 'vec3i', wgsl_code)
    wgsl_code = re.sub(r'\bivec4\b', 'vec4i', wgsl_code)
    wgsl_code = re.sub(r'\buvec2\b', 'vec2u', wgsl_code)
    wgsl_code = re.sub(r'\buvec3\b', 'vec3u', wgsl_code)
    wgsl_code = re.sub(r'\buvec4\b', 'vec4u', wgsl_code)
    wgsl_code = re.sub(r'\bbvec3\b', 'vec3<bool>', wgsl_code)
    wgsl_code = re.sub(r'\bbvec2\b', 'vec2<bool>', wgsl_code)
    wgsl_code = re.sub(r'\bbvec4\b', 'vec4<bool>', wgsl_code)
    
    # Matrix type conversions
    wgsl_code = re.sub(r'\bmat2\b', 'mat2x2f', wgsl_code)
    wgsl_code = re.sub(r'\bmat3\b', 'mat3x3f', wgsl_code)
    wgsl_code = re.sub(r'\bmat4\b', 'mat4x4f', wgsl_code)
    
    # Replace #define with const
    wgsl_code = re.sub(r'#define\s+(\w+)\s+(.+)', r'const \1 = \2;', wgsl_code)
    
    # Handle texture sampling - basic replacement, may need manual adjustment
    wgsl_code = re.sub(r'texture\(\$texSamplerSampler2D,\s*([^)]+)\)', r'textureSample($texSamplerSampler2D, $texSampler, \1)', wgsl_code)
    
    # Handle #include statements - convert to comment for now
    wgsl_code = re.sub(r'#include\s+"([^"]+)"', r'// include "\1"', wgsl_code)
    
    # Handle derivative functions (these are similar in WGSL)
    # dFdx, dFdy, fwidth are available in WGSL
    
    # Handle out parameters - add comment for manual review
    # WGSL doesn't support out parameters the same way, functions should return values
    
    # Integer type suffixes
    wgsl_code = re.sub(r'\b(\d+)u\b', r'\1u', wgsl_code)
    
    # Float suffixes - WGSL uses 'f' like GLSL
    
    return wgsl_code

def process_file(input_file, output_file):
    """Process a single GLSL file and convert to WGSL"""
    print(f"Converting {input_file} -> {output_file}")
    
    with open(input_file, 'r') as f:
        glsl_code = f.read()
    
    wgsl_code = convert_glsl_to_wgsl(glsl_code)
    
    # Ensure output directory exists
    output_file.parent.mkdir(parents=True, exist_ok=True)
    
    with open(output_file, 'w') as f:
        f.write(wgsl_code)

def main():
    """Main conversion process"""
    script_dir = Path(__file__).parent
    genglsl_dir = script_dir / "genglsl"
    genwgsl_dir = script_dir / "genwgsl"
    
    if not genglsl_dir.exists():
        print(f"Error: {genglsl_dir} not found")
        return
    
    # Find all .glsl files
    glsl_files = list(genglsl_dir.rglob("*.glsl"))
    
    print(f"Found {len(glsl_files)} GLSL files to convert")
    
    for glsl_file in glsl_files:
        # Calculate relative path and create corresponding .wgsl file
        relative_path = glsl_file.relative_to(genglsl_dir)
        wgsl_file = genwgsl_dir / relative_path.with_suffix('.wgsl')
        
        process_file(glsl_file, wgsl_file)
    
    print(f"\nConversion complete! Converted {len(glsl_files)} files.")
    print(f"Output directory: {genwgsl_dir}")

if __name__ == "__main__":
    main()

