void mx_ramptb_vector3(vec3 valuet, vec3 valueb, vec2 texcoord, out vec3 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix (valuet, valueb, clamp(uv.y, 0, 1) );
}
