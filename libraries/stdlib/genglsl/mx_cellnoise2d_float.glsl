#include "stdlib/genglsl/lib/mx_noise.glsl"

void mx_cellnoise2d_float(vec2 texcoord, out float result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mx_cell_noise_float(uv);
}
