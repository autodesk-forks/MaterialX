#include "stdlib/genglsl/mx_aastep.glsl"

void mx_splittb_float(float valuet, float valueb, float center, vec2 texcoord, out float result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix(valuet, valueb, mx_aastep(center, uv.y));
}
