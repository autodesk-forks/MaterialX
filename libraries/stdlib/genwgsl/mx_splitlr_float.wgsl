fn mx_splitlr_float(valuel: f32, valuer: f32, center: f32, texcoord: vec2<f32>, result: ptr<function, f32>) {
    (*result) = mix(valuel, valuer, mx_aastep(center, texcoord.x));
    return;
}
