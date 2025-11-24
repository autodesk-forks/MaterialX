fn mx_luminance_color3_(_in: vec3<f32>, lumacoeffs: vec3<f32>, result: ptr<function, vec3<f32>>) {
    (*result) = vec3(dot(_in, lumacoeffs));
    return;
}
