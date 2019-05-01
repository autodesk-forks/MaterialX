void mx_ramptb_vector2(vec2 valuet, vec2 valueb, vec2 texcoord, out vec2 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix (valuet, valueb, clamp(uv.y, 0, 1) );
}
