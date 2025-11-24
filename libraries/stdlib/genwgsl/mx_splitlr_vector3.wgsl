fn mx_splitlr_vector3_(valuel: vec3<f32>, valuer: vec3<f32>, center: f32, texcoord: vec2<f32>, result: ptr<function, vec3<f32>>) {
    (*result) = mix(valuel, valuer, vec3(mx_aastep(center, texcoord.x)));
    return;
}
