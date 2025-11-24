fn mx_premult_color4_(_in: vec4<f32>, result: ptr<function, vec4<f32>>) {
    (*result) = vec4<f32>(_in.xyz * _in.w, _in.w);
    return;
}
