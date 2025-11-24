fn mx_normalmap_vector2_(value: vec3<f32>, normal_scale: vec2<f32>, N: vec3<f32>, T: vec3<f32>, B: vec3<f32>, result: ptr<function, vec3<f32>>) {
    var local: vec3<f32>;
    if (dot(value, value) == 0f) {
        local = vec3<f32>(0f, 0f, 1f);
    } else {
        local = (value * 2f) - vec3(1f);
    }
    let transformed = ((T * local.x) * normal_scale.x) + ((B * local.y) * normal_scale.y) + (N * local.z);
    (*result) = normalize(transformed);
    return;
}

fn mx_normalmap_float(value_2: vec3<f32>, normal_scale_2: f32, N_2: vec3<f32>, T_2: vec3<f32>, B_2: vec3<f32>, result_1: ptr<function, vec3<f32>>) {
    mx_normalmap_vector2_(value_2, vec2(normal_scale_2), N_2, T_2, B_2, result_1);
    return;
}
