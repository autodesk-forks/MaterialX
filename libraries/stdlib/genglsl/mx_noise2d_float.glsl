#include "stdlib/genglsl/lib/mx_noise.glsl"

void mx_noise2d_float(float amplitude, float pivot, vec2 texcoord, out float result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    float value = mx_perlin_noise_float(uv);
    result = value * amplitude + pivot;
}
