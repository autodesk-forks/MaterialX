fn mx_splitlr_vector4_(valuel: vec4<f32>, valuer: vec4<f32>, center: f32, texcoord: vec2<f32>, result: ptr<function, vec4<f32>>) {
    (*result) = mix(valuel, valuer, vec4(mx_aastep(center, texcoord.x)));
    return;
}
