void mx_ramptb_float(float valuet, float valueb, vec2 texcoord, out float result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix (valuet, valueb, clamp(uv.y, 0, 1) );
}
