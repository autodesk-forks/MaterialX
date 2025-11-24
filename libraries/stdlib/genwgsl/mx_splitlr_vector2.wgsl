fn mx_splitlr_vector2_(valuel: vec2<f32>, valuer: vec2<f32>, center: f32, texcoord: vec2<f32>, result: ptr<function, vec2<f32>>) {
    (*result) = mix(valuel, valuer, vec2(mx_aastep(center, texcoord.x)));
    return;
}
