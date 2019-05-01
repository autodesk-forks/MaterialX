void mx_ramp4_vector2(vec2 valuetl, vec2 valuetr, vec2 valuebl, vec2 valuebr, vec2 texcoord, out vec2 result)
{
    vec2 uv = mx_get_target_uv(texcoord);
    float ss = clamp(uv.x, 0, 1);
    float tt = clamp(uv.y, 0, 1);
    result = mix(mix(valuetl, valuetr, ss),
                 mix(valuebl, valuebr, ss),
                 tt);
}
