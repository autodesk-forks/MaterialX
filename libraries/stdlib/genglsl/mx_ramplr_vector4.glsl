void mx_ramplr_vector4(vec4 valuel, vec4 valuer, vec2 texcoord, out vec4 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix (valuel, valuer, clamp(uv.x, 0, 1) );
}
