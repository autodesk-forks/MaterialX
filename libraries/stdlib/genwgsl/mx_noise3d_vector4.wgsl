fn mx_noise3d_vector4_(amplitude_4: vec4<f32>, pivot: f32, position: vec3<f32>, result_10: ptr<function, vec4<f32>>) {
    (*result_10) = (vec4<f32>(xyz).x, (xyz).y, (xyz).z, (w_2) * (amplitude_5) + vec4(pivot_1));
    return;
}
