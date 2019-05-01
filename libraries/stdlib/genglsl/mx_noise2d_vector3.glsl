#include "stdlib/genglsl/lib/mx_noise.glsl"

void mx_noise2d_vector3(vec3 amplitude, float pivot, vec2 texcoord, out vec3 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    vec3 value = mx_perlin_noise_vec3(uv);
    result = value * amplitude + pivot;
}
