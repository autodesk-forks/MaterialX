fn mx_luminance_color4_(_in: vec4<f32>, lumacoeffs: vec3<f32>, result: ptr<function, vec4<f32>>) {
    var _in_1: vec4<f32>;
    var lumacoeffs_1: vec3<f32>;

    _in_1 = _in;
    lumacoeffs_1 = lumacoeffs;
    let _e6 = _in_1;
    let _e8 = lumacoeffs_1;
    let _e10 = vec3(dot(_e6.xyz, _e8));
    let _e11 = _in_1;
    (*result) = vec4<f32>(_e10.x, _e10.y, _e10.z, _e11.w);
    return;
}