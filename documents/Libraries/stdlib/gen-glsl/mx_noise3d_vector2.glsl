#include "stdlib/gen-glsl/lib/mx_noise.glsl"

void mx_noise3d_vector2(vec2 amplitude, float pivot, vec3 position, out vec2 result)
{
    vec3 value = sx_perlin_noise_vec3(position);
    result = value.xy * amplitude + pivot;
}
