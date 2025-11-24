fn mx_ramplr_float(valuel: f32, valuer: f32, texcoord: vec2<f32>, result: ptr<function, f32>) {
    (*result) = mix(valuel, valuer, clamp(texcoord.x, 0f, 1f));
    return;
}
