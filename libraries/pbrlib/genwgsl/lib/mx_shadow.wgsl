var<private> gl_FragCoord_1: vec4<f32>;

fn mx_variance_shadow_occlusion(moments: vec2<f32>, fragmentDepth: f32) -> f32 {
    var moments_1: vec2<f32>;
    var fragmentDepth_1: f32;
    var MIN_VARIANCE: f32 = 0.00001f;
    var local: f32;
    var p: f32;
    var variance: f32;
    var d: f32;
    var pMax: f32;

    moments_1 = moments;
    fragmentDepth_1 = fragmentDepth;
    let _e6 = fragmentDepth_1;
    let _e7 = moments_1;
    if (_e6 <= _e7.x) {
        local = 1f;
    } else {
        local = 0f;
    }
    let _e13 = local;
    p = _e13;
    let _e15 = moments_1;
    let _e17 = moments_1;
    let _e19 = mx_square(_e17.x);
    variance = (_e15.y - _e19);
    let _e22 = variance;
    let _e23 = MIN_VARIANCE;
    variance = max(_e22, _e23);
    let _e25 = fragmentDepth_1;
    let _e26 = moments_1;
    d = (_e25 - _e26.x);
    let _e30 = variance;
    let _e31 = variance;
    let _e32 = d;
    let _e33 = mx_square(_e32);
    pMax = (_e30 / (_e31 + _e33));
    let _e37 = p;
    let _e38 = pMax;
    return max(_e37, _e38);
}

fn mx_compute_depth_moments() -> vec2<f32> {
    var depth: f32;

    let _e1 = gl_FragCoord_1;
    depth = _e1.z;
    let _e4 = depth;
    let _e5 = depth;
    let _e6 = mx_square(_e5);
    return vec2<f32>(_e4, _e6);
}
