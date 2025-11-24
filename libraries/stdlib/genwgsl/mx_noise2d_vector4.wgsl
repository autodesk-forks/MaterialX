fn mx_noise2d_vector4_(amplitude_4: vec4<f32>, pivot: f32, texcoord: vec2<f32>, result_10: ptr<function, vec4<f32>>) {
    (*result_10) = (vec4<f32>(xyz).x, (xyz).y, (xyz).z, (w_2) * (amplitude_5) + vec4(pivot_1));
    return;
}
