fn mx_ramplr_vector4_(valuel: vec4<f32>, valuer: vec4<f32>, texcoord: vec2<f32>, result: ptr<function, vec4<f32>>) {
    (*result) = mix(valuel, valuer, vec4(clamp(texcoord.x, 0f, 1f)));
    return;
}
