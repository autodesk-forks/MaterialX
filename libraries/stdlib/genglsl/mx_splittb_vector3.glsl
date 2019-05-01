#include "stdlib/genglsl/mx_aastep.glsl"

void mx_splittb_vector3(vec3 valuet, vec3 valueb, float center, vec2 texcoord, out vec3 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix(valuet, valueb, mx_aastep(center, uv.y));
}
