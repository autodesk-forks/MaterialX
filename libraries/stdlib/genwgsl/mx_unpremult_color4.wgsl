fn mx_unpremult_color4_(_in: vec4<f32>, result: ptr<function, vec4<f32>>) {
    (*result) = vec4<f32>(_in.xyz / vec3(_in.w), _in.w);
    return;
}
