#include "stdlib/genglsl/mx_aastep.glsl"

void mx_splittb_vector4(vec4 valuet, vec4 valueb, float center, vec2 texcoord, out vec4 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix(valuet, valueb, mx_aastep(center, uv.y));
}
