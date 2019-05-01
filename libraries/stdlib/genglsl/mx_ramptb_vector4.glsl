void mx_ramptb_vector4(vec4 valuet, vec4 valueb, vec2 texcoord, out vec4 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix (valuet, valueb, clamp(uv.y, 0, 1) );
}
