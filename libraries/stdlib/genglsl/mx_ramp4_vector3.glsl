void mx_ramp4_vector3(vec3 valuetl, vec3 valuetr, vec3 valuebl, vec3 valuebr, vec2 texcoord, out vec3 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    float ss = clamp(uv.x, 0, 1);
    float tt = clamp(uv.y, 0, 1);
    result = mix(mix(valuetl, valuetr, ss),
                 mix(valuebl, valuebr, ss),
                 tt);
}
