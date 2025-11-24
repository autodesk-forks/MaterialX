fn mx_transform_uv(uv: vec2<f32>, uv_scale: vec2<f32>, uv_offset: vec2<f32>) -> vec2<f32> {
    var uv_1: vec2<f32>;
    var uv_scale_1: vec2<f32>;
    var uv_offset_1: vec2<f32>;

    uv_1 = uv;
    uv_scale_1 = uv_scale;
    uv_offset_1 = uv_offset;
    let _e6 = uv_1;
    let _e7 = uv_scale_1;
    let _e9 = uv_offset_1;
    uv_1 = ((_e6 * _e7) + _e9);
    let _e11 = uv_1;
    let _e14 = uv_1;
    return vec2<f32>(_e11.x, (1f - _e14.y));
}
