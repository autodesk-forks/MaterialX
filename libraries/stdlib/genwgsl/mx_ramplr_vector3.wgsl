fn mx_ramplr_vector3_(valuel: vec3<f32>, valuer: vec3<f32>, texcoord: vec2<f32>, result: ptr<function, vec3<f32>>) {
    (*result) = mix(valuel, valuer, vec3(clamp(texcoord.x, 0f, 1f)));
    return;
}
