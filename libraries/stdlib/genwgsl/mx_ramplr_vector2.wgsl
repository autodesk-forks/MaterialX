fn mx_ramplr_vector2_(valuel: vec2<f32>, valuer: vec2<f32>, texcoord: vec2<f32>, result: ptr<function, vec2<f32>>) {
    (*result) = mix(valuel, valuer, vec2(clamp(texcoord.x, 0f, 1f)));
    return;
}
