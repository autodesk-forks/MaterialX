#include "stdlib/genglsl/lib/mx_noise.glsl"

void mx_noise2d_vector2(vec2 amplitude, float pivot, vec2 texcoord, out vec2 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    vec3 value = mx_perlin_noise_vec3(uv);
    result = value.xy * amplitude + pivot;
}
