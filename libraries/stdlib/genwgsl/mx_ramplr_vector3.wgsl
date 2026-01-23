fn mx_ramplr_vector3_(valuel: vec3<f32>, valuer: vec3<f32>, texcoord: vec2<f32>, result: ptr<function, vec3<f32>>) {
    var valuel_1: vec3<f32>;
    var valuer_1: vec3<f32>;
    var texcoord_1: vec2<f32>;

    valuel_1 = valuel;
    valuer_1 = valuer;
    texcoord_1 = texcoord;
    let _e8 = valuel_1;
    let _e9 = valuer_1;
    let _e10 = texcoord_1;
    (*result) = mix(_e8, _e9, vec3(clamp(_e10.x, 0f, 1f)));
    return;
}