fn mx_unpremult_color4_(_in: vec4<f32>, result: ptr<function, vec4<f32>>) {
    var _in_1: vec4<f32>;

    _in_1 = _in;
    let _e4 = _in_1;
    let _e6 = _in_1;
    let _e9 = (_e4.xyz / vec3(_e6.w));
    let _e10 = _in_1;
    (*result) = vec4<f32>(_e9.x, _e9.y, _e9.z, _e10.w);
    return;
}