void mx_ramplr_float(float valuel, float valuer, vec2 texcoord, out float result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix (valuel, valuer, clamp(uv.x, 0, 1) );
}
