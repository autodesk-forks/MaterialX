fn mx_normalmap_vector2_(value: vec3<f32>, normal_scale: vec2<f32>, N: vec3<f32>, T: vec3<f32>, B: vec3<f32>, result: ptr<function, vec3<f32>>) {
    var value_1: vec3<f32>;
    var normal_scale_1: vec2<f32>;
    var N_1: vec3<f32>;
    var T_1: vec3<f32>;
    var B_1: vec3<f32>;
    var local: vec3<f32>;

    value_1 = value;
    normal_scale_1 = normal_scale;
    N_1 = N;
    T_1 = T;
    B_1 = B;
    let _e12 = value_1;
    let _e13 = value_1;
    if (dot(_e12, _e13) == 0f) {
        local = vec3<f32>(0f, 0f, 1f);
    } else {
        let _e21 = value_1;
        local = ((_e21 * 2f) - vec3(1f));
    }
    let _e28 = local;
    value_1 = _e28;
    let _e29 = T_1;
    let _e30 = value_1;
    let _e33 = normal_scale_1;
    let _e36 = B_1;
    let _e37 = value_1;
    let _e40 = normal_scale_1;
    let _e44 = N_1;
    let _e45 = value_1;
    value_1 = ((((_e29 * _e30.x) * _e33.x) + ((_e36 * _e37.y) * _e40.y)) + (_e44 * _e45.z));
    let _e49 = value_1;
    (*result) = normalize(_e49);
    return;
}

fn mx_normalmap_float(value_2: vec3<f32>, normal_scale_2: f32, N_2: vec3<f32>, T_2: vec3<f32>, B_2: vec3<f32>, result_1: ptr<function, vec3<f32>>) {
    var value_3: vec3<f32>;
    var normal_scale_3: f32;
    var N_3: vec3<f32>;
    var T_3: vec3<f32>;
    var B_3: vec3<f32>;

    value_3 = value_2;
    normal_scale_3 = normal_scale_2;
    N_3 = N_2;
    T_3 = T_2;
    B_3 = B_2;
    let _e12 = value_3;
    let _e13 = normal_scale_3;
    let _e15 = N_3;
    let _e16 = T_3;
    let _e17 = B_3;
    mx_normalmap_vector2_(_e12, vec2(_e13), _e15, _e16, _e17, result_1);
    return;
}