fn mx_pow5_(x_6: f32) -> f32 {
    var x_7: f32;

    x_7 = x_6;
    let _e2 = x_7;
    let _e3 = mx_square(_e2);
    let _e4 = mx_square(_e3);
    let _e5 = x_7;
    return (_e4 * _e5);
}

fn mx_pow6_(x_8: f32) -> f32 {
    var x_9: f32;
    var x2_: f32;

    x_9 = x_8;
    let _e2 = x_9;
    let _e3 = mx_square(_e2);
    x2_ = _e3;
    let _e5 = x2_;
    let _e6 = mx_square(_e5);
    let _e7 = x2_;
    return (_e6 * _e7);
}

fn mx_fresnel_schlick(cosTheta: f32, F0_: f32) -> f32 {
    var cosTheta_1: f32;
    var F0_1: f32;
    var x_10: f32;
    var x5_: f32;

    cosTheta_1 = cosTheta;
    F0_1 = F0_;
    let _e5 = cosTheta_1;
    x_10 = clamp((1f - _e5), 0f, 1f);
    let _e11 = x_10;
    let _e12 = mx_pow5_(_e11);
    x5_ = _e12;
    let _e14 = F0_1;
    let _e16 = F0_1;
    let _e18 = x5_;
    return (_e14 + ((1f - _e16) * _e18));
}

fn mx_fresnel_schlick_1(cosTheta_2: f32, F0_2: vec3<f32>) -> vec3<f32> {
    var cosTheta_3: f32;
    var F0_3: vec3<f32>;
    var x_11: f32;
    var x5_1: f32;

    cosTheta_3 = cosTheta_2;
    F0_3 = F0_2;
    let _e5 = cosTheta_3;
    x_11 = clamp((1f - _e5), 0f, 1f);
    let _e11 = x_11;
    let _e12 = mx_pow5_(_e11);
    x5_1 = _e12;
    let _e14 = F0_3;
    let _e16 = F0_3;
    let _e19 = x5_1;
    return (_e14 + ((vec3(1f) - _e16) * _e19));
}

fn mx_fresnel_schlick_2(cosTheta_4: f32, F0_4: f32, F90_: f32) -> f32 {
    var cosTheta_5: f32;
    var F0_5: f32;
    var F90_1: f32;
    var x_12: f32;
    var x5_2: f32;

    cosTheta_5 = cosTheta_4;
    F0_5 = F0_4;
    F90_1 = F90_;
    let _e7 = cosTheta_5;
    x_12 = clamp((1f - _e7), 0f, 1f);
    let _e13 = x_12;
    let _e14 = mx_pow5_(_e13);
    x5_2 = _e14;
    let _e16 = F0_5;
    let _e17 = F90_1;
    let _e18 = x5_2;
    return mix(_e16, _e17, _e18);
}

fn mx_fresnel_schlick_3(cosTheta_6: f32, F0_6: vec3<f32>, F90_2: vec3<f32>) -> vec3<f32> {
    var cosTheta_7: f32;
    var F0_7: vec3<f32>;
    var F90_3: vec3<f32>;
    var x_13: f32;
    var x5_3: f32;

    cosTheta_7 = cosTheta_6;
    F0_7 = F0_6;
    F90_3 = F90_2;
    let _e7 = cosTheta_7;
    x_13 = clamp((1f - _e7), 0f, 1f);
    let _e13 = x_13;
    let _e14 = mx_pow5_(_e13);
    x5_3 = _e14;
    let _e16 = F0_7;
    let _e17 = F90_3;
    let _e18 = x5_3;
    return mix(_e16, _e17, vec3(_e18));
}

fn mx_fresnel_schlick_4(cosTheta_8: f32, F0_8: f32, F90_4: f32, exponent: f32) -> f32 {
    var cosTheta_9: f32;
    var F0_9: f32;
    var F90_5: f32;
    var exponent_1: f32;
    var x_14: f32;

    cosTheta_9 = cosTheta_8;
    F0_9 = F0_8;
    F90_5 = F90_4;
    exponent_1 = exponent;
    let _e9 = cosTheta_9;
    x_14 = clamp((1f - _e9), 0f, 1f);
    let _e15 = F0_9;
    let _e16 = F90_5;
    let _e17 = x_14;
    let _e18 = exponent_1;
    return mix(_e15, _e16, pow(_e17, _e18));
}

fn mx_fresnel_schlick_5(cosTheta_10: f32, F0_10: vec3<f32>, F90_6: vec3<f32>, exponent_2: f32) -> vec3<f32> {
    var cosTheta_11: f32;
    var F0_11: vec3<f32>;
    var F90_7: vec3<f32>;
    var exponent_3: f32;
    var x_15: f32;

    cosTheta_11 = cosTheta_10;
    F0_11 = F0_10;
    F90_7 = F90_6;
    exponent_3 = exponent_2;
    let _e9 = cosTheta_11;
    x_15 = clamp((1f - _e9), 0f, 1f);
    let _e15 = F0_11;
    let _e16 = F90_7;
    let _e17 = x_15;
    let _e18 = exponent_3;
    return mix(_e15, _e16, vec3(pow(_e17, _e18)));
}

fn mx_forward_facing_normal(N: vec3<f32>, V: vec3<f32>) -> vec3<f32> {
    var N_1: vec3<f32>;
    var V_1: vec3<f32>;
    var local: vec3<f32>;

    N_1 = N;
    V_1 = V;
    let _e4 = N_1;
    let _e5 = V_1;
    if (dot(_e4, _e5) < 0f) {
        let _e9 = N_1;
        local = -(_e9);
    } else {
        let _e11 = N_1;
        local = _e11;
    }
    let _e13 = local;
    return _e13;
}

fn mx_golden_ratio_sequence(i: i32) -> f32 {
    var i_1: i32;
    var GOLDEN_RATIO: f32 = 1.618034f;

    i_1 = i;
    let _e4 = i_1;
    let _e8 = GOLDEN_RATIO;
    return fract(((f32(_e4) + 1f) * _e8));
}

fn mx_spherical_fibonacci(i_2: i32, numSamples: i32) -> vec2<f32> {
    var i_3: i32;
    var numSamples_1: i32;

    i_3 = i_2;
    numSamples_1 = numSamples;
    let _e4 = i_3;
    let _e8 = numSamples_1;
    let _e11 = i_3;
    let _e12 = mx_golden_ratio_sequence(_e11);
    return vec2<f32>(((f32(_e4) + 0.5f) / f32(_e8)), _e12);
}

fn mx_uniform_sample_hemisphere(Xi: vec2<f32>) -> vec3<f32> {
    var Xi_1: vec2<f32>;
    var phi: f32;
    var cosTheta_12: f32;
    var sinTheta: f32;

    Xi_1 = Xi;
    let _e5 = Xi_1;
    phi = (6.2831855f * _e5.x);
    let _e10 = Xi_1;
    cosTheta_12 = (1f - _e10.y);
    let _e15 = cosTheta_12;
    let _e16 = mx_square(_e15);
    sinTheta = sqrt((1f - _e16));
    let _e20 = phi;
    let _e22 = sinTheta;
    let _e24 = phi;
    let _e26 = sinTheta;
    let _e28 = cosTheta_12;
    return vec3<f32>((cos(_e20) * _e22), (sin(_e24) * _e26), _e28);
}

fn mx_cosine_sample_hemisphere(Xi_2: vec2<f32>) -> vec3<f32> {
    var Xi_3: vec2<f32>;
    var phi_1: f32;
    var cosTheta_13: f32;
    var sinTheta_1: f32;

    Xi_3 = Xi_2;
    let _e5 = Xi_3;
    phi_1 = (6.2831855f * _e5.x);
    let _e9 = Xi_3;
    cosTheta_13 = sqrt(_e9.y);
    let _e14 = Xi_3;
    sinTheta_1 = sqrt((1f - _e14.y));
    let _e19 = phi_1;
    let _e21 = sinTheta_1;
    let _e23 = phi_1;
    let _e25 = sinTheta_1;
    let _e27 = cosTheta_13;
    return vec3<f32>((cos(_e19) * _e21), (sin(_e23) * _e25), _e27);
}

fn mx_orthonormal_basis(N_2: vec3<f32>) -> mat3x3<f32> {
    var N_3: vec3<f32>;
    var local_1: f32;
    var sign: f32;
    var a: f32;
    var b: f32;
    var X: vec3<f32>;
    var Y: vec3<f32>;

    N_3 = N_2;
    let _e2 = N_3;
    if (_e2.z < 0f) {
        local_1 = -1f;
    } else {
        local_1 = 1f;
    }
    let _e10 = local_1;
    sign = _e10;
    let _e14 = sign;
    let _e15 = N_3;
    a = (-1f / (_e14 + _e15.z));
    let _e20 = N_3;
    let _e22 = N_3;
    let _e25 = a;
    b = ((_e20.x * _e22.y) * _e25);
    let _e29 = sign;
    let _e30 = N_3;
    let _e33 = N_3;
    let _e36 = a;
    let _e39 = sign;
    let _e40 = b;
    let _e42 = sign;
    let _e44 = N_3;
    X = vec3<f32>((1f + (((_e29 * _e30.x) * _e33.x) * _e36)), (_e39 * _e40), (-(_e42) * _e44.x));
    let _e49 = b;
    let _e50 = sign;
    let _e51 = N_3;
    let _e53 = N_3;
    let _e56 = a;
    let _e59 = N_3;
    Y = vec3<f32>(_e49, (_e50 + ((_e51.y * _e53.y) * _e56)), -(_e59.y));
    let _e64 = X;
    let _e65 = Y;
    let _e66 = N_3;
    return mat3x3<f32>(vec3<f32>(_e64.x, _e64.y, _e64.z), vec3<f32>(_e65.x, _e65.y, _e65.z), vec3<f32>(_e66.x, _e66.y, _e66.z));
}
