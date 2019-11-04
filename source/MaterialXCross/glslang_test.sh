#!/bin/bash

GLSLANG=../../../SPIRV-Cross/external/glslang-build/install/bin/glslangValidator
$GLSLANG -H -S frag --entry-point main --keep-uncalled -g ../../libraries/pbrlib/genglsl/lib/mx_math.glsl
