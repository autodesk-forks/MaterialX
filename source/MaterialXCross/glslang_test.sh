#!/bin/bash

GLSLANG=../../../SPIRV-Cross/external/glslang-build/install/bin/glslangValidator
$GLSLANG -H -i -S frag -Od --entry-point main --keep-uncalled -g ../../libraries/pbrlib/genglsl/lib/mx_math.glsl || exit 1
SPIRV_CROSS=../../../build/SPIRV-Cross/Debug/Debug/spirv-cross.exe
$SPIRV_CROSS --hlsl --entry main --export-functions --output frag.hlsl frag.spv || exit 2

