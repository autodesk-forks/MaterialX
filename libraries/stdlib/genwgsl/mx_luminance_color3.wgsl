fn mx_luminance_color3_(_in: vec3<f32>, lumacoeffs: vec3<f32>, result: ptr<function, vec3<f32>>) {
    var _in_1: vec3<f32>;
    var lumacoeffs_1: vec3<f32>;

    _in_1 = _in;
    lumacoeffs_1 = lumacoeffs;
    let _e6 = _in_1;
    let _e7 = lumacoeffs_1;
    (*result) = vec3(dot(_e6, _e7));
    return;
}