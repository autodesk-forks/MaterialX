#include "stdlib/genglsl/mx_aastep.glsl"

void mx_splitlr_vector4(vec4 valuel, vec4 valuer, float center, vec2 texcoord, out vec4 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix(valuel, valuer, mx_aastep(center, uv.x));
}
