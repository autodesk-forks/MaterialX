// Main function: mx_sheen_bsdf
// Included from lib/mx_closure_type.glsl (WGSL)
struct ClosureData {
    closureType: i32,
    L: vec3<f32>,
    V: vec3<f32>,
    N: vec3<f32>,
    P: vec3<f32>,
    occlusion: f32,
}

fn makeClosureData(closureType: i32, L: vec3<f32>, V: vec3<f32>, N: vec3<f32>, P: vec3<f32>, occlusion: f32) -> ClosureData {
    var closureType_1: i32;
    var L_1: vec3<f32>;
    var V_1: vec3<f32>;
    var N_1: vec3<f32>;
    var P_1: vec3<f32>;
    var occlusion_1: f32;

    closureType_1 = closureType;
    L_1 = L;
    V_1 = V;
    N_1 = N;
    P_1 = P;
    occlusion_1 = occlusion;
    let _e13 = closureType_1;
    let _e14 = L_1;
    let _e15 = V_1;
    let _e16 = N_1;
    let _e17 = P_1;
    let _e18 = occlusion_1;
    return ClosureData(_e13, _e14, _e15, _e16, _e17, _e18);
}
// Included from lib/mx_microfacet_sheen.glsl (WGSL)
// Main function: mx_imageworks_sheen_NDF
// Included from mx_microfacet.glsl (WGSL)
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

fn mx_pow5_(x_6: f32) -> f32 {
    var x_7: f32;

    x_7 = x_6;
    let _e3 = x_7;
    let _e4 = mx_square(_e3);
    let _e5 = mx_square(_e4);
    let _e6 = x_7;
    return (_e5 * _e6);
}

fn mx_pow6_(x_8: f32) -> f32 {
    var x_9: f32;
    var x2_: f32;

    x_9 = x_8;
    let _e3 = x_9;
    let _e4 = mx_square(_e3);
    x2_ = _e4;
    let _e6 = x2_;
    let _e7 = mx_square(_e6);
    let _e8 = x2_;
    return (_e7 * _e8);
}

fn mx_fresnel_schlick(cosTheta: f32, F0_: f32) -> f32 {
    var cosTheta_1: f32;
    var F0_1: f32;
    var x_10: f32;
    var x5_: f32;

    cosTheta_1 = cosTheta;
    F0_1 = F0_;
    let _e6 = cosTheta_1;
    x_10 = clamp((1f - _e6), 0f, 1f);
    let _e12 = x_10;
    let _e13 = mx_pow5_(_e12);
    x5_ = _e13;
    let _e15 = F0_1;
    let _e17 = F0_1;
    let _e19 = x5_;
    return (_e15 + ((1f - _e17) * _e19));
}

fn mx_fresnel_schlick_1(cosTheta_2: f32, F0_2: vec3<f32>) -> vec3<f32> {
    var cosTheta_3: f32;
    var F0_3: vec3<f32>;
    var x_11: f32;
    var x5_1: f32;

    cosTheta_3 = cosTheta_2;
    F0_3 = F0_2;
    let _e6 = cosTheta_3;
    x_11 = clamp((1f - _e6), 0f, 1f);
    let _e12 = x_11;
    let _e13 = mx_pow5_(_e12);
    x5_1 = _e13;
    let _e15 = F0_3;
    let _e17 = F0_3;
    let _e20 = x5_1;
    return (_e15 + ((vec3(1f) - _e17) * _e20));
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
    let _e8 = cosTheta_5;
    x_12 = clamp((1f - _e8), 0f, 1f);
    let _e14 = x_12;
    let _e15 = mx_pow5_(_e14);
    x5_2 = _e15;
    let _e17 = F0_5;
    let _e18 = F90_1;
    let _e19 = x5_2;
    return mix(_e17, _e18, _e19);
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
    let _e8 = cosTheta_7;
    x_13 = clamp((1f - _e8), 0f, 1f);
    let _e14 = x_13;
    let _e15 = mx_pow5_(_e14);
    x5_3 = _e15;
    let _e17 = F0_7;
    let _e18 = F90_3;
    let _e19 = x5_3;
    return mix(_e17, _e18, vec3(_e19));
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
    let _e10 = cosTheta_9;
    x_14 = clamp((1f - _e10), 0f, 1f);
    let _e16 = F0_9;
    let _e17 = F90_5;
    let _e18 = x_14;
    let _e19 = exponent_1;
    return mix(_e16, _e17, pow(_e18, _e19));
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
    let _e10 = cosTheta_11;
    x_15 = clamp((1f - _e10), 0f, 1f);
    let _e16 = F0_11;
    let _e17 = F90_7;
    let _e18 = x_15;
    let _e19 = exponent_3;
    return mix(_e16, _e17, vec3(pow(_e18, _e19)));
}

fn mx_forward_facing_normal(N: vec3<f32>, V: vec3<f32>) -> vec3<f32> {
    var N_1: vec3<f32>;
    var V_1: vec3<f32>;
    var local: vec3<f32>;

    N_1 = N;
    V_1 = V;
    let _e5 = N_1;
    let _e6 = V_1;
    if (dot(_e5, _e6) < 0f) {
        let _e10 = N_1;
        local = -(_e10);
    } else {
        let _e12 = N_1;
        local = _e12;
    }
    let _e14 = local;
    return _e14;
}

fn mx_golden_ratio_sequence(i: i32) -> f32 {
    var i_1: i32;
    var GOLDEN_RATIO: f32 = 1.618034f;

    i_1 = i;
    let _e5 = i_1;
    let _e9 = GOLDEN_RATIO;
    return fract(((f32(_e5) + 1f) * _e9));
}

fn mx_spherical_fibonacci(i_2: i32, numSamples: i32) -> vec2<f32> {
    var i_3: i32;
    var numSamples_1: i32;

    i_3 = i_2;
    numSamples_1 = numSamples;
    let _e5 = i_3;
    let _e9 = numSamples_1;
    let _e12 = i_3;
    let _e13 = mx_golden_ratio_sequence(_e12);
    return vec2<f32>(((f32(_e5) + 0.5f) / f32(_e9)), _e13);
}

fn mx_uniform_sample_hemisphere(Xi: vec2<f32>) -> vec3<f32> {
    var Xi_1: vec2<f32>;
    var phi: f32;
    var cosTheta_12: f32;
    var sinTheta: f32;

    Xi_1 = Xi;
    let _e6 = Xi_1;
    phi = (6.2831855f * _e6.x);
    let _e11 = Xi_1;
    cosTheta_12 = (1f - _e11.y);
    let _e16 = cosTheta_12;
    let _e17 = mx_square(_e16);
    sinTheta = sqrt((1f - _e17));
    let _e21 = phi;
    let _e23 = sinTheta;
    let _e25 = phi;
    let _e27 = sinTheta;
    let _e29 = cosTheta_12;
    return vec3<f32>((cos(_e21) * _e23), (sin(_e25) * _e27), _e29);
}

fn mx_cosine_sample_hemisphere(Xi_2: vec2<f32>) -> vec3<f32> {
    var Xi_3: vec2<f32>;
    var phi_1: f32;
    var cosTheta_13: f32;
    var sinTheta_1: f32;

    Xi_3 = Xi_2;
    let _e6 = Xi_3;
    phi_1 = (6.2831855f * _e6.x);
    let _e10 = Xi_3;
    cosTheta_13 = sqrt(_e10.y);
    let _e15 = Xi_3;
    sinTheta_1 = sqrt((1f - _e15.y));
    let _e20 = phi_1;
    let _e22 = sinTheta_1;
    let _e24 = phi_1;
    let _e26 = sinTheta_1;
    let _e28 = cosTheta_13;
    return vec3<f32>((cos(_e20) * _e22), (sin(_e24) * _e26), _e28);
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
    let _e3 = N_3;
    if (_e3.z < 0f) {
        local_1 = -1f;
    } else {
        local_1 = 1f;
    }
    let _e11 = local_1;
    sign = _e11;
    let _e15 = sign;
    let _e16 = N_3;
    a = (-1f / (_e15 + _e16.z));
    let _e21 = N_3;
    let _e23 = N_3;
    let _e26 = a;
    b = ((_e21.x * _e23.y) * _e26);
    let _e30 = sign;
    let _e31 = N_3;
    let _e34 = N_3;
    let _e37 = a;
    let _e40 = sign;
    let _e41 = b;
    let _e43 = sign;
    let _e45 = N_3;
    X = vec3<f32>((1f + (((_e30 * _e31.x) * _e34.x) * _e37)), (_e40 * _e41), (-(_e43) * _e45.x));
    let _e50 = b;
    let _e51 = sign;
    let _e52 = N_3;
    let _e54 = N_3;
    let _e57 = a;
    let _e60 = N_3;
    Y = vec3<f32>(_e50, (_e51 + ((_e52.y * _e54.y) * _e57)), -(_e60.y));
    let _e65 = X;
    let _e66 = Y;
    let _e67 = N_3;
    return mat3x3<f32>(vec3<f32>(_e65.x, _e65.y, _e65.z), vec3<f32>(_e66.x, _e66.y, _e66.z), vec3<f32>(_e67.x, _e67.y, _e67.z));
}

// Main function from mx_microfacet_sheen.glsl
fn mx_imageworks_sheen_NDF(NdotH: f32, roughness: f32) -> f32 {
    var NdotH_1: f32;
    var roughness_1: f32;
    var invRoughness: f32;
    var cos2_: f32;
    var sin2_: f32;

    NdotH_1 = NdotH;
    roughness_1 = roughness;
    let _e6 = roughness_1;
    invRoughness = (1f / max(_e6, 0.005f));
    let _e11 = NdotH_1;
    let _e12 = NdotH_1;
    cos2_ = (_e11 * _e12);
    let _e16 = cos2_;
    sin2_ = (1f - _e16);
    let _e20 = invRoughness;
    let _e22 = sin2_;
    let _e23 = invRoughness;
    return (((2f + _e20) * pow(_e22, (_e23 * 0.5f))) / 6.2831855f);
}

// Main function from mx_sheen_bsdf.glsl
fn mx_sheen_bsdf(closureData: ClosureData, weight: f32, color: vec3<f32>, roughness_18: f32, N_16: vec3<f32>, mode: i32, bsdf: ptr<function, BSDF>) {
    var closureData_1: ClosureData;
    var weight_1: f32;
    var color_1: vec3<f32>;
    var roughness_19: f32;
    var N_17: vec3<f32>;
    var mode_1: i32;
    var V_13: vec3<f32>;
    var L_5: vec3<f32>;
    var NdotV_15: f32;
    var dirAlbedo_1: f32;
    var H_1: vec3<f32>;
    var NdotL_3: f32;
    var NdotH_5: f32;
    var fr: vec3<f32>;
    var fr_1: vec3<f32>;
    var dirAlbedo_2: f32;
    var Li: vec3<f32>;

    closureData_1 = closureData;
    weight_1 = weight;
    color_1 = color;
    roughness_19 = roughness_18;
    N_17 = N_16;
    mode_1 = mode;
    let _e14 = weight_1;
    if (_e14 < 0.00000001f) {
        {
            return;
        }
    }
    let _e17 = closureData_1;
    V_13 = _e17.V;
    let _e20 = closureData_1;
    L_5 = _e20.L;
    let _e23 = N_17;
    let _e24 = V_13;
    let _e25 = mx_forward_facing_normal(_e23, _e24);
    N_17 = _e25;
    let _e26 = N_17;
    let _e27 = V_13;
    NdotV_15 = clamp(dot(_e26, _e27), 0.00000001f, 1f);
    let _e33 = closureData_1;
    if (_e33.closureType == 1i) {
        {
            let _e38 = mode_1;
            if (_e38 == 0i) {
                {
                    let _e41 = L_5;
                    let _e42 = V_13;
                    H_1 = normalize((_e41 + _e42));
                    let _e46 = N_17;
                    let _e47 = L_5;
                    NdotL_3 = clamp(dot(_e46, _e47), 0.00000001f, 1f);
                    let _e53 = N_17;
                    let _e54 = H_1;
                    NdotH_5 = clamp(dot(_e53, _e54), 0.00000001f, 1f);
                    let _e60 = color_1;
                    let _e61 = NdotL_3;
                    let _e62 = NdotV_15;
                    let _e63 = NdotH_5;
                    let _e64 = roughness_19;
                    let _e65 = mx_imageworks_sheen_brdf(_e61, _e62, _e63, _e64);
                    fr = (_e60 * _e65);
                    let _e68 = NdotV_15;
                    let _e69 = roughness_19;
                    let _e70 = mx_imageworks_sheen_dir_albedo(_e68, _e69);
                    dirAlbedo_1 = _e70;
                    let _e72 = fr;
                    let _e73 = NdotL_3;
                    let _e75 = closureData_1;
                    let _e78 = weight_1;
                    (*bsdf).response = (((_e72 * _e73) * _e75.occlusion) * _e78);
                }
            } else {
                {
                    let _e80 = roughness_19;
                    roughness_19 = clamp(_e80, 0.01f, 1f);
                    let _e84 = color_1;
                    let _e85 = L_5;
                    let _e86 = V_13;
                    let _e87 = N_17;
                    let _e88 = NdotV_15;
                    let _e89 = roughness_19;
                    let _e90 = mx_zeltner_sheen_brdf(_e85, _e86, _e87, _e88, _e89);
                    fr_1 = (_e84 * _e90);
                    let _e93 = NdotV_15;
                    let _e94 = roughness_19;
                    let _e95 = mx_zeltner_sheen_dir_albedo(_e93, _e94);
                    dirAlbedo_1 = _e95;
                    let _e97 = dirAlbedo_1;
                    let _e98 = fr_1;
                    let _e100 = closureData_1;
                    let _e103 = weight_1;
                    (*bsdf).response = (((_e97 * _e98) * _e100.occlusion) * _e103);
                }
            }
            let _e107 = dirAlbedo_1;
            let _e108 = weight_1;
            (*bsdf).throughput = vec3((1f - (_e107 * _e108)));
            return;
        }
    } else {
        let _e112 = closureData_1;
        if (_e112.closureType == 3i) {
            {
                let _e117 = mode_1;
                if (_e117 == 0i) {
                    {
                        let _e120 = NdotV_15;
                        let _e121 = roughness_19;
                        let _e122 = mx_imageworks_sheen_dir_albedo(_e120, _e121);
                        dirAlbedo_2 = _e122;
                    }
                } else {
                    {
                        let _e123 = roughness_19;
                        roughness_19 = clamp(_e123, 0.01f, 1f);
                        let _e127 = NdotV_15;
                        let _e128 = roughness_19;
                        let _e129 = mx_zeltner_sheen_dir_albedo(_e127, _e128);
                        dirAlbedo_2 = _e129;
                    }
                }
                let _e130 = N_17;
                let _e131 = mx_environment_irradiance(_e130);
                Li = _e131;
                let _e134 = Li;
                let _e135 = color_1;
                let _e137 = dirAlbedo_2;
                let _e139 = weight_1;
                (*bsdf).response = (((_e134 * _e135) * _e137) * _e139);
                let _e143 = dirAlbedo_2;
                let _e144 = weight_1;
                (*bsdf).throughput = vec3((1f - (_e143 * _e144)));
                return;
            }
        } else {
            return;
        }
    }
}