void mx_ramplr_vector3(vec3 valuel, vec3 valuer, vec2 texcoord, out vec3 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    result = mix (valuel, valuer, clamp(uv.x, 0, 1) );
}
