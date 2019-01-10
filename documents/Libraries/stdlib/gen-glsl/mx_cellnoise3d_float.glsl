#include "stdlib/gsn-glsl/lib/mx_noise.glsl"

void mx_cellnoise3d_float(vec3 position, out float result)
{
    result = sx_cell_noise_float(position);
}
