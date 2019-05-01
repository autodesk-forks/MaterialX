void mx_ramplr_vector2(vec2 valuel, vec2 valuer, vec2 texcoord, out vec2 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix (valuel, valuer, clamp(uv.x, 0, 1) );
}
