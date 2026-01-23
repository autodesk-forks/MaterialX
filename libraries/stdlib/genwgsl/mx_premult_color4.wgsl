fn mx_premult_color4_(_in: vec4<f32>, result: ptr<function, vec4<f32>>) {
    var _in_1: vec4<f32>;

    _in_1 = _in;
    let _e4 = _in_1;
    let _e6 = _in_1;
    let _e8 = (_e4.xyz * _e6.w);
    let _e9 = _in_1;
    (*result) = vec4<f32>(_e8.x, _e8.y, _e8.z, _e9.w);
    return;
}