#include "stdlib/genglsl/lib/mx_noise.glsl"

void mx_noise2d_fa_vector4(float amplitude, float pivot, vec2 texcoord, out vec4 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    vec3 xyz = mx_perlin_noise_vec3(uv);
    float w = mx_perlin_noise_float(uv + vec2(19, 73));
    result = vec4(xyz, w) * amplitude + pivot;
}
