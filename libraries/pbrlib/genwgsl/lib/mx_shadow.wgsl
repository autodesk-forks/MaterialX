struct BSDF {
    response: vec3<f32>,
    throughput: vec3<f32>,
}

struct FragmentOutput {
    @location(0) fragColor: vec4<f32>,
}

var<private> fragColor: vec4<f32>;
var<private> gl_FragCoord_1: vec4<f32>;

fn mx_matrix_mul(v: vec2<f32>, m: mat2x2<f32>) -> vec2<f32> {
    var v_1: vec2<f32>;
    var m_1: mat2x2<f32>;

    v_1 = v;
    m_1 = m;
    let _e4 = v_1;
    let _e5 = m_1;
    return (_e4 * _e5);
}

fn mx_matrix_mul_1(v_2: vec3<f32>, m_2: mat3x3<f32>) -> vec3<f32> {
    var v_3: vec3<f32>;
    var m_3: mat3x3<f32>;

    v_3 = v_2;
    m_3 = m_2;
    let _e4 = v_3;
    let _e5 = m_3;
    return (_e4 * _e5);
}

fn mx_matrix_mul_2(v_4: vec4<f32>, m_4: mat4x4<f32>) -> vec4<f32> {
    var v_5: vec4<f32>;
    var m_5: mat4x4<f32>;

    v_5 = v_4;
    m_5 = m_4;
    let _e4 = v_5;
    let _e5 = m_5;
    return (_e4 * _e5);
}

fn mx_matrix_mul_3(m_6: mat2x2<f32>, v_6: vec2<f32>) -> vec2<f32> {
    var m_7: mat2x2<f32>;
    var v_7: vec2<f32>;

    m_7 = m_6;
    v_7 = v_6;
    let _e4 = m_7;
    let _e5 = v_7;
    return (_e4 * _e5);
}

fn mx_matrix_mul_4(m_8: mat3x3<f32>, v_8: vec3<f32>) -> vec3<f32> {
    var m_9: mat3x3<f32>;
    var v_9: vec3<f32>;

    m_9 = m_8;
    v_9 = v_8;
    let _e4 = m_9;
    let _e5 = v_9;
    return (_e4 * _e5);
}

fn mx_matrix_mul_5(m_10: mat4x4<f32>, v_10: vec4<f32>) -> vec4<f32> {
    var m_11: mat4x4<f32>;
    var v_11: vec4<f32>;

    m_11 = m_10;
    v_11 = v_10;
    let _e4 = m_11;
    let _e5 = v_11;
    return (_e4 * _e5);
}

fn mx_matrix_mul_6(m1_: mat2x2<f32>, m2_: mat2x2<f32>) -> mat2x2<f32> {
    var m1_1: mat2x2<f32>;
    var m2_1: mat2x2<f32>;

    m1_1 = m1_;
    m2_1 = m2_;
    let _e4 = m1_1;
    let _e5 = m2_1;
    return (_e4 * _e5);
}

fn mx_matrix_mul_7(m1_2: mat3x3<f32>, m2_2: mat3x3<f32>) -> mat3x3<f32> {
    var m1_3: mat3x3<f32>;
    var m2_3: mat3x3<f32>;

    m1_3 = m1_2;
    m2_3 = m2_2;
    let _e4 = m1_3;
    let _e5 = m2_3;
    return (_e4 * _e5);
}

fn mx_matrix_mul_8(m1_4: mat4x4<f32>, m2_4: mat4x4<f32>) -> mat4x4<f32> {
    var m1_5: mat4x4<f32>;
    var m2_5: mat4x4<f32>;

    m1_5 = m1_4;
    m2_5 = m2_4;
    let _e4 = m1_5;
    let _e5 = m2_5;
    return (_e4 * _e5);
}

fn mx_square(x: f32) -> f32 {
    var x_1: f32;

    x_1 = x;
    let _e2 = x_1;
    let _e3 = x_1;
    return (_e2 * _e3);
}

fn mx_square_1(x_2: vec2<f32>) -> vec2<f32> {
    var x_3: vec2<f32>;

    x_3 = x_2;
    let _e2 = x_3;
    let _e3 = x_3;
    return (_e2 * _e3);
}

fn mx_square_2(x_4: vec3<f32>) -> vec3<f32> {
    var x_5: vec3<f32>;

    x_5 = x_4;
    let _e2 = x_5;
    let _e3 = x_5;
    return (_e2 * _e3);
}

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
    let _e7 = fragmentDepth_1;
    let _e8 = moments_1;
    if (_e7 <= _e8.x) {
        local = 1f;
    } else {
        local = 0f;
    }
    let _e14 = local;
    p = _e14;
    let _e16 = moments_1;
    let _e18 = moments_1;
    let _e20 = mx_square(_e18.x);
    variance = (_e16.y - _e20);
    let _e23 = variance;
    let _e24 = MIN_VARIANCE;
    variance = max(_e23, _e24);
    let _e26 = fragmentDepth_1;
    let _e27 = moments_1;
    d = (_e26 - _e27.x);
    let _e31 = variance;
    let _e32 = variance;
    let _e33 = d;
    let _e34 = mx_square(_e33);
    pMax = (_e31 / (_e32 + _e34));
    let _e38 = p;
    let _e39 = pMax;
    return max(_e38, _e39);
}

fn mx_compute_depth_moments() -> vec2<f32> {
    var depth: f32;

    let _e2 = gl_FragCoord_1;
    depth = _e2.z;
    let _e5 = depth;
    let _e6 = depth;
    let _e7 = mx_square(_e6);
    return vec2<f32>(_e5, _e7);
}

@fragment 
fn main(@builtin(position) gl_FragCoord: vec4<f32>) -> FragmentOutput {
    gl_FragCoord_1 = gl_FragCoord;
    main_1();
    let _e5 = fragColor;
    return FragmentOutput(_e5);
}