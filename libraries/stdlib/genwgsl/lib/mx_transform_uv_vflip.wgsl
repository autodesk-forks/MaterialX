fn mx_transform_uv(uv: vec2<f32>, uv_scale: vec2<f32>, uv_offset: vec2<f32>) -> vec2<f32> {
    var uv_1: vec2<f32>;
    var uv_scale_1: vec2<f32>;
    var uv_offset_1: vec2<f32>;

    uv_1 = uv;
    uv_scale_1 = uv_scale;
    uv_offset_1 = uv_offset;
    let _e7 = uv_1;
    let _e8 = uv_scale_1;
    let _e10 = uv_offset_1;
    uv_1 = ((_e7 * _e8) + _e10);
    let _e12 = uv_1;
    let _e15 = uv_1;
    return vec2<f32>(_e12.x, (1f - _e15.y));
}