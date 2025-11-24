fn mx_luminance_color4_(_in: vec4<f32>, lumacoeffs: vec3<f32>, result: ptr<function, vec4<f32>>) {
    (*result) = vec4<f32>(vec3(dot(_in.xyz, lumacoeffs)), _in.w);
    return;
}
