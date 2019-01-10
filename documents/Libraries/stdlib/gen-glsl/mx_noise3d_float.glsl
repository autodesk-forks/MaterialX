#include "stdlib/gen-glsl/lib/mx_noise.glsl"

void mx_noise3d_float(float amplitude, float pivot, vec3 position, out float result)
{
    float value = sx_perlin_noise_float(position);
    result = value * amplitude + pivot;
}
