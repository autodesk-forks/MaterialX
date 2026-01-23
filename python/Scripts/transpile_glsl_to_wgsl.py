#!/usr/bin/env python
"""
Transpile GLSL shader files to WGSL using naga.

This script:
1. Finds all .glsl files in specified library folders
2. Wraps each file with a dummy main function for naga transpilation
3. Transpiles GLSL to WGSL using naga-cli
4. Removes the dummy main function from the output
5. Saves the result to genwgsl folders mirroring the genglsl structure
"""

import os
import sys
import subprocess
import shutil
import tempfile
import re
from pathlib import Path

# Base library path - relative to script location
SCRIPT_DIR = Path(__file__).parent.resolve()
LIBRARIES_BASE = SCRIPT_DIR.parent.parent / "libraries"

# Folders to process (relative to libraries/)
FOLDERS_TO_PROCESS = [
    "stdlib/genglsl",
    "pbrlib/genglsl",
    "lights/genglsl",
    "bxdf/genglsl",
    "nprlib/genglsl",
    # cmlib doesn't have genglsl folder based on directory listing
]

def find_naga():
    """Find naga-cli executable."""
    # Try common locations
    naga_commands = ["naga", "naga-cli"]
    
    for cmd in naga_commands:
        try:
            result = subprocess.run(
                [cmd, "--version"],
                capture_output=True,
                text=True,
                timeout=5
            )
            if result.returncode == 0 or "naga" in result.stderr.lower() or "naga" in result.stdout.lower():
                return cmd
        except (FileNotFoundError, subprocess.TimeoutExpired):
            continue
    
    return None

def apply_token_substitutions(glsl_content):
    """
    Apply MaterialX token substitutions.
    Common tokens that need to be replaced for transpilation.
    """
    # Common MaterialX token substitutions
    # These are defaults - in real MaterialX these are context-dependent
    token_subs = {
        '$fileTransformUv': 'mx_transform_uv.glsl',  # Default to non-flipped version
        '$texSamplerSignature': 'sampler2D tex_sampler',  # Will be replaced, handling comma separately
        '$texSamplerSampler2D': 'tex_sampler',
        '$closureDataConstructor': 'ClosureData(closureType, L, V, N, P, occlusion)',
        '$albedoTable': 'sampler2D albedo_table',
        '$envRadianceSamples': '16',  # Default sample count
        '$envRadianceMips': '10',  # Default mip count
        '$envMatrix': 'mat4(1.0)',  # Identity matrix
        '$envRadiance': 'sampler2D env_radiance',
        '$envIrradiance': 'sampler2D env_irradiance',
        '$envLightIntensity': '1.0',  # Default intensity
    }
    
    result = glsl_content
    for token, replacement in token_subs.items():
        # Special handling for $texSamplerSignature - if it's on its own line with a comma, preserve indentation
        if token == '$texSamplerSignature':
            # Pattern: line with just the token and a comma (preserve indentation)
            # Match: whitespace, token, optional whitespace, comma, optional whitespace, newline
            result = re.sub(r'^(\s*)\$texSamplerSignature\s*,\s*$', r'\1' + replacement + ',', result, flags=re.MULTILINE)
            # Pattern: line with just the token (no comma) - but followed by comma on next line or in function params
            result = re.sub(r'^(\s*)\$texSamplerSignature\s*$', r'\1' + replacement, result, flags=re.MULTILINE)
            # Pattern: token in middle of line
            result = result.replace(token, replacement)
        else:
            result = result.replace(token, replacement)
    
    return result

def find_include_file(include_path, file_path):
    """
    Find the actual include file path given an include path and the current file.
    Returns the Path to the include file, or None if not found.
    """
    # Apply token substitutions to include path
    include_path = apply_token_substitutions(include_path)
    
    # Resolve the include path relative to the current file
    file_dir = Path(file_path).parent
    include_file = file_dir / include_path
    
    # Try relative to file directory first
    if not include_file.exists():
        # Try relative to genglsl directory
        genglsl_dir = file_dir
        while genglsl_dir.name != 'genglsl' and genglsl_dir.parent != genglsl_dir:
            genglsl_dir = genglsl_dir.parent
        if genglsl_dir.name == 'genglsl':
            include_file = genglsl_dir / include_path
    
    # Try absolute path from libraries
    if not include_file.exists():
        include_file = LIBRARIES_BASE / include_path
    
    if include_file.exists():
        return include_file
    return None

def resolve_includes(glsl_content, file_path, for_transpilation=True, included_files=None):
    """
    Resolve #include directives.
    
    If for_transpilation=True: Inline GLSL content (needed for naga to see all functions)
    If for_transpilation=False: Replace with markers for WGSL inlining later
    
    included_files: Set of already-included file paths to prevent duplicates across recursive calls.
    
    Returns the GLSL content with includes resolved.
    """
    # Apply token substitutions first (before any processing)
    glsl_content = apply_token_substitutions(glsl_content)
    
    if not for_transpilation:
        # Just replace with markers
        lines = glsl_content.split('\n')
        resolved_lines = []
        for line in lines:
            stripped = line.strip()
            if stripped.startswith('#include'):
                match = re.search(r'#include\s+"([^"]+)"', stripped)
                if match:
                    include_path = match.group(1)
                    resolved_lines.append(f"// WGSL_INCLUDE: {include_path}")
                else:
                    resolved_lines.append(line)
            else:
                resolved_lines.append(line)
        return '\n'.join(resolved_lines)
    
    # For transpilation: inline GLSL content
    # Use shared included_files set to prevent duplicates across recursive calls
    if included_files is None:
        included_files = set()
    
    lines = glsl_content.split('\n')
    resolved_lines = []
    
    for line in lines:
        stripped = line.strip()
        if stripped.startswith('#include'):
            # Extract the include path
            # Format: #include "path/to/file.glsl"
            match = re.search(r'#include\s+"([^"]+)"', stripped)
            if match:
                include_path = match.group(1)
                
                # Find the include file
                include_file = find_include_file(include_path, file_path)
                
                if include_file and include_file not in included_files:
                    included_files.add(include_file)
                    try:
                        with open(include_file, 'r', encoding='utf-8') as f:
                            included_content = f.read()
                        
                        # Recursively resolve includes in the included file
                        # Pass the same included_files set to prevent duplicates across include chains
                        resolved_included = resolve_includes(included_content, include_file, for_transpilation=True, included_files=included_files)
                        # Add marker so we can replace with WGSL later
                        resolved_lines.append(f"// WGSL_INCLUDE_START: {include_path}")
                        resolved_lines.extend(resolved_included.split('\n'))
                        resolved_lines.append(f"// WGSL_INCLUDE_END: {include_path}")
                    except Exception as e:
                        # If we can't read the include, keep the original line
                        resolved_lines.append(line)
                        print(f"    Warning: Could not resolve include '{include_path}': {e}")
                elif include_file in included_files:
                    # Already included, skip to avoid duplication
                    pass
                else:
                    resolved_lines.append(line)
                    print(f"    Warning: Include file not found: {include_path}")
            else:
                # Malformed include, keep original line
                resolved_lines.append(line)
        else:
            resolved_lines.append(line)
    
    return '\n'.join(resolved_lines)

def wrap_with_dummy_main(glsl_content, file_path=None):
    """
    Wrap GLSL content with a dummy main function.
    This is needed because naga requires a complete shader with an entry point.
    """
    # Apply token substitutions first (before any other processing)
    glsl_content = apply_token_substitutions(glsl_content)
    
    # Resolve includes first if file_path is provided
    # This must happen BEFORE we check what functions are defined
    if file_path and "#include" in glsl_content:
        glsl_content = resolve_includes(glsl_content, file_path)
    
    # Determine shader stage based on content
    # For fragment shaders, we'll use a simple main that doesn't interfere
    # For vertex shaders, we'll use a vertex main
    
    # Check if it looks like a vertex shader (has position, etc.)
    is_vertex = any(keyword in glsl_content for keyword in [
        "gl_Position", "gl_VertexID", "gl_InstanceID"
    ])
    
    # Check if #version already exists
    has_version = "#version" in glsl_content
    
    # Check if we need MaterialX type definitions
    # Check the RESOLVED content (after includes) to see what's already defined
    needs_bsdf = 'BSDF' in glsl_content and 'struct BSDF' not in glsl_content
    needs_edf = 'EDF' in glsl_content
    needs_displacement = 'displacementshader' in glsl_content
    needs_surface = 'surfaceshader' in glsl_content
    # Always add math functions if needed
    needs_math = 'mx_square' in glsl_content or 'mx_matrix_mul' in glsl_content or 'mx_cos' in glsl_content or 'mx_sin' in glsl_content
    needs_environment = 'mx_environment_irradiance' in glsl_content or 'mx_environment_radiance' in glsl_content
    needs_shadow = 'mx_variance_shadow_occlusion' in glsl_content
    # Check if pow functions are already defined in the RESOLVED content (after includes)
    # Count occurrences - if it appears, it's already defined
    pow5_def_count = len(re.findall(r'float\s+mx_pow5\s*\(', glsl_content))
    pow6_def_count = len(re.findall(r'float\s+mx_pow6\s*\(', glsl_content))
    has_pow5_defined = pow5_def_count > 0
    has_pow6_defined = pow6_def_count > 0
    # Only add if functions are used but not yet defined
    needs_pow5 = 'mx_pow5(' in glsl_content and not has_pow5_defined
    needs_pow6 = 'mx_pow6(' in glsl_content and not has_pow6_defined
    
    # Build MaterialX type definitions
    materialx_types = ""
    if needs_bsdf:
        materialx_types += """
struct BSDF {
    vec3 response;
    vec3 throughput;
};
"""
    if needs_edf:
        materialx_types += """
#define EDF vec3
"""
    if needs_displacement:
        materialx_types += """
struct displacementshader {
    vec3 offset;
    float scale;
};
"""
    if needs_surface:
        materialx_types += """
struct surfaceshader {
    vec3 color;
    vec3 transparency;
};
"""
    
    # Add math functions if needed
    math_functions = ""
    if needs_math:
        math_functions = """
// MaterialX math function aliases (from mx_math.glsl)
#define mx_mod mod
#define mx_inverse inverse
#define mx_inversesqrt inversesqrt
#define mx_sin sin
#define mx_cos cos
#define mx_tan tan
#define mx_asin asin
#define mx_acos acos
#define mx_atan atan
#define mx_radians radians
#define mx_float_bits_to_int floatBitsToInt

// MaterialX math functions
vec2 mx_matrix_mul(vec2 v, mat2 m) { return v * m; }
vec3 mx_matrix_mul(vec3 v, mat3 m) { return v * m; }
vec4 mx_matrix_mul(vec4 v, mat4 m) { return v * m; }
vec2 mx_matrix_mul(mat2 m, vec2 v) { return m * v; }
vec3 mx_matrix_mul(mat3 m, vec3 v) { return m * v; }
vec4 mx_matrix_mul(mat4 m, vec4 v) { return m * v; }
mat2 mx_matrix_mul(mat2 m1, mat2 m2) { return m1 * m2; }
mat3 mx_matrix_mul(mat3 m1, mat3 m2) { return m1 * m2; }
mat4 mx_matrix_mul(mat4 m1, mat4 m2) { return m1 * m2; }

float mx_square(float x) { return x*x; }
vec2 mx_square(vec2 x) { return x*x; }
vec3 mx_square(vec3 x) { return x*x; }
"""
    
    # Add pow functions separately if needed (and not already defined)
    pow_functions = ""
    if needs_pow5:
        pow_functions += """
// MaterialX microfacet utility functions
float mx_pow5(float x) { return mx_square(mx_square(x)) * x; }
"""
    if needs_pow6:
        pow_functions += """
float mx_pow6(float x) { float x2 = mx_square(x); return mx_square(x2) * x2; }
"""
    
    # Add stub functions for MaterialX environment functions if needed
    environment_functions = ""
    if needs_environment:
        environment_functions = """
// MaterialX environment function stubs
vec3 mx_environment_irradiance(vec3 N) { return vec3(0.0); }
vec3 mx_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 roughness, int distribution, vec3 ior) { return vec3(0.0); }
"""
    
    # Add stub functions for shadow functions if needed
    shadow_functions = ""
    if needs_shadow:
        shadow_functions = """
// MaterialX shadow function stub
float mx_variance_shadow_occlusion(vec2 moments, float depth) {
    float variance = moments.y - mx_square(moments.x);
    float d = depth - moments.x;
    float pMax = variance / (variance + mx_square(d));
    return clamp((depth <= moments.x ? 1.0 : pMax), 0.0, 1.0);
}
"""
    
    # Add common constants (always add M_PI and M_PI_INV if they might be needed)
    constants = ""
    if 'M_PI' in glsl_content or 'M_PI_INV' in glsl_content or 'pbrlib' in str(file_path) if file_path else False:
        constants = """
#define M_PI 3.14159265358979323846
#define M_PI_INV (1.0 / M_PI)
"""
    if 'AIRY_FRESNEL_ITERATIONS' in glsl_content:
        constants += """
#define AIRY_FRESNEL_ITERATIONS 10
"""
    
    # Build the shader content
    if has_version:
        # If version exists, keep it and add wrapper after
        # Add common MaterialX constants after version
        common_defines = """
#define M_FLOAT_EPS 1e-8
""" + constants + materialx_types + math_functions + environment_functions + shadow_functions
        if is_vertex:
            wrapper = common_defines + """
void main() {
    // Dummy vertex shader entry point
    gl_Position = vec4(0.0);
}
"""
        else:
            wrapper = common_defines + """
out vec4 fragColor;

void main() {
    // Dummy fragment shader entry point
    fragColor = vec4(0.0);
}
"""
        return glsl_content + wrapper
    else:
        # No version directive, add it first
        # Add common MaterialX constants that might be used
        common_defines = """#version 450

#define M_FLOAT_EPS 1e-8
""" + constants + materialx_types + math_functions + pow_functions + environment_functions + shadow_functions
        
        if is_vertex:
            return common_defines + glsl_content + """

void main() {
    // Dummy vertex shader entry point
    gl_Position = vec4(0.0);
}
"""
        else:
            return common_defines + """
out vec4 fragColor;

""" + glsl_content + """

void main() {
    // Dummy fragment shader entry point
    fragColor = vec4(0.0);
}
"""

def remove_dummy_main(wgsl_content):
    """
    Remove the dummy main function and all related entry point artifacts from WGSL output.
    This includes main functions, helper functions like main_1, FragmentOutput structs,
    and fragColor variables that are only used by the dummy entry point.
    """
    # Use regex to remove all main functions and helper functions first
    # This is more reliable than line-by-line parsing
    
    # Remove main_1() helper functions
    wgsl_content = re.sub(
        r'fn\s+main_1\([^)]*\)\s*\{[^}]*\}',
        '',
        wgsl_content,
        flags=re.MULTILINE | re.DOTALL
    )
    
    # Remove @fragment/@vertex/@compute fn main() functions
    # Match from @attribute to end of function
    wgsl_content = re.sub(
        r'@(vertex|fragment|compute)\s*\n\s*fn\s+main\([^)]*\)\s*->\s*[^{]*\{[^}]*\}',
        '',
        wgsl_content,
        flags=re.MULTILINE | re.DOTALL
    )
    
    # Also match if @attribute and fn main are on same line
    wgsl_content = re.sub(
        r'@(vertex|fragment|compute)\s+fn\s+main\([^)]*\)\s*->\s*[^{]*\{[^}]*\}',
        '',
        wgsl_content,
        flags=re.MULTILINE | re.DOTALL
    )
    
    # Now check if FragmentOutput is still used
    # Count references to FragmentOutput
    fragment_output_refs = len(re.findall(r'\bFragmentOutput\b', wgsl_content))
    
    # If FragmentOutput is only in its struct definition (1 reference), remove it
    if fragment_output_refs <= 1:
        wgsl_content = re.sub(
            r'struct\s+FragmentOutput\s*\{[^}]*\}',
            '',
            wgsl_content,
            flags=re.MULTILINE
        )
    
    # Check if fragColor is still used
    fragcolor_refs = len(re.findall(r'\bfragColor\b', wgsl_content))
    
    # If fragColor is only in its variable declaration (1 reference), remove it
    if fragcolor_refs <= 1:
        wgsl_content = re.sub(
            r'var<private>\s+fragColor[^;]*;',
            '',
            wgsl_content,
            flags=re.MULTILINE
        )
    
    # Clean up extra blank lines (more than 2 consecutive)
    wgsl_content = re.sub(r'\n\s*\n\s*\n+', '\n\n', wgsl_content)
    
    # Remove leading blank lines
    wgsl_content = re.sub(r'^\s*\n+', '', wgsl_content)
    
    # Remove trailing blank lines
    wgsl_content = wgsl_content.rstrip()
    
    return wgsl_content

def extract_main_function_name(glsl_file_path):
    """
    Extract the main function name from a GLSL file.
    Returns the function name (e.g., 'mx_noise2d_float') or None.
    """
    try:
        with open(glsl_file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Look for function definitions (void function_name(...) or type function_name(...))
        # Skip includes and comments
        lines = content.split('\n')
        for line in lines:
            stripped = line.strip()
            if stripped.startswith('//') or stripped.startswith('#include') or not stripped:
                continue
            
            # Match function definitions: void name(...) or type name(...)
            match = re.search(r'(void|\w+)\s+(\w+)\s*\(', stripped)
            if match:
                func_name = match.group(2)
                # Skip common GLSL built-ins and our dummy main
                if func_name not in ['main', 'mx_select', 'mx_floor', 'mx_bilerp']:
                    return func_name
    except:
        pass
    return None

def get_functions_from_wgsl(wgsl_content):
    """
    Extract all function names from WGSL content.
    Returns a set of function names.
    """
    functions = set()
    # Match: fn function_name(...)
    matches = re.findall(r'fn\s+(\w+)\s*\(', wgsl_content)
    functions.update(matches)
    return functions

def resolve_wgsl_includes(wgsl_content, original_glsl_file, naga_cmd):
    """
    Replace inlined include content with separately-transpiled WGSL includes.
    
    Strategy:
    1. Identify the main function from the original GLSL file
    2. Find functions that belong to includes (by checking the include WGSL files)
    3. Replace those functions with the WGSL content from the include file
    """
    # Read original GLSL to find what includes it has
    try:
        with open(original_glsl_file, 'r', encoding='utf-8') as f:
            original_glsl = f.read()
    except:
        return wgsl_content
    
    # Find include paths
    include_paths = re.findall(r'#include\s+"([^"]+)"', original_glsl)
    if not include_paths:
        return wgsl_content
    
    # Get the main function name from the original file
    main_func_name = extract_main_function_name(original_glsl_file)
    if not main_func_name:
        return wgsl_content
    
    # Get all functions in the current WGSL
    all_functions = get_functions_from_wgsl(wgsl_content)
    
    # For each include, find its functions and replace them
    original_dir = Path(original_glsl_file).parent
    genglsl_dir = original_dir
    while genglsl_dir.name != 'genglsl' and genglsl_dir.parent != genglsl_dir:
        genglsl_dir = genglsl_dir.parent
    
    if genglsl_dir.name != 'genglsl':
        return wgsl_content
    
    genwgsl_dir = genglsl_dir.parent / "genwgsl"
    result_lines = []
    result_lines.append(f"// Main function: {main_func_name}")
    
    # Process each include
    for include_path in include_paths:
        include_glsl_file = find_include_file(include_path, original_glsl_file)
        if include_glsl_file:
            include_wgsl_file = genwgsl_dir / include_glsl_file.relative_to(genglsl_dir).with_suffix('.wgsl')
            
            if include_wgsl_file.exists():
                try:
                    with open(include_wgsl_file, 'r', encoding='utf-8') as f:
                        include_wgsl = f.read().strip()
                    
                    # Get functions from the include
                    include_functions = get_functions_from_wgsl(include_wgsl)
                    
                    # Add include comment and content
                    result_lines.append(f"// Included from {include_path} (WGSL)")
                    result_lines.append(include_wgsl)
                except Exception as e:
                    result_lines.append(f"// Error reading include {include_path}: {e}")
    
    # Now extract only the main function from the original WGSL
    # Find the main function definition - try both with and without underscore suffix
    lines = wgsl_content.split('\n')
    in_main_func = False
    brace_count = 0
    main_func_lines = []
    
    # Try to find the main function (may have _ suffix from naga)
    for i, line in enumerate(lines):
        # Check for function definition: fn main_func_name(...) or fn main_func_name_(...)
        if re.search(rf'fn\s+{re.escape(main_func_name)}[_\w]*\s*\(', line):
            in_main_func = True
            main_func_lines = [line]
            brace_count = line.count('{') - line.count('}')
            continue
        
        if in_main_func:
            main_func_lines.append(line)
            brace_count += line.count('{') - line.count('}')
            if brace_count == 0 and len(main_func_lines) > 1:
                # Found the end of the function
                result_lines.append("")
                result_lines.append(f"// Main function from {Path(original_glsl_file).name}")
                result_lines.extend(main_func_lines)
                break
    
    if not main_func_lines:
        # Couldn't find main function, include all content but prepend includes
        result_lines.append("")
        result_lines.append("// Original content (could not extract main function):")
        result_lines.append(wgsl_content)
    
    return '\n'.join(result_lines)

def transpile_glsl_to_wgsl(glsl_file_path, naga_cmd):
    """
    Transpile a single GLSL file to WGSL.
    
    Returns the WGSL content as a string, or None if transpilation failed.
    """
    try:
        # Read the GLSL file
        with open(glsl_file_path, 'r', encoding='utf-8') as f:
            glsl_content = f.read()
        
        # Wrap with dummy main (includes will be resolved inside)
        wrapped_glsl = wrap_with_dummy_main(glsl_content, str(glsl_file_path))
        
        # Check if wrapping failed
        if wrapped_glsl is None:
            return None
        
        # Determine shader stage for file extension
        is_vertex = any(keyword in wrapped_glsl for keyword in [
            "gl_Position", "gl_VertexID", "gl_InstanceID"
        ])
        file_ext = ".vert" if is_vertex else ".frag"
        
        # Create temporary files with appropriate extensions
        # naga needs the extension to determine the shader stage
        with tempfile.NamedTemporaryFile(mode='w', suffix=file_ext, delete=False) as temp_glsl:
            temp_glsl.write(wrapped_glsl)
            temp_glsl_path = temp_glsl.name
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.wgsl', delete=False) as temp_wgsl:
            temp_wgsl_path = temp_wgsl.name
        
        try:
            # Run naga to transpile
            # naga-cli syntax: naga <input> <output> --input-kind glsl --shader-stage <stage>
            # Stage values: 'vert', 'frag', 'compute'
            stage_value = "vert" if is_vertex else "frag"
            result = subprocess.run(
                [naga_cmd, temp_glsl_path, temp_wgsl_path, 
                 "--input-kind", "glsl", 
                 "--shader-stage", stage_value],
                capture_output=True,
                text=True,
                timeout=30
            )
            
            if result.returncode != 0:
                print(f"Error transpiling {glsl_file_path}:")
                print(result.stderr)
                return None
            
            # Read the WGSL output
            with open(temp_wgsl_path, 'r', encoding='utf-8') as f:
                wgsl_content = f.read()
            
            # Remove dummy main function
            wgsl_content = remove_dummy_main(wgsl_content)
            
            # Resolve WGSL includes (replace include comments with actual WGSL content)
            wgsl_content = resolve_wgsl_includes(wgsl_content, glsl_file_path, naga_cmd)
            
            return wgsl_content
            
        finally:
            # Clean up temp files
            try:
                os.unlink(temp_glsl_path)
                os.unlink(temp_wgsl_path)
            except:
                pass
                
    except Exception as e:
        print(f"Error processing {glsl_file_path}: {e}")
        return None

def process_folder(folder_path, naga_cmd):
    """
    Process all .glsl files in a folder and create corresponding .wgsl files
    in a genwgsl folder mirroring the structure.
    
    Uses a two-pass approach:
    1. First pass: Transpile all include files (lib/*.glsl) and standalone files
    2. Second pass: Transpile files with includes, replacing inlined content with WGSL includes
    """
    folder_path = Path(folder_path)
    
    if not folder_path.exists():
        print(f"Folder does not exist: {folder_path}")
        return
    
    # Find all .glsl files recursively
    glsl_files = list(folder_path.rglob("*.glsl"))
    
    if not glsl_files:
        print(f"No .glsl files found in {folder_path}")
        return
    
    print(f"\nProcessing {len(glsl_files)} files in {folder_path}...")
    
    # Create genwgsl folder structure
    # Replace genglsl with genwgsl in the path
    genwgsl_path = folder_path.parent / "genwgsl"
    
    # Separate files into two groups:
    # 1. Include files (lib/*.glsl) and files without includes - transpile first
    # 2. Files with includes - transpile second
    include_files = []
    files_with_includes = []
    standalone_files = []
    
    for glsl_file in glsl_files:
        with open(glsl_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Check if it's in a lib folder (these are typically includes)
        if 'lib' in glsl_file.parts:
            include_files.append(glsl_file)
        elif '#include' in content:
            files_with_includes.append(glsl_file)
        else:
            standalone_files.append(glsl_file)
    
    # First pass: Transpile include files and standalone files
    print(f"  Pass 1: Transpiling {len(include_files)} include files and {len(standalone_files)} standalone files...")
    first_pass_files = include_files + standalone_files
    
    success_count = 0
    error_count = 0
    
    for glsl_file in first_pass_files:
        # Calculate relative path from folder_path
        rel_path = glsl_file.relative_to(folder_path)
        
        # Create corresponding genwgsl path
        wgsl_file = genwgsl_path / rel_path.with_suffix('.wgsl')
        
        # Create parent directories
        wgsl_file.parent.mkdir(parents=True, exist_ok=True)
        
        # Transpile
        print(f"  Transpiling: {rel_path}")
        wgsl_content = transpile_glsl_to_wgsl(glsl_file, naga_cmd)
        
        if wgsl_content:
            # Write WGSL file
            with open(wgsl_file, 'w', encoding='utf-8') as f:
                f.write(wgsl_content)
            print(f"    -> {wgsl_file.relative_to(LIBRARIES_BASE)}")
            success_count += 1
        else:
            print(f"    ERROR: Failed to transpile")
            error_count += 1
    
    print(f"\n  Pass 1 Summary: {success_count} succeeded, {error_count} failed")
    
    # Second pass: Transpile files with includes
    if files_with_includes:
        print(f"  Pass 2: Transpiling {len(files_with_includes)} files with includes...")
        pass2_success = 0
        pass2_error = 0
        
        for glsl_file in files_with_includes:
            # Calculate relative path from folder_path
            rel_path = glsl_file.relative_to(folder_path)
            
            # Create corresponding genwgsl path
            wgsl_file = genwgsl_path / rel_path.with_suffix('.wgsl')
            
            # Create parent directories
            wgsl_file.parent.mkdir(parents=True, exist_ok=True)
            
            # Transpile
            print(f"  Transpiling: {rel_path}")
            wgsl_content = transpile_glsl_to_wgsl(glsl_file, naga_cmd)
            
            if wgsl_content:
                # Write WGSL file
                with open(wgsl_file, 'w', encoding='utf-8') as f:
                    f.write(wgsl_content)
                print(f"    -> {wgsl_file.relative_to(LIBRARIES_BASE)}")
                pass2_success += 1
            else:
                print(f"    ERROR: Failed to transpile")
                pass2_error += 1
        
        success_count += pass2_success
        error_count += pass2_error
        print(f"\n  Pass 2 Summary: {pass2_success} succeeded, {pass2_error} failed")
    
    print(f"\n  Total Summary: {success_count} succeeded, {error_count} failed")

def main():
    """Main entry point."""
    print("GLSL to WGSL Transpiler")
    print("=" * 50)
    
    # Find naga
    naga_cmd = find_naga()
    if not naga_cmd:
        print("ERROR: naga-cli not found!")
        print("\nPlease install naga-cli:")
        print("  cargo install naga-cli")
        print("\nOr download from: https://github.com/gfx-rs/naga")
        sys.exit(1)
    
    print(f"Using naga: {naga_cmd}")
    
    # Process each folder
    for folder_name in FOLDERS_TO_PROCESS:
        folder_path = LIBRARIES_BASE / folder_name
        if folder_path.exists():
            process_folder(folder_path, naga_cmd)
        else:
            print(f"\nSkipping {folder_name} (folder does not exist)")
    
    print("\n" + "=" * 50)
    print("Transpilation complete!")

if __name__ == "__main__":
    main()
