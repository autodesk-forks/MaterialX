struct BSDF {
    response: vec3<f32>,
    throughput: vec3<f32>,
}

struct surfaceshader {
    color: vec3<f32>,
    transparency: vec3<f32>,
}

struct volumeshader {
    color: vec3<f32>,
    transparency: vec3<f32>,
}

struct displacementshader {
    offset: vec3<f32>,
    scale: f32,
}

struct lightshader {
    intensity: vec3<f32>,
    direction: vec3<f32>,
}

struct PrivateUniforms_pixel {
    u_envMatrix: mat4x4<f32>,
    u_envLightIntensity: f32,
    u_envRadianceMips: i32,
    u_envRadianceSamples: i32,
    u_refractionTwoSided: i32,
    u_viewPosition: vec3<f32>,
    u_numActiveLightSources: i32,
}

struct PublicUniforms_pixel {
    backsurfaceshader: surfaceshader,
    displacementshader1_: displacementshader,
    material_type: i32,
    surface_cutout: vec3<f32>,
    surface_albedo: vec3<f32>,
    surface_roughness: f32,
    surface_anisotropy: f32,
    surface_rotation: f32,
    metal_f0_: vec3<f32>,
    opaque_albedo: vec3<f32>,
    opaque_f0_: f32,
    opaque_translucency: i32,
    opaque_mfp_modifier: vec3<f32>,
    opaque_mfp: f32,
    opaque_emission: i32,
    opaque_luminance: f32,
    opaque_luminance_modifier: vec3<f32>,
    transparent_ior: f32,
    transparent_color: vec3<f32>,
    transparent_distance: f32,
    layered_f0_: f32,
    layered_roughness: f32,
    layered_anisotropy: f32,
    layered_rotation: f32,
    layered_fraction: f32,
    layered_fraction_invert: i32,
    layered_diffuse: vec3<f32>,
    layered_bottom_f0_: vec3<f32>,
    glazing_transmission_color: vec3<f32>,
    glazing_transmission_roughness: f32,
    glazing_f0_: vec3<f32>,
    glazing_backface_culling: i32,
}

struct VertexData {
    normalWorld: vec3<f32>,
    tangentWorld: vec3<f32>,
    positionWorld: vec3<f32>,
}

struct FresnelData {
    model: i32,
    airy: bool,
    ior: vec3<f32>,
    extinction: vec3<f32>,
    F0_: vec3<f32>,
    F82_: vec3<f32>,
    F90_: vec3<f32>,
    exponent: f32,
    tf_thickness: f32,
    tf_ior: f32,
    refraction: bool,
}

struct LightData {
    direction: vec3<f32>,
    color: vec3<f32>,
    light_type: i32,
    intensity: f32,
    pad0_: f32,
    pad1_: f32,
}

struct LightData_pixel {
    u_lightData: array<LightData, 1>,
}

struct ClosureData {
    closureType: i32,
    L: vec3<f32>,
    V: vec3<f32>,
    N: vec3<f32>,
    P: vec3<f32>,
    occlusion: f32,
}

struct FragmentOutput {
    @location(0) out1_: vec4<f32>,
}

const FRESNEL_MODEL_DIELECTRIC: i32 = 0i;
const FRESNEL_MODEL_CONDUCTOR: i32 = 1i;
const FRESNEL_MODEL_SCHLICK: i32 = 2i;
const FUJII_CONSTANT_1_: f32 = 0.2877934f;
const FUJII_CONSTANT_2_: f32 = 0.07248825f;

@group(0) @binding(1) 
var<uniform> global: PrivateUniforms_pixel;
@group(0) @binding(2) 
var u_envRadiance_texture: texture_2d<f32>;
@group(0) @binding(3) 
var u_envRadiance_sampler: sampler;
@group(0) @binding(4) 
var u_envIrradiance_texture: texture_2d<f32>;
@group(0) @binding(5) 
var u_envIrradiance_sampler: sampler;
@group(0) @binding(6) 
var<uniform> global_1: PublicUniforms_pixel;
var<private> vd: VertexData;
var<private> out1_10: vec4<f32>;
@group(0) @binding(7) 
var<uniform> global_2: LightData_pixel;
var<private> gl_FrontFacing_1: bool;

fn mx_matrix_mul(v: vec2<f32>, m: mat2x2<f32>) -> vec2<f32> {
    var v_1: vec2<f32>;
    var m_1: mat2x2<f32>;

    v_1 = v;
    m_1 = m;
    let _e94 = v_1;
    let _e95 = m_1;
    return (_e94 * _e95);
}

fn mx_matrix_mul_1(v_2: vec3<f32>, m_2: mat3x3<f32>) -> vec3<f32> {
    var v_3: vec3<f32>;
    var m_3: mat3x3<f32>;

    v_3 = v_2;
    m_3 = m_2;
    let _e94 = v_3;
    let _e95 = m_3;
    return (_e94 * _e95);
}

fn mx_matrix_mul_2(v_4: vec4<f32>, m_4: mat4x4<f32>) -> vec4<f32> {
    var v_5: vec4<f32>;
    var m_5: mat4x4<f32>;

    v_5 = v_4;
    m_5 = m_4;
    let _e94 = v_5;
    let _e95 = m_5;
    return (_e94 * _e95);
}

fn mx_matrix_mul_3(m_6: mat2x2<f32>, v_6: vec2<f32>) -> vec2<f32> {
    var m_7: mat2x2<f32>;
    var v_7: vec2<f32>;

    m_7 = m_6;
    v_7 = v_6;
    let _e94 = m_7;
    let _e95 = v_7;
    return (_e94 * _e95);
}

fn mx_matrix_mul_4(m_8: mat3x3<f32>, v_8: vec3<f32>) -> vec3<f32> {
    var m_9: mat3x3<f32>;
    var v_9: vec3<f32>;

    m_9 = m_8;
    v_9 = v_8;
    let _e94 = m_9;
    let _e95 = v_9;
    return (_e94 * _e95);
}

fn mx_matrix_mul_5(m_10: mat4x4<f32>, v_10: vec4<f32>) -> vec4<f32> {
    var m_11: mat4x4<f32>;
    var v_11: vec4<f32>;

    m_11 = m_10;
    v_11 = v_10;
    let _e94 = m_11;
    let _e95 = v_11;
    return (_e94 * _e95);
}

fn mx_matrix_mul_6(m1_: mat2x2<f32>, m2_: mat2x2<f32>) -> mat2x2<f32> {
    var m1_1: mat2x2<f32>;
    var m2_1: mat2x2<f32>;

    m1_1 = m1_;
    m2_1 = m2_;
    let _e94 = m1_1;
    let _e95 = m2_1;
    return (_e94 * _e95);
}

fn mx_matrix_mul_7(m1_2: mat3x3<f32>, m2_2: mat3x3<f32>) -> mat3x3<f32> {
    var m1_3: mat3x3<f32>;
    var m2_3: mat3x3<f32>;

    m1_3 = m1_2;
    m2_3 = m2_2;
    let _e94 = m1_3;
    let _e95 = m2_3;
    return (_e94 * _e95);
}

fn mx_matrix_mul_8(m1_4: mat4x4<f32>, m2_4: mat4x4<f32>) -> mat4x4<f32> {
    var m1_5: mat4x4<f32>;
    var m2_5: mat4x4<f32>;

    m1_5 = m1_4;
    m2_5 = m2_4;
    let _e94 = m1_5;
    let _e95 = m2_5;
    return (_e94 * _e95);
}

fn mx_square(x: f32) -> f32 {
    var x_1: f32;

    x_1 = x;
    let _e92 = x_1;
    let _e93 = x_1;
    return (_e92 * _e93);
}

fn mx_square_1(x_2: vec2<f32>) -> vec2<f32> {
    var x_3: vec2<f32>;

    x_3 = x_2;
    let _e92 = x_3;
    let _e93 = x_3;
    return (_e92 * _e93);
}

fn mx_square_2(x_4: vec3<f32>) -> vec3<f32> {
    var x_5: vec3<f32>;

    x_5 = x_4;
    let _e92 = x_5;
    let _e93 = x_5;
    return (_e92 * _e93);
}

fn mx_srgb_encode(color: vec3<f32>) -> vec3<f32> {
    var color_1: vec3<f32>;
    var isAbove: vec3<bool>;
    var linSeg: vec3<f32>;
    var powSeg: vec3<f32>;

    color_1 = color;
    let _e92 = color_1;
    isAbove = (_e92 > vec3(0.0031308f));
    let _e97 = color_1;
    linSeg = (_e97 * 12.92f);
    let _e102 = color_1;
    powSeg = ((1.055f * pow(max(_e102, vec3(0f)), vec3(0.41666666f))) - vec3(0.055f));
    let _e116 = linSeg;
    let _e117 = powSeg;
    let _e118 = isAbove;
    return select(_e116, _e117, _e118);
}

fn mx_pow5_(x_6: f32) -> f32 {
    var x_7: f32;

    x_7 = x_6;
    let _e92 = x_7;
    let _e93 = mx_square(_e92);
    let _e94 = mx_square(_e93);
    let _e95 = x_7;
    return (_e94 * _e95);
}

fn mx_pow6_(x_8: f32) -> f32 {
    var x_9: f32;
    var x2_: f32;

    x_9 = x_8;
    let _e92 = x_9;
    let _e93 = mx_square(_e92);
    x2_ = _e93;
    let _e95 = x2_;
    let _e96 = mx_square(_e95);
    let _e97 = x2_;
    return (_e96 * _e97);
}

fn mx_fresnel_schlick(cosTheta: f32, F0_: f32) -> f32 {
    var cosTheta_1: f32;
    var F0_1: f32;
    var x_10: f32;
    var x5_: f32;

    cosTheta_1 = cosTheta;
    F0_1 = F0_;
    let _e95 = cosTheta_1;
    x_10 = clamp((1f - _e95), 0f, 1f);
    let _e101 = x_10;
    let _e102 = mx_pow5_(_e101);
    x5_ = _e102;
    let _e104 = F0_1;
    let _e106 = F0_1;
    let _e108 = x5_;
    return (_e104 + ((1f - _e106) * _e108));
}

fn mx_fresnel_schlick_1(cosTheta_2: f32, F0_2: vec3<f32>) -> vec3<f32> {
    var cosTheta_3: f32;
    var F0_3: vec3<f32>;
    var x_11: f32;
    var x5_1: f32;

    cosTheta_3 = cosTheta_2;
    F0_3 = F0_2;
    let _e95 = cosTheta_3;
    x_11 = clamp((1f - _e95), 0f, 1f);
    let _e101 = x_11;
    let _e102 = mx_pow5_(_e101);
    x5_1 = _e102;
    let _e104 = F0_3;
    let _e106 = F0_3;
    let _e109 = x5_1;
    return (_e104 + ((vec3(1f) - _e106) * _e109));
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
    let _e97 = cosTheta_5;
    x_12 = clamp((1f - _e97), 0f, 1f);
    let _e103 = x_12;
    let _e104 = mx_pow5_(_e103);
    x5_2 = _e104;
    let _e106 = F0_5;
    let _e107 = F90_1;
    let _e108 = x5_2;
    return mix(_e106, _e107, _e108);
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
    let _e97 = cosTheta_7;
    x_13 = clamp((1f - _e97), 0f, 1f);
    let _e103 = x_13;
    let _e104 = mx_pow5_(_e103);
    x5_3 = _e104;
    let _e106 = F0_7;
    let _e107 = F90_3;
    let _e108 = x5_3;
    return mix(_e106, _e107, vec3(_e108));
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
    let _e99 = cosTheta_9;
    x_14 = clamp((1f - _e99), 0f, 1f);
    let _e105 = F0_9;
    let _e106 = F90_5;
    let _e107 = x_14;
    let _e108 = exponent_1;
    return mix(_e105, _e106, pow(_e107, _e108));
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
    let _e99 = cosTheta_11;
    x_15 = clamp((1f - _e99), 0f, 1f);
    let _e105 = F0_11;
    let _e106 = F90_7;
    let _e107 = x_15;
    let _e108 = exponent_3;
    return mix(_e105, _e106, vec3(pow(_e107, _e108)));
}

fn mx_forward_facing_normal(N: vec3<f32>, V: vec3<f32>) -> vec3<f32> {
    var N_1: vec3<f32>;
    var V_1: vec3<f32>;
    var local: vec3<f32>;

    N_1 = N;
    V_1 = V;
    let _e94 = N_1;
    let _e95 = V_1;
    if (dot(_e94, _e95) < 0f) {
        let _e99 = N_1;
        local = -(_e99);
    } else {
        let _e101 = N_1;
        local = _e101;
    }
    let _e103 = local;
    return _e103;
}

fn mx_golden_ratio_sequence(i: i32) -> f32 {
    var i_1: i32;
    var GOLDEN_RATIO: f32 = 1.618034f;

    i_1 = i;
    let _e94 = i_1;
    let _e98 = GOLDEN_RATIO;
    return fract(((f32(_e94) + 1f) * _e98));
}

fn mx_spherical_fibonacci(i_2: i32, numSamples: i32) -> vec2<f32> {
    var i_3: i32;
    var numSamples_1: i32;

    i_3 = i_2;
    numSamples_1 = numSamples;
    let _e94 = i_3;
    let _e98 = numSamples_1;
    let _e101 = i_3;
    let _e102 = mx_golden_ratio_sequence(_e101);
    return vec2<f32>(((f32(_e94) + 0.5f) / f32(_e98)), _e102);
}

fn mx_uniform_sample_hemisphere(Xi: vec2<f32>) -> vec3<f32> {
    var Xi_1: vec2<f32>;
    var phi: f32;
    var cosTheta_12: f32;
    var sinTheta: f32;

    Xi_1 = Xi;
    let _e95 = Xi_1;
    phi = (6.2831855f * _e95.x);
    let _e100 = Xi_1;
    cosTheta_12 = (1f - _e100.y);
    let _e105 = cosTheta_12;
    let _e106 = mx_square(_e105);
    sinTheta = sqrt((1f - _e106));
    let _e110 = phi;
    let _e112 = sinTheta;
    let _e114 = phi;
    let _e116 = sinTheta;
    let _e118 = cosTheta_12;
    return vec3<f32>((cos(_e110) * _e112), (sin(_e114) * _e116), _e118);
}

fn mx_cosine_sample_hemisphere(Xi_2: vec2<f32>) -> vec3<f32> {
    var Xi_3: vec2<f32>;
    var phi_1: f32;
    var cosTheta_13: f32;
    var sinTheta_1: f32;

    Xi_3 = Xi_2;
    let _e95 = Xi_3;
    phi_1 = (6.2831855f * _e95.x);
    let _e99 = Xi_3;
    cosTheta_13 = sqrt(_e99.y);
    let _e104 = Xi_3;
    sinTheta_1 = sqrt((1f - _e104.y));
    let _e109 = phi_1;
    let _e111 = sinTheta_1;
    let _e113 = phi_1;
    let _e115 = sinTheta_1;
    let _e117 = cosTheta_13;
    return vec3<f32>((cos(_e109) * _e111), (sin(_e113) * _e115), _e117);
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
    let _e92 = N_3;
    if (_e92.z < 0f) {
        local_1 = -1f;
    } else {
        local_1 = 1f;
    }
    let _e100 = local_1;
    sign = _e100;
    let _e104 = sign;
    let _e105 = N_3;
    a = (-1f / (_e104 + _e105.z));
    let _e110 = N_3;
    let _e112 = N_3;
    let _e115 = a;
    b = ((_e110.x * _e112.y) * _e115);
    let _e119 = sign;
    let _e120 = N_3;
    let _e123 = N_3;
    let _e126 = a;
    let _e129 = sign;
    let _e130 = b;
    let _e132 = sign;
    let _e134 = N_3;
    X = vec3<f32>((1f + (((_e119 * _e120.x) * _e123.x) * _e126)), (_e129 * _e130), (-(_e132) * _e134.x));
    let _e139 = b;
    let _e140 = sign;
    let _e141 = N_3;
    let _e143 = N_3;
    let _e146 = a;
    let _e149 = N_3;
    Y = vec3<f32>(_e139, (_e140 + ((_e141.y * _e143.y) * _e146)), -(_e149.y));
    let _e154 = X;
    let _e155 = Y;
    let _e156 = N_3;
    return mat3x3<f32>(vec3<f32>(_e154.x, _e154.y, _e154.z), vec3<f32>(_e155.x, _e155.y, _e155.z), vec3<f32>(_e156.x, _e156.y, _e156.z));
}

fn mx_ggx_NDF(H: vec3<f32>, alpha: vec2<f32>) -> f32 {
    var H_1: vec3<f32>;
    var alpha_1: vec2<f32>;
    var He: vec2<f32>;
    var denom: f32;

    H_1 = H;
    alpha_1 = alpha;
    let _e97 = H_1;
    let _e99 = alpha_1;
    He = (_e97.xy / _e99);
    let _e102 = He;
    let _e103 = He;
    let _e105 = H_1;
    let _e107 = mx_square(_e105.z);
    denom = (dot(_e102, _e103) + _e107);
    let _e112 = alpha_1;
    let _e115 = alpha_1;
    let _e118 = denom;
    let _e119 = mx_square(_e118);
    return (1f / (((3.1415927f * _e112.x) * _e115.y) * _e119));
}

fn mx_ggx_importance_sample_VNDF(Xi_4: vec2<f32>, V_2: vec3<f32>, alpha_2: vec2<f32>) -> vec3<f32> {
    var Xi_5: vec2<f32>;
    var V_3: vec3<f32>;
    var alpha_3: vec2<f32>;
    var phi_2: f32;
    var z: f32;
    var sinTheta_2: f32;
    var x_16: f32;
    var y: f32;
    var c: vec3<f32>;
    var H_2: vec3<f32>;

    Xi_5 = Xi_4;
    V_3 = V_2;
    alpha_3 = alpha_2;
    let _e99 = V_3;
    let _e101 = alpha_3;
    let _e102 = (_e99.xy * _e101);
    let _e103 = V_3;
    V_3 = normalize(vec3<f32>(_e102.x, _e102.y, _e103.z));
    let _e112 = Xi_5;
    phi_2 = (6.2831855f * _e112.x);
    let _e117 = Xi_5;
    let _e121 = V_3;
    let _e125 = V_3;
    z = (((1f - _e117.y) * (1f + _e121.z)) - _e125.z);
    let _e130 = z;
    let _e131 = z;
    sinTheta_2 = sqrt(clamp((1f - (_e130 * _e131)), 0f, 1f));
    let _e139 = sinTheta_2;
    let _e140 = phi_2;
    x_16 = (_e139 * cos(_e140));
    let _e144 = sinTheta_2;
    let _e145 = phi_2;
    y = (_e144 * sin(_e145));
    let _e149 = x_16;
    let _e150 = y;
    let _e151 = z;
    c = vec3<f32>(_e149, _e150, _e151);
    let _e154 = c;
    let _e155 = V_3;
    H_2 = (_e154 + _e155);
    let _e158 = H_2;
    let _e160 = alpha_3;
    let _e161 = (_e158.xy * _e160);
    let _e162 = H_2;
    H_2 = normalize(vec3<f32>(_e161.x, _e161.y, max(_e162.z, 0f)));
    let _e170 = H_2;
    return _e170;
}

fn mx_ggx_smith_G1_(cosTheta_14: f32, alpha_4: f32) -> f32 {
    var cosTheta_15: f32;
    var alpha_5: f32;
    var cosTheta2_: f32;
    var tanTheta2_: f32;

    cosTheta_15 = cosTheta_14;
    alpha_5 = alpha_4;
    let _e97 = cosTheta_15;
    let _e98 = mx_square(_e97);
    cosTheta2_ = _e98;
    let _e101 = cosTheta2_;
    let _e103 = cosTheta2_;
    tanTheta2_ = ((1f - _e101) / _e103);
    let _e109 = alpha_5;
    let _e110 = mx_square(_e109);
    let _e111 = tanTheta2_;
    return (2f / (1f + sqrt((1f + (_e110 * _e111)))));
}

fn mx_ggx_smith_G2_(NdotL: f32, NdotV: f32, alpha_6: f32) -> f32 {
    var NdotL_1: f32;
    var NdotV_1: f32;
    var alpha_7: f32;
    var alpha2_: f32;
    var lambdaL: f32;
    var lambdaV: f32;

    NdotL_1 = NdotL;
    NdotV_1 = NdotV;
    alpha_7 = alpha_6;
    let _e99 = alpha_7;
    let _e100 = mx_square(_e99);
    alpha2_ = _e100;
    let _e102 = alpha2_;
    let _e104 = alpha2_;
    let _e106 = NdotL_1;
    let _e107 = mx_square(_e106);
    lambdaL = sqrt((_e102 + ((1f - _e104) * _e107)));
    let _e112 = alpha2_;
    let _e114 = alpha2_;
    let _e116 = NdotV_1;
    let _e117 = mx_square(_e116);
    lambdaV = sqrt((_e112 + ((1f - _e114) * _e117)));
    let _e123 = NdotL_1;
    let _e125 = NdotV_1;
    let _e127 = lambdaL;
    let _e128 = NdotV_1;
    let _e130 = lambdaV;
    let _e131 = NdotL_1;
    return (((2f * _e123) * _e125) / ((_e127 * _e128) + (_e130 * _e131)));
}

fn mx_ggx_dir_albedo_analytic(NdotV_2: f32, alpha_8: f32, F0_12: vec3<f32>, F90_8: vec3<f32>) -> vec3<f32> {
    var NdotV_3: f32;
    var alpha_9: f32;
    var F0_13: vec3<f32>;
    var F90_9: vec3<f32>;
    var x_17: f32;
    var y_1: f32;
    var x2_1: f32;
    var y2_: f32;
    var r: vec4<f32>;
    var AB: vec2<f32>;

    NdotV_3 = NdotV_2;
    alpha_9 = alpha_8;
    F0_13 = F0_12;
    F90_9 = F90_8;
    let _e101 = NdotV_3;
    x_17 = _e101;
    let _e103 = alpha_9;
    y_1 = _e103;
    let _e105 = x_17;
    let _e106 = mx_square(_e105);
    x2_1 = _e106;
    let _e108 = y_1;
    let _e109 = mx_square(_e108);
    y2_ = _e109;
    let _e124 = x_17;
    let _e132 = y_1;
    let _e143 = x_17;
    let _e145 = y_1;
    let _e153 = x2_1;
    let _e164 = y2_;
    let _e174 = x2_1;
    let _e176 = y_1;
    let _e184 = x_17;
    let _e186 = y2_;
    let _e196 = x2_1;
    let _e198 = y2_;
    r = ((((((((vec4<f32>(0.1003f, 0.9345f, 1f, 1f) + (vec4<f32>(-0.6303f, -2.323f, -1.765f, 0.2281f) * _e124)) + (vec4<f32>(9.748f, 2.229f, 8.263f, 15.94f) * _e132)) + ((vec4<f32>(-2.038f, -3.748f, 11.53f, -55.83f) * _e143) * _e145)) + (vec4<f32>(29.34f, 1.424f, 28.96f, 13.08f) * _e153)) + (vec4<f32>(-8.245f, -0.7684f, -7.507f, 41.26f) * _e164)) + ((vec4<f32>(-26.44f, 1.436f, -36.11f, 54.9f) * _e174) * _e176)) + ((vec4<f32>(19.99f, 0.2913f, 15.86f, 300.2f) * _e184) * _e186)) + ((vec4<f32>(-5.448f, 0.6286f, 33.37f, -285.1f) * _e196) * _e198));
    let _e202 = r;
    let _e204 = r;
    AB = clamp((_e202.xy / _e204.zw), vec2(0f), vec2(1f));
    let _e213 = F0_13;
    let _e214 = AB;
    let _e217 = F90_9;
    let _e218 = AB;
    return ((_e213 * _e214.x) + (_e217 * _e218.y));
}

fn mx_ggx_dir_albedo_table_lookup(NdotV_4: f32, alpha_10: f32, F0_14: vec3<f32>, F90_10: vec3<f32>) -> vec3<f32> {
    var NdotV_5: f32;
    var alpha_11: f32;
    var F0_15: vec3<f32>;
    var F90_11: vec3<f32>;

    NdotV_5 = NdotV_4;
    alpha_11 = alpha_10;
    F0_15 = F0_14;
    F90_11 = F90_10;
    return vec3(0f);
}

fn mx_ggx_dir_albedo_monte_carlo(NdotV_6: f32, alpha_12: f32, F0_16: vec3<f32>, F90_12: vec3<f32>) -> vec3<f32> {
    var NdotV_7: f32;
    var alpha_13: f32;
    var F0_17: vec3<f32>;
    var F90_13: vec3<f32>;
    var V_4: vec3<f32>;
    var AB_1: vec2<f32> = vec2(0f);
    var SAMPLE_COUNT: i32 = 64i;
    var i_4: i32 = 0i;
    var Xi_6: vec2<f32>;
    var H_3: vec3<f32>;
    var L: vec3<f32>;
    var NdotL_2: f32;
    var VdotH: f32;
    var Fc: f32;
    var G2_: f32;

    NdotV_7 = NdotV_6;
    alpha_13 = alpha_12;
    F0_17 = F0_16;
    F90_13 = F90_12;
    let _e101 = NdotV_7;
    NdotV_7 = clamp(_e101, 0.00000001f, 1f);
    let _e106 = NdotV_7;
    let _e107 = mx_square(_e106);
    let _e111 = NdotV_7;
    V_4 = vec3<f32>(sqrt((1f - _e107)), 0f, _e111);
    loop {
        let _e122 = i_4;
        let _e123 = SAMPLE_COUNT;
        if !((_e122 < _e123)) {
            break;
        }
        {
            let _e129 = i_4;
            let _e130 = SAMPLE_COUNT;
            let _e131 = mx_spherical_fibonacci(_e129, _e130);
            Xi_6 = _e131;
            let _e133 = Xi_6;
            let _e134 = V_4;
            let _e135 = alpha_13;
            let _e137 = mx_ggx_importance_sample_VNDF(_e133, _e134, vec2(_e135));
            H_3 = _e137;
            let _e139 = V_4;
            let _e140 = H_3;
            L = -(reflect(_e139, _e140));
            let _e144 = L;
            NdotL_2 = clamp(_e144.z, 0.00000001f, 1f);
            let _e150 = V_4;
            let _e151 = H_3;
            VdotH = clamp(dot(_e150, _e151), 0.00000001f, 1f);
            let _e157 = VdotH;
            let _e160 = mx_fresnel_schlick_2(_e157, 0f, 1f);
            Fc = _e160;
            let _e162 = NdotL_2;
            let _e163 = NdotV_7;
            let _e164 = alpha_13;
            let _e165 = mx_ggx_smith_G2_(_e162, _e163, _e164);
            G2_ = _e165;
            let _e167 = AB_1;
            let _e168 = G2_;
            let _e170 = Fc;
            let _e173 = G2_;
            let _e174 = Fc;
            AB_1 = (_e167 + vec2<f32>((_e168 * (1f - _e170)), (_e173 * _e174)));
        }
        continuing {
            let _e126 = i_4;
            i_4 = (_e126 + 1i);
        }
    }
    let _e178 = AB_1;
    let _e179 = NdotV_7;
    let _e180 = alpha_13;
    let _e181 = mx_ggx_smith_G1_(_e179, _e180);
    let _e182 = SAMPLE_COUNT;
    AB_1 = (_e178 / vec2((_e181 * f32(_e182))));
    let _e187 = F0_17;
    let _e188 = AB_1;
    let _e191 = F90_13;
    let _e192 = AB_1;
    return ((_e187 * _e188.x) + (_e191 * _e192.y));
}

fn mx_ggx_dir_albedo(NdotV_8: f32, alpha_14: f32, F0_18: vec3<f32>, F90_14: vec3<f32>) -> vec3<f32> {
    var NdotV_9: f32;
    var alpha_15: f32;
    var F0_19: vec3<f32>;
    var F90_15: vec3<f32>;

    NdotV_9 = NdotV_8;
    alpha_15 = alpha_14;
    F0_19 = F0_18;
    F90_15 = F90_14;
    let _e101 = NdotV_9;
    let _e102 = alpha_15;
    let _e103 = F0_19;
    let _e104 = F90_15;
    let _e105 = mx_ggx_dir_albedo_analytic(_e101, _e102, _e103, _e104);
    return _e105;
}

fn mx_ggx_dir_albedo_1(NdotV_10: f32, alpha_16: f32, F0_20: f32, F90_16: f32) -> f32 {
    var NdotV_11: f32;
    var alpha_17: f32;
    var F0_21: f32;
    var F90_17: f32;

    NdotV_11 = NdotV_10;
    alpha_17 = alpha_16;
    F0_21 = F0_20;
    F90_17 = F90_16;
    let _e101 = NdotV_11;
    let _e102 = alpha_17;
    let _e103 = F0_21;
    let _e105 = F90_17;
    let _e107 = mx_ggx_dir_albedo(_e101, _e102, vec3(_e103), vec3(_e105));
    return _e107.x;
}

fn mx_ggx_energy_compensation(NdotV_12: f32, alpha_18: f32, Fss: vec3<f32>) -> vec3<f32> {
    var NdotV_13: f32;
    var alpha_19: f32;
    var Fss_1: vec3<f32>;
    var Ess: f32;

    NdotV_13 = NdotV_12;
    alpha_19 = alpha_18;
    Fss_1 = Fss;
    let _e99 = NdotV_13;
    let _e100 = alpha_19;
    let _e103 = mx_ggx_dir_albedo_1(_e99, _e100, 1f, 1f);
    Ess = _e103;
    let _e106 = Fss_1;
    let _e108 = Ess;
    let _e111 = Ess;
    return (vec3(1f) + ((_e106 * (1f - _e108)) / vec3(_e111)));
}

fn mx_ggx_energy_compensation_1(NdotV_14: f32, alpha_20: f32, Fss_2: f32) -> f32 {
    var NdotV_15: f32;
    var alpha_21: f32;
    var Fss_3: f32;

    NdotV_15 = NdotV_14;
    alpha_21 = alpha_20;
    Fss_3 = Fss_2;
    let _e99 = NdotV_15;
    let _e100 = alpha_21;
    let _e101 = Fss_3;
    let _e103 = mx_ggx_energy_compensation(_e99, _e100, vec3(_e101));
    return _e103.x;
}

fn mx_average_alpha(alpha_22: vec2<f32>) -> f32 {
    var alpha_23: vec2<f32>;

    alpha_23 = alpha_22;
    let _e95 = alpha_23;
    let _e97 = alpha_23;
    return sqrt((_e95.x * _e97.y));
}

fn mx_ior_to_f0_(ior: f32) -> f32 {
    var ior_1: f32;

    ior_1 = ior;
    let _e95 = ior_1;
    let _e98 = ior_1;
    let _e102 = mx_square(((_e95 - 1f) / (_e98 + 1f)));
    return _e102;
}

fn mx_f0_to_ior(F0_22: f32) -> f32 {
    var F0_23: f32;
    var sqrtF0_: f32;

    F0_23 = F0_22;
    let _e95 = F0_23;
    sqrtF0_ = sqrt(clamp(_e95, 0.01f, 0.99f));
    let _e102 = sqrtF0_;
    let _e105 = sqrtF0_;
    return ((1f + _e102) / (1f - _e105));
}

fn mx_f0_to_ior_1(F0_24: vec3<f32>) -> vec3<f32> {
    var F0_25: vec3<f32>;
    var sqrtF0_1: vec3<f32>;

    F0_25 = F0_24;
    let _e95 = F0_25;
    sqrtF0_1 = sqrt(clamp(_e95, vec3(0.01f), vec3(0.99f)));
    let _e105 = sqrtF0_1;
    let _e109 = sqrtF0_1;
    return ((vec3(1f) + _e105) / (vec3(1f) - _e109));
}

fn mx_fresnel_hoffman_schlick(cosTheta_16: f32, fd: FresnelData) -> vec3<f32> {
    var cosTheta_17: f32;
    var fd_1: FresnelData;
    var COS_THETA_MAX: f32 = 0.14285715f;
    var COS_THETA_FACTOR: f32;
    var x_18: f32;
    var a_1: vec3<f32>;

    cosTheta_17 = cosTheta_16;
    fd_1 = fd;
    let _e102 = COS_THETA_MAX;
    let _e104 = COS_THETA_MAX;
    COS_THETA_FACTOR = (1f / (_e102 * pow((1f - _e104), 6f)));
    let _e111 = cosTheta_17;
    x_18 = clamp(_e111, 0f, 1f);
    let _e116 = fd_1;
    let _e118 = fd_1;
    let _e121 = COS_THETA_MAX;
    let _e123 = fd_1;
    let _e130 = fd_1;
    let _e134 = COS_THETA_FACTOR;
    a_1 = ((mix(_e116.F0_, _e118.F90_, vec3(pow((1f - _e121), _e123.exponent))) * (vec3(1f) - _e130.F82_)) * _e134);
    let _e137 = fd_1;
    let _e139 = fd_1;
    let _e142 = x_18;
    let _e144 = fd_1;
    let _e149 = a_1;
    let _e150 = x_18;
    let _e153 = x_18;
    let _e155 = mx_pow6_((1f - _e153));
    return (mix(_e137.F0_, _e139.F90_, vec3(pow((1f - _e142), _e144.exponent))) - ((_e149 * _e150) * _e155));
}

fn mx_fresnel_dielectric(cosTheta_18: f32, ior_2: f32) -> f32 {
    var cosTheta_19: f32;
    var ior_3: f32;
    var c_1: f32;
    var g2_: f32;
    var g: f32;

    cosTheta_19 = cosTheta_18;
    ior_3 = ior_2;
    let _e97 = cosTheta_19;
    c_1 = _e97;
    let _e99 = ior_3;
    let _e100 = ior_3;
    let _e102 = c_1;
    let _e103 = c_1;
    g2_ = (((_e99 * _e100) + (_e102 * _e103)) - 1f);
    let _e109 = g2_;
    if (_e109 < 0f) {
        {
            return 1f;
        }
    }
    let _e113 = g2_;
    g = sqrt(_e113);
    let _e117 = g;
    let _e118 = c_1;
    let _e120 = g;
    let _e121 = c_1;
    let _e124 = mx_square(((_e117 - _e118) / (_e120 + _e121)));
    let _e127 = g;
    let _e128 = c_1;
    let _e130 = c_1;
    let _e134 = g;
    let _e135 = c_1;
    let _e137 = c_1;
    let _e142 = mx_square(((((_e127 + _e128) * _e130) - 1f) / (((_e134 - _e135) * _e137) + 1f)));
    return ((0.5f * _e124) * (1f + _e142));
}

fn mx_fresnel_dielectric_polarized(cosTheta_20: f32, ior_4: f32) -> vec2<f32> {
    var cosTheta_21: f32;
    var ior_5: f32;
    var cosTheta2_1: f32;
    var sinTheta2_: f32;
    var t0_: f32;
    var t1_: f32;
    var t2_: f32;
    var Rs: f32;
    var t3_: f32;
    var t4_: f32;
    var Rp: f32;

    cosTheta_21 = cosTheta_20;
    ior_5 = ior_4;
    let _e97 = cosTheta_21;
    let _e101 = mx_square(clamp(_e97, 0f, 1f));
    cosTheta2_1 = _e101;
    let _e104 = cosTheta2_1;
    sinTheta2_ = (1f - _e104);
    let _e107 = ior_5;
    let _e108 = ior_5;
    let _e110 = sinTheta2_;
    t0_ = max(((_e107 * _e108) - _e110), 0f);
    let _e115 = t0_;
    let _e116 = cosTheta2_1;
    t1_ = (_e115 + _e116);
    let _e120 = t0_;
    let _e123 = cosTheta_21;
    t2_ = ((2f * sqrt(_e120)) * _e123);
    let _e126 = t1_;
    let _e127 = t2_;
    let _e129 = t1_;
    let _e130 = t2_;
    Rs = ((_e126 - _e127) / (_e129 + _e130));
    let _e134 = cosTheta2_1;
    let _e135 = t0_;
    let _e137 = sinTheta2_;
    let _e138 = sinTheta2_;
    t3_ = ((_e134 * _e135) + (_e137 * _e138));
    let _e142 = t2_;
    let _e143 = sinTheta2_;
    t4_ = (_e142 * _e143);
    let _e146 = Rs;
    let _e147 = t3_;
    let _e148 = t4_;
    let _e151 = t3_;
    let _e152 = t4_;
    Rp = ((_e146 * (_e147 - _e148)) / (_e151 + _e152));
    let _e156 = Rp;
    let _e157 = Rs;
    return vec2<f32>(_e156, _e157);
}

fn mx_fresnel_conductor_polarized(cosTheta_22: f32, n: vec3<f32>, k: vec3<f32>, Rp_1: ptr<function, vec3<f32>>, Rs_1: ptr<function, vec3<f32>>) {
    var cosTheta_23: f32;
    var n_1: vec3<f32>;
    var k_1: vec3<f32>;
    var cosTheta2_2: f32;
    var sinTheta2_1: f32;
    var n2_: vec3<f32>;
    var k2_: vec3<f32>;
    var t0_1: vec3<f32>;
    var a2plusb2_: vec3<f32>;
    var t1_1: vec3<f32>;
    var a_2: vec3<f32>;
    var t2_1: vec3<f32>;
    var t3_1: vec3<f32>;
    var t4_1: vec3<f32>;

    cosTheta_23 = cosTheta_22;
    n_1 = n;
    k_1 = k;
    let _e101 = cosTheta_23;
    let _e105 = mx_square(clamp(_e101, 0f, 1f));
    cosTheta2_2 = _e105;
    let _e108 = cosTheta2_2;
    sinTheta2_1 = (1f - _e108);
    let _e111 = n_1;
    let _e112 = n_1;
    n2_ = (_e111 * _e112);
    let _e115 = k_1;
    let _e116 = k_1;
    k2_ = (_e115 * _e116);
    let _e119 = n2_;
    let _e120 = k2_;
    let _e122 = sinTheta2_1;
    t0_1 = ((_e119 - _e120) - vec3(_e122));
    let _e126 = t0_1;
    let _e127 = t0_1;
    let _e130 = n2_;
    let _e132 = k2_;
    a2plusb2_ = sqrt(((_e126 * _e127) + ((4f * _e130) * _e132)));
    let _e137 = a2plusb2_;
    let _e138 = cosTheta2_2;
    t1_1 = (_e137 + vec3(_e138));
    let _e143 = a2plusb2_;
    let _e144 = t0_1;
    a_2 = sqrt(max((0.5f * (_e143 + _e144)), vec3(0f)));
    let _e153 = a_2;
    let _e155 = cosTheta_23;
    t2_1 = ((2f * _e153) * _e155);
    let _e158 = t1_1;
    let _e159 = t2_1;
    let _e161 = t1_1;
    let _e162 = t2_1;
    (*Rs_1) = ((_e158 - _e159) / (_e161 + _e162));
    let _e165 = cosTheta2_2;
    let _e166 = a2plusb2_;
    let _e168 = sinTheta2_1;
    let _e169 = sinTheta2_1;
    t3_1 = ((_e165 * _e166) + vec3((_e168 * _e169)));
    let _e174 = t2_1;
    let _e175 = sinTheta2_1;
    t4_1 = (_e174 * _e175);
    let _e178 = (*Rs_1);
    let _e179 = t3_1;
    let _e180 = t4_1;
    let _e183 = t3_1;
    let _e184 = t4_1;
    (*Rp_1) = ((_e178 * (_e179 - _e180)) / (_e183 + _e184));
    return;
}

fn mx_fresnel_conductor(cosTheta_24: f32, n_2: vec3<f32>, k_2: vec3<f32>) -> vec3<f32> {
    var cosTheta_25: f32;
    var n_3: vec3<f32>;
    var k_3: vec3<f32>;
    var Rp_2: vec3<f32>;
    var Rs_2: vec3<f32>;

    cosTheta_25 = cosTheta_24;
    n_3 = n_2;
    k_3 = k_2;
    let _e101 = cosTheta_25;
    let _e102 = n_3;
    let _e103 = k_3;
    mx_fresnel_conductor_polarized(_e101, _e102, _e103, (&Rp_2), (&Rs_2));
    let _e109 = Rp_2;
    let _e110 = Rs_2;
    return (0.5f * (_e109 + _e110));
}

fn mx_fresnel_conductor_phase_polarized(cosTheta_26: f32, eta1_: f32, eta2_: vec3<f32>, kappa2_: vec3<f32>, phiP: ptr<function, vec3<f32>>, phiS: ptr<function, vec3<f32>>) {
    var cosTheta_27: f32;
    var eta1_1: f32;
    var eta2_1: vec3<f32>;
    var kappa2_1: vec3<f32>;
    var k2_1: vec3<f32>;
    var sinThetaSqr: vec3<f32>;
    var A: vec3<f32>;
    var B: vec3<f32>;
    var U: vec3<f32>;
    var V_5: vec3<f32>;

    cosTheta_27 = cosTheta_26;
    eta1_1 = eta1_;
    eta2_1 = eta2_;
    kappa2_1 = kappa2_;
    let _e103 = kappa2_1;
    let _e104 = eta2_1;
    k2_1 = (_e103 / _e104);
    let _e109 = cosTheta_27;
    let _e110 = cosTheta_27;
    sinThetaSqr = (vec3(1f) - vec3((_e109 * _e110)));
    let _e115 = eta2_1;
    let _e116 = eta2_1;
    let _e120 = k2_1;
    let _e121 = k2_1;
    let _e125 = eta1_1;
    let _e126 = eta1_1;
    let _e128 = sinThetaSqr;
    A = (((_e115 * _e116) * (vec3(1f) - (_e120 * _e121))) - ((_e125 * _e126) * _e128));
    let _e132 = A;
    let _e133 = A;
    let _e136 = eta2_1;
    let _e138 = eta2_1;
    let _e140 = k2_1;
    let _e142 = mx_square_2((((2f * _e136) * _e138) * _e140));
    B = sqrt(((_e132 * _e133) + _e142));
    let _e146 = A;
    let _e147 = B;
    U = sqrt(((_e146 + _e147) / vec3(2f)));
    let _e156 = B;
    let _e157 = A;
    V_5 = max(vec3(0f), sqrt(((_e156 - _e157) / vec3(2f))));
    let _e166 = eta1_1;
    let _e168 = V_5;
    let _e170 = cosTheta_27;
    let _e172 = U;
    let _e173 = U;
    let _e175 = V_5;
    let _e176 = V_5;
    let _e179 = eta1_1;
    let _e180 = cosTheta_27;
    let _e182 = mx_square((_e179 * _e180));
    (*phiS) = atan2((((2f * _e166) * _e168) * _e170), (((_e172 * _e173) + (_e175 * _e176)) - vec3(_e182)));
    let _e187 = eta1_1;
    let _e189 = eta2_1;
    let _e191 = eta2_1;
    let _e193 = cosTheta_27;
    let _e196 = k2_1;
    let _e198 = U;
    let _e202 = k2_1;
    let _e203 = k2_1;
    let _e206 = V_5;
    let _e210 = eta2_1;
    let _e211 = eta2_1;
    let _e215 = k2_1;
    let _e216 = k2_1;
    let _e220 = cosTheta_27;
    let _e222 = mx_square_2((((_e210 * _e211) * (vec3(1f) + (_e215 * _e216))) * _e220));
    let _e223 = eta1_1;
    let _e224 = eta1_1;
    let _e226 = U;
    let _e227 = U;
    let _e229 = V_5;
    let _e230 = V_5;
    (*phiP) = atan2((((((2f * _e187) * _e189) * _e191) * _e193) * (((2f * _e196) * _e198) - ((vec3(1f) - (_e202 * _e203)) * _e206))), (_e222 - ((_e223 * _e224) * ((_e226 * _e227) + (_e229 * _e230)))));
    return;
}

fn mx_eval_sensitivity(opd: f32, shift: vec3<f32>) -> vec3<f32> {
    var opd_1: f32;
    var shift_1: vec3<f32>;
    var phase: f32;
    var val: vec3<f32> = vec3<f32>(0.00000000000054856f, 0.00000000000044201f, 0.00000000000052481f);
    var pos: vec3<f32> = vec3<f32>(1681000f, 1795300f, 2208400f);
    var var_: vec3<f32> = vec3<f32>(4327800000f, 9304600000f, 6612100000f);
    var xyz: vec3<f32>;

    opd_1 = opd;
    shift_1 = shift;
    let _e100 = opd_1;
    phase = (6.2831855f * _e100);
    let _e118 = val;
    let _e122 = var_;
    let _e126 = pos;
    let _e127 = phase;
    let _e129 = shift_1;
    let _e133 = var_;
    let _e135 = phase;
    let _e137 = phase;
    xyz = (((_e118 * sqrt((6.2831855f * _e122))) * cos(((_e126 * _e127) + _e129))) * exp(((-(_e133) * _e135) * _e137)));
    let _e143 = xyz;
    let _e154 = phase;
    let _e158 = shift_1.x;
    let _e164 = phase;
    let _e166 = phase;
    xyz.x = (_e143.x + ((0.00000001644083f * cos(((2239900f * _e154) + _e158))) * exp(((-4528200000f * _e164) * _e166))));
    let _e171 = xyz;
    return (_e171 / vec3(0.00000010685f));
}

fn mx_fresnel_airy(cosTheta_28: f32, fd_2: FresnelData) -> vec3<f32> {
    var cosTheta_29: f32;
    var fd_3: FresnelData;
    var XYZ_TO_RGB: mat3x3<f32> = mat3x3<f32>(vec3<f32>(2.3706744f, -0.513885f, 0.0052982f), vec3<f32>(-0.9000405f, 1.4253036f, -0.0146949f), vec3<f32>(-0.4706338f, 0.0885814f, 1.0093968f));
    var eta1_2: f32 = 1f;
    var eta2_2: f32;
    var local_2: vec3<f32>;
    var eta3_: vec3<f32>;
    var local_3: vec3<f32>;
    var kappa3_: vec3<f32>;
    var cosThetaT: f32;
    var R12_: vec2<f32>;
    var T121_: vec2<f32>;
    var R23p: vec3<f32>;
    var R23s: vec3<f32>;
    var f: vec3<f32>;
    var cosB: f32;
    var local_4: f32;
    var phi21_: vec2<f32>;
    var phi23p: vec3<f32>;
    var phi23s: vec3<f32>;
    var local_5: f32;
    var local_6: f32;
    var local_7: f32;
    var r123p: vec3<f32>;
    var r123s: vec3<f32>;
    var I: vec3<f32> = vec3(0f);
    var Cm: vec3<f32>;
    var Sm: vec3<f32>;
    var distMeters: f32;
    var opd_2: f32;
    var Rs_3: vec3<f32>;
    var m_12: i32 = 1i;
    var Rp_3: vec3<f32>;
    var m_13: i32 = 1i;

    cosTheta_29 = cosTheta_28;
    fd_3 = fd_2;
    let _e117 = fd_3;
    let _e119 = eta1_2;
    eta2_2 = max(_e117.tf_ior, _e119);
    let _e122 = fd_3;
    if (_e122.model == FRESNEL_MODEL_SCHLICK) {
        let _e125 = fd_3;
        let _e127 = mx_f0_to_ior_1(_e125.F0_);
        local_2 = _e127;
    } else {
        let _e128 = fd_3;
        local_2 = _e128.ior;
    }
    let _e131 = local_2;
    eta3_ = _e131;
    let _e133 = fd_3;
    if (_e133.model == FRESNEL_MODEL_SCHLICK) {
        local_3 = vec3(0f);
    } else {
        let _e138 = fd_3;
        local_3 = _e138.extinction;
    }
    let _e141 = local_3;
    kappa3_ = _e141;
    let _e145 = cosTheta_29;
    let _e146 = mx_square(_e145);
    let _e148 = eta1_2;
    let _e149 = eta2_2;
    let _e151 = mx_square((_e148 / _e149));
    cosThetaT = sqrt((1f - ((1f - _e146) * _e151)));
    let _e156 = cosTheta_29;
    let _e157 = eta2_2;
    let _e158 = eta1_2;
    let _e160 = mx_fresnel_dielectric_polarized(_e156, (_e157 / _e158));
    R12_ = _e160;
    let _e162 = cosThetaT;
    if (_e162 <= 0f) {
        {
            R12_ = vec2(1f);
        }
    }
    let _e169 = R12_;
    T121_ = (vec2(1f) - _e169);
    let _e174 = fd_3;
    if (_e174.model == FRESNEL_MODEL_SCHLICK) {
        {
            let _e177 = cosThetaT;
            let _e178 = fd_3;
            let _e179 = mx_fresnel_hoffman_schlick(_e177, _e178);
            f = _e179;
            let _e182 = f;
            R23p = (0.5f * _e182);
            let _e185 = f;
            R23s = (0.5f * _e185);
        }
    } else {
        {
            let _e187 = cosThetaT;
            let _e188 = eta3_;
            let _e189 = eta2_2;
            let _e192 = kappa3_;
            let _e193 = eta2_2;
            mx_fresnel_conductor_polarized(_e187, (_e188 / vec3(_e189)), (_e192 / vec3(_e193)), (&R23p), (&R23s));
        }
    }
    let _e200 = eta2_2;
    let _e201 = eta1_2;
    cosB = cos(atan((_e200 / _e201)));
    let _e206 = cosTheta_29;
    let _e207 = cosB;
    if (_e206 < _e207) {
        local_4 = 0f;
    } else {
        local_4 = 3.1415927f;
    }
    let _e212 = local_4;
    phi21_ = vec2<f32>(_e212, 3.1415927f);
    let _e218 = fd_3;
    if (_e218.model == FRESNEL_MODEL_SCHLICK) {
        {
            let _e223 = eta3_.x;
            let _e224 = eta2_2;
            if (_e223 < _e224) {
                local_5 = 3.1415927f;
            } else {
                local_5 = 0f;
            }
            let _e229 = local_5;
            let _e232 = eta3_.y;
            let _e233 = eta2_2;
            if (_e232 < _e233) {
                local_6 = 3.1415927f;
            } else {
                local_6 = 0f;
            }
            let _e238 = local_6;
            let _e241 = eta3_.z;
            let _e242 = eta2_2;
            if (_e241 < _e242) {
                local_7 = 3.1415927f;
            } else {
                local_7 = 0f;
            }
            let _e247 = local_7;
            phi23p = vec3<f32>(_e229, _e238, _e247);
            let _e249 = phi23p;
            phi23s = _e249;
        }
    } else {
        {
            let _e250 = cosThetaT;
            let _e251 = eta2_2;
            let _e252 = eta3_;
            let _e253 = kappa3_;
            mx_fresnel_conductor_phase_polarized(_e250, _e251, _e252, _e253, (&phi23p), (&phi23s));
        }
    }
    let _e258 = R12_;
    let _e260 = R23p;
    r123p = max(sqrt((_e258.x * _e260)), vec3(0f));
    let _e267 = R12_;
    let _e269 = R23s;
    r123s = max(sqrt((_e267.y * _e269)), vec3(0f));
    let _e281 = fd_3;
    distMeters = (_e281.tf_thickness * 0.000000001f);
    let _e287 = eta2_2;
    let _e289 = cosThetaT;
    let _e291 = distMeters;
    opd_2 = (((2f * _e287) * _e289) * _e291);
    let _e294 = T121_;
    let _e296 = mx_square(_e294.x);
    let _e297 = R23p;
    let _e301 = R12_;
    let _e303 = R23p;
    Rs_3 = ((_e296 * _e297) / (vec3(1f) - (_e301.x * _e303)));
    let _e308 = I;
    let _e309 = R12_;
    let _e311 = Rs_3;
    I = (_e308 + (vec3(_e309.x) + _e311));
    let _e315 = Rs_3;
    let _e316 = T121_;
    Cm = (_e315 - vec3(_e316.x));
    loop {
        let _e322 = m_12;
        if !((_e322 <= 2i)) {
            break;
        }
        {
            let _e329 = Cm;
            let _e330 = r123p;
            Cm = (_e329 * _e330);
            let _e333 = m_12;
            let _e335 = opd_2;
            let _e337 = m_12;
            let _e339 = phi23p;
            let _e340 = phi21_;
            let _e345 = mx_eval_sensitivity((f32(_e333) * _e335), (f32(_e337) * (_e339 + vec3(_e340.x))));
            Sm = (2f * _e345);
            let _e347 = I;
            let _e348 = Cm;
            let _e349 = Sm;
            I = (_e347 + (_e348 * _e349));
        }
        continuing {
            let _e326 = m_12;
            m_12 = (_e326 + 1i);
        }
    }
    let _e352 = T121_;
    let _e354 = mx_square(_e352.y);
    let _e355 = R23s;
    let _e359 = R12_;
    let _e361 = R23s;
    Rp_3 = ((_e354 * _e355) / (vec3(1f) - (_e359.y * _e361)));
    let _e366 = I;
    let _e367 = R12_;
    let _e369 = Rp_3;
    I = (_e366 + (vec3(_e367.y) + _e369));
    let _e373 = Rp_3;
    let _e374 = T121_;
    Cm = (_e373 - vec3(_e374.y));
    loop {
        let _e380 = m_13;
        if !((_e380 <= 2i)) {
            break;
        }
        {
            let _e387 = Cm;
            let _e388 = r123s;
            Cm = (_e387 * _e388);
            let _e391 = m_13;
            let _e393 = opd_2;
            let _e395 = m_13;
            let _e397 = phi23s;
            let _e398 = phi21_;
            let _e403 = mx_eval_sensitivity((f32(_e391) * _e393), (f32(_e395) * (_e397 + vec3(_e398.y))));
            Sm = (2f * _e403);
            let _e405 = I;
            let _e406 = Cm;
            let _e407 = Sm;
            I = (_e405 + (_e406 * _e407));
        }
        continuing {
            let _e384 = m_13;
            m_13 = (_e384 + 1i);
        }
    }
    let _e410 = I;
    I = (_e410 * 0.5f);
    let _e413 = XYZ_TO_RGB;
    let _e414 = I;
    let _e415 = mx_matrix_mul_4(_e413, _e414);
    I = clamp(_e415, vec3(0f), vec3(1f));
    let _e421 = I;
    return _e421;
}

fn mx_init_fresnel_dielectric(ior_6: f32, tf_thickness: f32, tf_ior: f32) -> FresnelData {
    var ior_7: f32;
    var tf_thickness_1: f32;
    var tf_ior_1: f32;
    var fd_4: FresnelData;

    ior_7 = ior_6;
    tf_thickness_1 = tf_thickness;
    tf_ior_1 = tf_ior;
    fd_4.model = FRESNEL_MODEL_DIELECTRIC;
    let _e102 = tf_thickness_1;
    fd_4.airy = (_e102 > 0f);
    let _e106 = ior_7;
    fd_4.ior = vec3(_e106);
    fd_4.extinction = vec3(0f);
    fd_4.F0_ = vec3(0f);
    fd_4.F82_ = vec3(0f);
    fd_4.F90_ = vec3(0f);
    fd_4.exponent = 0f;
    let _e123 = tf_thickness_1;
    fd_4.tf_thickness = _e123;
    let _e125 = tf_ior_1;
    fd_4.tf_ior = _e125;
    fd_4.refraction = false;
    let _e128 = fd_4;
    return _e128;
}

fn mx_init_fresnel_conductor(ior_8: vec3<f32>, extinction: vec3<f32>, tf_thickness_2: f32, tf_ior_2: f32) -> FresnelData {
    var ior_9: vec3<f32>;
    var extinction_1: vec3<f32>;
    var tf_thickness_3: f32;
    var tf_ior_3: f32;
    var fd_5: FresnelData;

    ior_9 = ior_8;
    extinction_1 = extinction;
    tf_thickness_3 = tf_thickness_2;
    tf_ior_3 = tf_ior_2;
    fd_5.model = FRESNEL_MODEL_CONDUCTOR;
    let _e104 = tf_thickness_3;
    fd_5.airy = (_e104 > 0f);
    let _e108 = ior_9;
    fd_5.ior = _e108;
    let _e110 = extinction_1;
    fd_5.extinction = _e110;
    fd_5.F0_ = vec3(0f);
    fd_5.F82_ = vec3(0f);
    fd_5.F90_ = vec3(0f);
    fd_5.exponent = 0f;
    let _e123 = tf_thickness_3;
    fd_5.tf_thickness = _e123;
    let _e125 = tf_ior_3;
    fd_5.tf_ior = _e125;
    fd_5.refraction = false;
    let _e128 = fd_5;
    return _e128;
}

fn mx_init_fresnel_schlick(F0_26: vec3<f32>, F82_: vec3<f32>, F90_18: vec3<f32>, exponent_4: f32, tf_thickness_4: f32, tf_ior_4: f32) -> FresnelData {
    var F0_27: vec3<f32>;
    var F82_1: vec3<f32>;
    var F90_19: vec3<f32>;
    var exponent_5: f32;
    var tf_thickness_5: f32;
    var tf_ior_5: f32;
    var fd_6: FresnelData;

    F0_27 = F0_26;
    F82_1 = F82_;
    F90_19 = F90_18;
    exponent_5 = exponent_4;
    tf_thickness_5 = tf_thickness_4;
    tf_ior_5 = tf_ior_4;
    fd_6.model = FRESNEL_MODEL_SCHLICK;
    let _e108 = tf_thickness_5;
    fd_6.airy = (_e108 > 0f);
    fd_6.ior = vec3(0f);
    fd_6.extinction = vec3(0f);
    let _e118 = F0_27;
    fd_6.F0_ = _e118;
    let _e120 = F82_1;
    fd_6.F82_ = _e120;
    let _e122 = F90_19;
    fd_6.F90_ = _e122;
    let _e124 = exponent_5;
    fd_6.exponent = _e124;
    let _e126 = tf_thickness_5;
    fd_6.tf_thickness = _e126;
    let _e128 = tf_ior_5;
    fd_6.tf_ior = _e128;
    fd_6.refraction = false;
    let _e131 = fd_6;
    return _e131;
}

fn mx_compute_fresnel(cosTheta_30: f32, fd_7: FresnelData) -> vec3<f32> {
    var cosTheta_31: f32;
    var fd_8: FresnelData;

    cosTheta_31 = cosTheta_30;
    fd_8 = fd_7;
    let _e97 = fd_8;
    if _e97.airy {
        {
            let _e99 = cosTheta_31;
            let _e100 = fd_8;
            let _e101 = mx_fresnel_airy(_e99, _e100);
            return _e101;
        }
    } else {
        let _e102 = fd_8;
        if (_e102.model == FRESNEL_MODEL_DIELECTRIC) {
            {
                let _e105 = cosTheta_31;
                let _e106 = fd_8;
                let _e109 = mx_fresnel_dielectric(_e105, _e106.ior.x);
                return vec3(_e109);
            }
        } else {
            let _e111 = fd_8;
            if (_e111.model == FRESNEL_MODEL_CONDUCTOR) {
                {
                    let _e114 = cosTheta_31;
                    let _e115 = fd_8;
                    let _e117 = fd_8;
                    let _e119 = mx_fresnel_conductor(_e114, _e115.ior, _e117.extinction);
                    return _e119;
                }
            } else {
                {
                    let _e120 = cosTheta_31;
                    let _e121 = fd_8;
                    let _e122 = mx_fresnel_hoffman_schlick(_e120, _e121);
                    return _e122;
                }
            }
        }
    }
}

fn mx_refraction_solid_sphere(R: vec3<f32>, N_4: vec3<f32>, ior_10: f32) -> vec3<f32> {
    var R_1: vec3<f32>;
    var N_5: vec3<f32>;
    var ior_11: f32;
    var N1_: vec3<f32>;

    R_1 = R;
    N_5 = N_4;
    ior_11 = ior_10;
    let _e99 = R_1;
    let _e100 = N_5;
    let _e102 = ior_11;
    R_1 = refract(_e99, _e100, (1f / _e102));
    let _e105 = R_1;
    let _e106 = R_1;
    let _e107 = N_5;
    let _e110 = N_5;
    N1_ = normalize(((_e105 * dot(_e106, _e107)) - (_e110 * 0.5f)));
    let _e116 = R_1;
    let _e117 = N1_;
    let _e118 = ior_11;
    return refract(_e116, _e117, _e118);
}

fn mx_latlong_projection(dir: vec3<f32>) -> vec2<f32> {
    var dir_1: vec3<f32>;
    var latitude: f32;
    var longitude: f32;

    dir_1 = dir;
    let _e95 = dir_1;
    latitude = ((-(asin(_e95.y)) * 0.31830987f) + 0.5f);
    let _e106 = dir_1;
    let _e108 = dir_1;
    longitude = (((atan2(_e106.x, -(_e108.z)) * 0.31830987f) * 0.5f) + 0.5f);
    let _e121 = longitude;
    let _e122 = latitude;
    return vec2<f32>(_e121, _e122);
}

fn mx_latlong_map_lookup(dir_2: vec3<f32>, transform: mat4x4<f32>, lod: f32, tex_texture: texture_2d<f32>, tex_sampler: sampler) -> vec3<f32> {
    var dir_3: vec3<f32>;
    var transform_1: mat4x4<f32>;
    var lod_1: f32;
    var envDir: vec3<f32>;
    var uv: vec2<f32>;

    dir_3 = dir_2;
    transform_1 = transform;
    lod_1 = lod;
    let _e101 = transform_1;
    let _e102 = dir_3;
    let _e108 = mx_matrix_mul_5(_e101, vec4<f32>(_e102.x, _e102.y, _e102.z, 0f));
    envDir = normalize(_e108.xyz);
    let _e112 = envDir;
    let _e113 = mx_latlong_projection(_e112);
    uv = _e113;
    let _e115 = uv;
    let _e116 = lod_1;
    let _e117 = textureSampleLevel(tex_texture, tex_sampler, _e115, _e116);
    return _e117.xyz;
}

fn mx_latlong_compute_lod(dir_4: vec3<f32>, pdf: f32, maxMipLevel: f32, envSamples: i32) -> f32 {
    var dir_5: vec3<f32>;
    var pdf_1: f32;
    var maxMipLevel_1: f32;
    var envSamples_1: i32;
    var MIP_LEVEL_OFFSET: f32 = 1.5f;
    var effectiveMaxMipLevel: f32;
    var distortion: f32;

    dir_5 = dir_4;
    pdf_1 = pdf;
    maxMipLevel_1 = maxMipLevel;
    envSamples_1 = envSamples;
    let _e103 = maxMipLevel_1;
    let _e104 = MIP_LEVEL_OFFSET;
    effectiveMaxMipLevel = (_e103 - _e104);
    let _e108 = dir_5;
    let _e110 = mx_square(_e108.y);
    distortion = sqrt((1f - _e110));
    let _e114 = effectiveMaxMipLevel;
    let _e116 = envSamples_1;
    let _e118 = pdf_1;
    let _e120 = distortion;
    return max((_e114 - (0.5f * log2(((f32(_e116) * _e118) * _e120)))), 0f);
}

fn mx_environment_radiance(N_6: vec3<f32>, V_6: vec3<f32>, X_1: vec3<f32>, alpha_24: vec2<f32>, distribution: i32, fd_9: FresnelData) -> vec3<f32> {
    var N_7: vec3<f32>;
    var V_7: vec3<f32>;
    var X_2: vec3<f32>;
    var alpha_25: vec2<f32>;
    var distribution_1: i32;
    var fd_10: FresnelData;
    var Y_1: vec3<f32>;
    var tangentToWorld: mat3x3<f32>;
    var NdotV_16: f32;
    var avgAlpha: f32;
    var G1V: f32;
    var radiance: vec3<f32> = vec3(0f);
    var envRadianceSamples: i32;
    var i_5: i32 = 0i;
    var Xi_7: vec2<f32>;
    var H_4: vec3<f32>;
    var local_8: vec3<f32>;
    var L_1: vec3<f32>;
    var NdotL_3: f32;
    var VdotH_1: f32;
    var Lw: vec3<f32>;
    var pdf_2: f32;
    var lod_2: f32;
    var sampleColor: vec3<f32>;
    var F: vec3<f32>;
    var G: f32;
    var local_9: vec3<f32>;
    var FG: vec3<f32>;
    var local_10: vec3<f32>;

    N_7 = N_6;
    V_7 = V_6;
    X_2 = X_1;
    alpha_25 = alpha_24;
    distribution_1 = distribution;
    fd_10 = fd_9;
    let _e105 = X_2;
    let _e106 = X_2;
    let _e107 = N_7;
    let _e109 = N_7;
    X_2 = normalize((_e105 - (dot(_e106, _e107) * _e109)));
    let _e113 = N_7;
    let _e114 = X_2;
    Y_1 = cross(_e113, _e114);
    let _e117 = X_2;
    let _e118 = Y_1;
    let _e119 = N_7;
    tangentToWorld = mat3x3<f32>(vec3<f32>(_e117.x, _e117.y, _e117.z), vec3<f32>(_e118.x, _e118.y, _e118.z), vec3<f32>(_e119.x, _e119.y, _e119.z));
    let _e134 = V_7;
    let _e135 = X_2;
    let _e137 = V_7;
    let _e138 = Y_1;
    let _e140 = V_7;
    let _e141 = N_7;
    V_7 = vec3<f32>(dot(_e134, _e135), dot(_e137, _e138), dot(_e140, _e141));
    let _e144 = V_7;
    NdotV_16 = clamp(_e144.z, 0.00000001f, 1f);
    let _e150 = alpha_25;
    let _e151 = mx_average_alpha(_e150);
    avgAlpha = _e151;
    let _e153 = NdotV_16;
    let _e154 = avgAlpha;
    let _e155 = mx_ggx_smith_G1_(_e153, _e154);
    G1V = _e155;
    let _e160 = global.u_envRadianceSamples;
    envRadianceSamples = _e160;
    loop {
        let _e164 = i_5;
        let _e165 = envRadianceSamples;
        if !((_e164 < _e165)) {
            break;
        }
        {
            let _e171 = i_5;
            let _e172 = envRadianceSamples;
            let _e173 = mx_spherical_fibonacci(_e171, _e172);
            Xi_7 = _e173;
            let _e175 = Xi_7;
            let _e176 = V_7;
            let _e177 = alpha_25;
            let _e178 = mx_ggx_importance_sample_VNDF(_e175, _e176, _e177);
            H_4 = _e178;
            let _e180 = fd_10;
            if _e180.refraction {
                let _e182 = V_7;
                let _e184 = H_4;
                let _e185 = fd_10;
                let _e188 = mx_refraction_solid_sphere(-(_e182), _e184, _e185.ior.x);
                local_8 = _e188;
            } else {
                let _e189 = V_7;
                let _e190 = H_4;
                local_8 = -(reflect(_e189, _e190));
            }
            let _e194 = local_8;
            L_1 = _e194;
            let _e196 = L_1;
            NdotL_3 = clamp(_e196.z, 0.00000001f, 1f);
            let _e202 = V_7;
            let _e203 = H_4;
            VdotH_1 = clamp(dot(_e202, _e203), 0.00000001f, 1f);
            let _e209 = tangentToWorld;
            let _e210 = L_1;
            let _e211 = mx_matrix_mul_4(_e209, _e210);
            Lw = _e211;
            let _e213 = H_4;
            let _e214 = alpha_25;
            let _e215 = mx_ggx_NDF(_e213, _e214);
            let _e216 = G1V;
            let _e219 = NdotV_16;
            pdf_2 = ((_e215 * _e216) / (4f * _e219));
            let _e223 = Lw;
            let _e224 = pdf_2;
            let _e225 = global.u_envRadianceMips;
            let _e229 = envRadianceSamples;
            let _e230 = mx_latlong_compute_lod(_e223, _e224, f32((_e225 - 1i)), _e229);
            lod_2 = _e230;
            let _e232 = Lw;
            let _e233 = global.u_envMatrix;
            let _e234 = lod_2;
            let _e235 = mx_latlong_map_lookup(_e232, _e233, _e234, u_envRadiance_texture, u_envRadiance_sampler);
            sampleColor = _e235;
            let _e237 = VdotH_1;
            let _e238 = fd_10;
            let _e239 = mx_compute_fresnel(_e237, _e238);
            F = _e239;
            let _e241 = NdotL_3;
            let _e242 = NdotV_16;
            let _e243 = avgAlpha;
            let _e244 = mx_ggx_smith_G2_(_e241, _e242, _e243);
            G = _e244;
            let _e246 = fd_10;
            if _e246.refraction {
                let _e250 = F;
                local_9 = (vec3(1f) - _e250);
            } else {
                let _e252 = F;
                let _e253 = G;
                local_9 = (_e252 * _e253);
            }
            let _e256 = local_9;
            FG = _e256;
            let _e258 = radiance;
            let _e259 = sampleColor;
            let _e260 = FG;
            radiance = (_e258 + (_e259 * _e260));
        }
        continuing {
            let _e168 = i_5;
            i_5 = (_e168 + 1i);
        }
    }
    let _e263 = radiance;
    let _e264 = G1V;
    let _e265 = envRadianceSamples;
    radiance = (_e263 / vec3((_e264 * f32(_e265))));
    let _e270 = global.u_envRadianceSamples;
    if (_e270 == 0i) {
        local_10 = vec3(0f);
    } else {
        let _e275 = radiance;
        local_10 = _e275;
    }
    let _e277 = local_10;
    let _e278 = global.u_envLightIntensity;
    return (_e277 * _e278);
}

fn mx_environment_irradiance(N_8: vec3<f32>) -> vec3<f32> {
    var N_9: vec3<f32>;
    var Li: vec3<f32>;

    N_9 = N_8;
    let _e95 = N_9;
    let _e96 = global.u_envMatrix;
    let _e98 = mx_latlong_map_lookup(_e95, _e96, 0f, u_envIrradiance_texture, u_envIrradiance_sampler);
    Li = _e98;
    let _e100 = Li;
    let _e101 = global.u_envLightIntensity;
    return (_e100 * _e101);
}

fn mx_surface_transmission(N_10: vec3<f32>, V_8: vec3<f32>, X_3: vec3<f32>, alpha_26: vec2<f32>, distribution_2: i32, fd_11: FresnelData, tint: vec3<f32>) -> vec3<f32> {
    var N_11: vec3<f32>;
    var V_9: vec3<f32>;
    var X_4: vec3<f32>;
    var alpha_27: vec2<f32>;
    var distribution_3: i32;
    var fd_12: FresnelData;
    var tint_1: vec3<f32>;

    N_11 = N_10;
    V_9 = V_8;
    X_4 = X_3;
    alpha_27 = alpha_26;
    distribution_3 = distribution_2;
    fd_12 = fd_11;
    tint_1 = tint;
    fd_12.refraction = true;
    let _e109 = global.u_refractionTwoSided;
    if bool(_e109) {
        {
            let _e111 = tint_1;
            let _e112 = mx_square_2(_e111);
            tint_1 = _e112;
        }
    }
    let _e113 = N_11;
    let _e114 = V_9;
    let _e115 = X_4;
    let _e116 = alpha_27;
    let _e117 = distribution_3;
    let _e118 = fd_12;
    let _e119 = mx_environment_radiance(_e113, _e114, _e115, _e116, _e117, _e118);
    let _e120 = tint_1;
    return (_e119 * _e120);
}

fn mx_directional_light(light: LightData, position: vec3<f32>, result: ptr<function, lightshader>) {
    var light_1: LightData;
    var position_1: vec3<f32>;

    light_1 = light;
    position_1 = position;
    let _e101 = light_1;
    (*result).direction = -(_e101.direction);
    let _e105 = light_1;
    let _e107 = light_1;
    (*result).intensity = (_e105.color * _e107.intensity);
    return;
}

fn numActiveLightSources() -> i32 {
    let _e95 = global.u_numActiveLightSources;
    return min(_e95, 1i);
}

fn sampleLightSource(light_2: LightData, position_2: vec3<f32>, result_1: ptr<function, lightshader>) {
    var light_3: LightData;
    var position_3: vec3<f32>;

    light_3 = light_2;
    position_3 = position_2;
    (*result_1).intensity = vec3<f32>(0f, 0f, 0f);
    (*result_1).direction = vec3<f32>(0f, 0f, 0f);
    let _e110 = light_3;
    if (_e110.light_type == 1i) {
        {
            let _e114 = light_3;
            let _e115 = position_3;
            mx_directional_light(_e114, _e115, result_1);
            return;
        }
    } else {
        return;
    }
}

fn NG_switch_floatI(in1_: f32, in11_: f32, in12_: f32, in13_: f32, in14_: f32, in15_: f32, in16_: f32, in17_: f32, in18_: f32, in19_: f32, value2_: i32, out1_: ptr<function, f32>) {
    var in1_1: f32;
    var in11_1: f32;
    var in12_1: f32;
    var in13_1: f32;
    var in14_1: f32;
    var in15_1: f32;
    var in16_1: f32;
    var in17_1: f32;
    var in18_1: f32;
    var in19_1: f32;
    var value2_1: i32;
    var ifgreater_10_value1_tmp: i32 = 10i;
    var ifgreater_10_in2_tmp: f32 = 0f;
    var local_11: f32;
    var ifgreater_10_out: f32;
    var ifgreater_9_value1_tmp: i32 = 9i;
    var local_12: f32;
    var ifgreater_9_out: f32;
    var ifgreater_8_value1_tmp: i32 = 8i;
    var local_13: f32;
    var ifgreater_8_out: f32;
    var ifgreater_7_value1_tmp: i32 = 7i;
    var local_14: f32;
    var ifgreater_7_out: f32;
    var ifgreater_6_value1_tmp: i32 = 6i;
    var local_15: f32;
    var ifgreater_6_out: f32;
    var ifgreater_5_value1_tmp: i32 = 5i;
    var local_16: f32;
    var ifgreater_5_out: f32;
    var ifgreater_4_value1_tmp: i32 = 4i;
    var local_17: f32;
    var ifgreater_4_out: f32;
    var ifgreater_3_value1_tmp: i32 = 3i;
    var local_18: f32;
    var ifgreater_3_out: f32;
    var ifgreater_2_value1_tmp: i32 = 2i;
    var local_19: f32;
    var ifgreater_2_out: f32;
    var ifgreater_1_value1_tmp: i32 = 1i;
    var local_20: f32;
    var ifgreater_1_out: f32;

    in1_1 = in1_;
    in11_1 = in11_;
    in12_1 = in12_;
    in13_1 = in13_;
    in14_1 = in14_;
    in15_1 = in15_;
    in16_1 = in16_;
    in17_1 = in17_;
    in18_1 = in18_;
    in19_1 = in19_;
    value2_1 = value2_;
    let _e122 = ifgreater_10_value1_tmp;
    let _e123 = value2_1;
    if (_e122 > _e123) {
        let _e125 = in19_1;
        local_11 = _e125;
    } else {
        let _e126 = ifgreater_10_in2_tmp;
        local_11 = _e126;
    }
    let _e128 = local_11;
    ifgreater_10_out = _e128;
    let _e132 = ifgreater_9_value1_tmp;
    let _e133 = value2_1;
    if (_e132 > _e133) {
        let _e135 = in18_1;
        local_12 = _e135;
    } else {
        let _e136 = ifgreater_10_out;
        local_12 = _e136;
    }
    let _e138 = local_12;
    ifgreater_9_out = _e138;
    let _e142 = ifgreater_8_value1_tmp;
    let _e143 = value2_1;
    if (_e142 > _e143) {
        let _e145 = in17_1;
        local_13 = _e145;
    } else {
        let _e146 = ifgreater_9_out;
        local_13 = _e146;
    }
    let _e148 = local_13;
    ifgreater_8_out = _e148;
    let _e152 = ifgreater_7_value1_tmp;
    let _e153 = value2_1;
    if (_e152 > _e153) {
        let _e155 = in16_1;
        local_14 = _e155;
    } else {
        let _e156 = ifgreater_8_out;
        local_14 = _e156;
    }
    let _e158 = local_14;
    ifgreater_7_out = _e158;
    let _e162 = ifgreater_6_value1_tmp;
    let _e163 = value2_1;
    if (_e162 > _e163) {
        let _e165 = in15_1;
        local_15 = _e165;
    } else {
        let _e166 = ifgreater_7_out;
        local_15 = _e166;
    }
    let _e168 = local_15;
    ifgreater_6_out = _e168;
    let _e172 = ifgreater_5_value1_tmp;
    let _e173 = value2_1;
    if (_e172 > _e173) {
        let _e175 = in14_1;
        local_16 = _e175;
    } else {
        let _e176 = ifgreater_6_out;
        local_16 = _e176;
    }
    let _e178 = local_16;
    ifgreater_5_out = _e178;
    let _e182 = ifgreater_4_value1_tmp;
    let _e183 = value2_1;
    if (_e182 > _e183) {
        let _e185 = in13_1;
        local_17 = _e185;
    } else {
        let _e186 = ifgreater_5_out;
        local_17 = _e186;
    }
    let _e188 = local_17;
    ifgreater_4_out = _e188;
    let _e192 = ifgreater_3_value1_tmp;
    let _e193 = value2_1;
    if (_e192 > _e193) {
        let _e195 = in12_1;
        local_18 = _e195;
    } else {
        let _e196 = ifgreater_4_out;
        local_18 = _e196;
    }
    let _e198 = local_18;
    ifgreater_3_out = _e198;
    let _e202 = ifgreater_2_value1_tmp;
    let _e203 = value2_1;
    if (_e202 > _e203) {
        let _e205 = in11_1;
        local_19 = _e205;
    } else {
        let _e206 = ifgreater_3_out;
        local_19 = _e206;
    }
    let _e208 = local_19;
    ifgreater_2_out = _e208;
    let _e212 = ifgreater_1_value1_tmp;
    let _e213 = value2_1;
    if (_e212 > _e213) {
        let _e215 = in1_1;
        local_20 = _e215;
    } else {
        let _e216 = ifgreater_2_out;
        local_20 = _e216;
    }
    let _e218 = local_20;
    ifgreater_1_out = _e218;
    let _e220 = ifgreater_1_out;
    (*out1_) = _e220;
    return;
}

fn adsk_NG_adsk_roughness_adjust(in2_: f32, in21_: f32, out1_1: ptr<function, f32>) {
    var in2_1: f32;
    var in21_1: f32;
    var oneminus_aniso_in1_tmp: f32 = 1f;
    var oneminus_aniso_out: f32;
    var sqrt_oneminus_aniso_out: f32;
    var adjusted_roughness_out: f32;

    in2_1 = in2_;
    in21_1 = in21_;
    let _e102 = oneminus_aniso_in1_tmp;
    let _e103 = in21_1;
    oneminus_aniso_out = (_e102 - _e103);
    let _e106 = oneminus_aniso_out;
    sqrt_oneminus_aniso_out = sqrt(_e106);
    let _e109 = sqrt_oneminus_aniso_out;
    let _e110 = in2_1;
    adjusted_roughness_out = (_e109 * _e110);
    let _e113 = adjusted_roughness_out;
    (*out1_1) = _e113;
    return;
}

fn adsk_NG_adsk_f0_to_ior(in1_2: f32, out1_2: ptr<function, f32>) {
    var in1_3: f32;
    var sqrt_f0_out: f32;
    var oneplus_sqrt_f0_in1_tmp: f32 = 1f;
    var oneplus_sqrt_f0_out: f32;
    var oneminus_sqrt_f0_in1_tmp: f32 = 1f;
    var oneminus_sqrt_f0_out: f32;
    var ior_value_out: f32;

    in1_3 = in1_2;
    let _e98 = in1_3;
    sqrt_f0_out = sqrt(_e98);
    let _e103 = oneplus_sqrt_f0_in1_tmp;
    let _e104 = sqrt_f0_out;
    oneplus_sqrt_f0_out = (_e103 + _e104);
    let _e109 = oneminus_sqrt_f0_in1_tmp;
    let _e110 = sqrt_f0_out;
    oneminus_sqrt_f0_out = (_e109 - _e110);
    let _e113 = oneplus_sqrt_f0_out;
    let _e114 = oneminus_sqrt_f0_out;
    ior_value_out = (_e113 / _e114);
    let _e117 = ior_value_out;
    (*out1_2) = _e117;
    return;
}

fn mx_luminance_color3_(_in: vec3<f32>, lumacoeffs: vec3<f32>, result_2: ptr<function, vec3<f32>>) {
    var _in_1: vec3<f32>;
    var lumacoeffs_1: vec3<f32>;

    _in_1 = _in;
    lumacoeffs_1 = lumacoeffs;
    let _e100 = _in_1;
    let _e101 = lumacoeffs_1;
    (*result_2) = vec3(dot(_e100, _e101));
    return;
}

fn adsk_NG_adsk_anisotropy_adjust(in2_2: f32, out1_3: ptr<function, f32>) {
    var in2_3: f32;
    var twotimes_aniso_in1_tmp: f32 = 2f;
    var twotimes_aniso_out: f32;
    var aniso_squared_out: f32;
    var adjusted_anisotropy_out: f32;

    in2_3 = in2_2;
    let _e100 = twotimes_aniso_in1_tmp;
    let _e101 = in2_3;
    twotimes_aniso_out = (_e100 * _e101);
    let _e104 = in2_3;
    let _e105 = in2_3;
    aniso_squared_out = (_e104 * _e105);
    let _e108 = twotimes_aniso_out;
    let _e109 = aniso_squared_out;
    adjusted_anisotropy_out = (_e108 - _e109);
    let _e112 = adjusted_anisotropy_out;
    (*out1_3) = _e112;
    return;
}

fn adsk_NG_adsk_rotation_normalize(in1_4: f32, out1_4: ptr<function, f32>) {
    var in1_5: f32;
    var div_rotation_in2_tmp: f32 = 360f;
    var div_rotation_out: f32;
    var normalized_rotation_in2_tmp: f32 = 1f;
    var normalized_rotation_out: f32;

    in1_5 = in1_4;
    let _e100 = in1_5;
    let _e101 = div_rotation_in2_tmp;
    div_rotation_out = (_e100 / _e101);
    let _e106 = div_rotation_out;
    let _e107 = normalized_rotation_in2_tmp;
    normalized_rotation_out = (_e106 - (floor((_e106 / _e107)) * _e107));
    let _e113 = normalized_rotation_out;
    (*out1_4) = _e113;
    return;
}

fn NG_switch_color3I(in1_6: vec3<f32>, in11_2: vec3<f32>, in12_2: vec3<f32>, in13_2: vec3<f32>, in14_2: vec3<f32>, in15_2: vec3<f32>, in16_2: vec3<f32>, in17_2: vec3<f32>, in18_2: vec3<f32>, in19_2: vec3<f32>, value2_2: i32, out1_5: ptr<function, vec3<f32>>) {
    var in1_7: vec3<f32>;
    var in11_3: vec3<f32>;
    var in12_3: vec3<f32>;
    var in13_3: vec3<f32>;
    var in14_3: vec3<f32>;
    var in15_3: vec3<f32>;
    var in16_3: vec3<f32>;
    var in17_3: vec3<f32>;
    var in18_3: vec3<f32>;
    var in19_3: vec3<f32>;
    var value2_3: i32;
    var ifgreater_10_value1_tmp_1: i32 = 10i;
    var ifgreater_10_in2_tmp_1: vec3<f32> = vec3<f32>(0f, 0f, 0f);
    var local_21: vec3<f32>;
    var ifgreater_10_out_1: vec3<f32>;
    var ifgreater_9_value1_tmp_1: i32 = 9i;
    var local_22: vec3<f32>;
    var ifgreater_9_out_1: vec3<f32>;
    var ifgreater_8_value1_tmp_1: i32 = 8i;
    var local_23: vec3<f32>;
    var ifgreater_8_out_1: vec3<f32>;
    var ifgreater_7_value1_tmp_1: i32 = 7i;
    var local_24: vec3<f32>;
    var ifgreater_7_out_1: vec3<f32>;
    var ifgreater_6_value1_tmp_1: i32 = 6i;
    var local_25: vec3<f32>;
    var ifgreater_6_out_1: vec3<f32>;
    var ifgreater_5_value1_tmp_1: i32 = 5i;
    var local_26: vec3<f32>;
    var ifgreater_5_out_1: vec3<f32>;
    var ifgreater_4_value1_tmp_1: i32 = 4i;
    var local_27: vec3<f32>;
    var ifgreater_4_out_1: vec3<f32>;
    var ifgreater_3_value1_tmp_1: i32 = 3i;
    var local_28: vec3<f32>;
    var ifgreater_3_out_1: vec3<f32>;
    var ifgreater_2_value1_tmp_1: i32 = 2i;
    var local_29: vec3<f32>;
    var ifgreater_2_out_1: vec3<f32>;
    var ifgreater_1_value1_tmp_1: i32 = 1i;
    var local_30: vec3<f32>;
    var ifgreater_1_out_1: vec3<f32>;

    in1_7 = in1_6;
    in11_3 = in11_2;
    in12_3 = in12_2;
    in13_3 = in13_2;
    in14_3 = in14_2;
    in15_3 = in15_2;
    in16_3 = in16_2;
    in17_3 = in17_2;
    in18_3 = in18_2;
    in19_3 = in19_2;
    value2_3 = value2_2;
    let _e125 = ifgreater_10_value1_tmp_1;
    let _e126 = value2_3;
    if (_e125 > _e126) {
        let _e128 = in19_3;
        local_21 = _e128;
    } else {
        let _e129 = ifgreater_10_in2_tmp_1;
        local_21 = _e129;
    }
    let _e131 = local_21;
    ifgreater_10_out_1 = _e131;
    let _e135 = ifgreater_9_value1_tmp_1;
    let _e136 = value2_3;
    if (_e135 > _e136) {
        let _e138 = in18_3;
        local_22 = _e138;
    } else {
        let _e139 = ifgreater_10_out_1;
        local_22 = _e139;
    }
    let _e141 = local_22;
    ifgreater_9_out_1 = _e141;
    let _e145 = ifgreater_8_value1_tmp_1;
    let _e146 = value2_3;
    if (_e145 > _e146) {
        let _e148 = in17_3;
        local_23 = _e148;
    } else {
        let _e149 = ifgreater_9_out_1;
        local_23 = _e149;
    }
    let _e151 = local_23;
    ifgreater_8_out_1 = _e151;
    let _e155 = ifgreater_7_value1_tmp_1;
    let _e156 = value2_3;
    if (_e155 > _e156) {
        let _e158 = in16_3;
        local_24 = _e158;
    } else {
        let _e159 = ifgreater_8_out_1;
        local_24 = _e159;
    }
    let _e161 = local_24;
    ifgreater_7_out_1 = _e161;
    let _e165 = ifgreater_6_value1_tmp_1;
    let _e166 = value2_3;
    if (_e165 > _e166) {
        let _e168 = in15_3;
        local_25 = _e168;
    } else {
        let _e169 = ifgreater_7_out_1;
        local_25 = _e169;
    }
    let _e171 = local_25;
    ifgreater_6_out_1 = _e171;
    let _e175 = ifgreater_5_value1_tmp_1;
    let _e176 = value2_3;
    if (_e175 > _e176) {
        let _e178 = in14_3;
        local_26 = _e178;
    } else {
        let _e179 = ifgreater_6_out_1;
        local_26 = _e179;
    }
    let _e181 = local_26;
    ifgreater_5_out_1 = _e181;
    let _e185 = ifgreater_4_value1_tmp_1;
    let _e186 = value2_3;
    if (_e185 > _e186) {
        let _e188 = in13_3;
        local_27 = _e188;
    } else {
        let _e189 = ifgreater_5_out_1;
        local_27 = _e189;
    }
    let _e191 = local_27;
    ifgreater_4_out_1 = _e191;
    let _e195 = ifgreater_3_value1_tmp_1;
    let _e196 = value2_3;
    if (_e195 > _e196) {
        let _e198 = in12_3;
        local_28 = _e198;
    } else {
        let _e199 = ifgreater_4_out_1;
        local_28 = _e199;
    }
    let _e201 = local_28;
    ifgreater_3_out_1 = _e201;
    let _e205 = ifgreater_2_value1_tmp_1;
    let _e206 = value2_3;
    if (_e205 > _e206) {
        let _e208 = in11_3;
        local_29 = _e208;
    } else {
        let _e209 = ifgreater_3_out_1;
        local_29 = _e209;
    }
    let _e211 = local_29;
    ifgreater_2_out_1 = _e211;
    let _e215 = ifgreater_1_value1_tmp_1;
    let _e216 = value2_3;
    if (_e215 > _e216) {
        let _e218 = in1_7;
        local_30 = _e218;
    } else {
        let _e219 = ifgreater_2_out_1;
        local_30 = _e219;
    }
    let _e221 = local_30;
    ifgreater_1_out_1 = _e221;
    let _e223 = ifgreater_1_out_1;
    (*out1_5) = _e223;
    return;
}

fn NG_switch_vector3I(in1_8: vec3<f32>, in11_4: vec3<f32>, in12_4: vec3<f32>, in13_4: vec3<f32>, in14_4: vec3<f32>, in15_4: vec3<f32>, in16_4: vec3<f32>, in17_4: vec3<f32>, in18_4: vec3<f32>, in19_4: vec3<f32>, value2_4: i32, out1_6: ptr<function, vec3<f32>>) {
    var in1_9: vec3<f32>;
    var in11_5: vec3<f32>;
    var in12_5: vec3<f32>;
    var in13_5: vec3<f32>;
    var in14_5: vec3<f32>;
    var in15_5: vec3<f32>;
    var in16_5: vec3<f32>;
    var in17_5: vec3<f32>;
    var in18_5: vec3<f32>;
    var in19_5: vec3<f32>;
    var value2_5: i32;
    var ifgreater_10_value1_tmp_2: i32 = 10i;
    var ifgreater_10_in2_tmp_2: vec3<f32> = vec3<f32>(0f, 0f, 0f);
    var local_31: vec3<f32>;
    var ifgreater_10_out_2: vec3<f32>;
    var ifgreater_9_value1_tmp_2: i32 = 9i;
    var local_32: vec3<f32>;
    var ifgreater_9_out_2: vec3<f32>;
    var ifgreater_8_value1_tmp_2: i32 = 8i;
    var local_33: vec3<f32>;
    var ifgreater_8_out_2: vec3<f32>;
    var ifgreater_7_value1_tmp_2: i32 = 7i;
    var local_34: vec3<f32>;
    var ifgreater_7_out_2: vec3<f32>;
    var ifgreater_6_value1_tmp_2: i32 = 6i;
    var local_35: vec3<f32>;
    var ifgreater_6_out_2: vec3<f32>;
    var ifgreater_5_value1_tmp_2: i32 = 5i;
    var local_36: vec3<f32>;
    var ifgreater_5_out_2: vec3<f32>;
    var ifgreater_4_value1_tmp_2: i32 = 4i;
    var local_37: vec3<f32>;
    var ifgreater_4_out_2: vec3<f32>;
    var ifgreater_3_value1_tmp_2: i32 = 3i;
    var local_38: vec3<f32>;
    var ifgreater_3_out_2: vec3<f32>;
    var ifgreater_2_value1_tmp_2: i32 = 2i;
    var local_39: vec3<f32>;
    var ifgreater_2_out_2: vec3<f32>;
    var ifgreater_1_value1_tmp_2: i32 = 1i;
    var local_40: vec3<f32>;
    var ifgreater_1_out_2: vec3<f32>;

    in1_9 = in1_8;
    in11_5 = in11_4;
    in12_5 = in12_4;
    in13_5 = in13_4;
    in14_5 = in14_4;
    in15_5 = in15_4;
    in16_5 = in16_4;
    in17_5 = in17_4;
    in18_5 = in18_4;
    in19_5 = in19_4;
    value2_5 = value2_4;
    let _e125 = ifgreater_10_value1_tmp_2;
    let _e126 = value2_5;
    if (_e125 > _e126) {
        let _e128 = in19_5;
        local_31 = _e128;
    } else {
        let _e129 = ifgreater_10_in2_tmp_2;
        local_31 = _e129;
    }
    let _e131 = local_31;
    ifgreater_10_out_2 = _e131;
    let _e135 = ifgreater_9_value1_tmp_2;
    let _e136 = value2_5;
    if (_e135 > _e136) {
        let _e138 = in18_5;
        local_32 = _e138;
    } else {
        let _e139 = ifgreater_10_out_2;
        local_32 = _e139;
    }
    let _e141 = local_32;
    ifgreater_9_out_2 = _e141;
    let _e145 = ifgreater_8_value1_tmp_2;
    let _e146 = value2_5;
    if (_e145 > _e146) {
        let _e148 = in17_5;
        local_33 = _e148;
    } else {
        let _e149 = ifgreater_9_out_2;
        local_33 = _e149;
    }
    let _e151 = local_33;
    ifgreater_8_out_2 = _e151;
    let _e155 = ifgreater_7_value1_tmp_2;
    let _e156 = value2_5;
    if (_e155 > _e156) {
        let _e158 = in16_5;
        local_34 = _e158;
    } else {
        let _e159 = ifgreater_8_out_2;
        local_34 = _e159;
    }
    let _e161 = local_34;
    ifgreater_7_out_2 = _e161;
    let _e165 = ifgreater_6_value1_tmp_2;
    let _e166 = value2_5;
    if (_e165 > _e166) {
        let _e168 = in15_5;
        local_35 = _e168;
    } else {
        let _e169 = ifgreater_7_out_2;
        local_35 = _e169;
    }
    let _e171 = local_35;
    ifgreater_6_out_2 = _e171;
    let _e175 = ifgreater_5_value1_tmp_2;
    let _e176 = value2_5;
    if (_e175 > _e176) {
        let _e178 = in14_5;
        local_36 = _e178;
    } else {
        let _e179 = ifgreater_6_out_2;
        local_36 = _e179;
    }
    let _e181 = local_36;
    ifgreater_5_out_2 = _e181;
    let _e185 = ifgreater_4_value1_tmp_2;
    let _e186 = value2_5;
    if (_e185 > _e186) {
        let _e188 = in13_5;
        local_37 = _e188;
    } else {
        let _e189 = ifgreater_5_out_2;
        local_37 = _e189;
    }
    let _e191 = local_37;
    ifgreater_4_out_2 = _e191;
    let _e195 = ifgreater_3_value1_tmp_2;
    let _e196 = value2_5;
    if (_e195 > _e196) {
        let _e198 = in12_5;
        local_38 = _e198;
    } else {
        let _e199 = ifgreater_4_out_2;
        local_38 = _e199;
    }
    let _e201 = local_38;
    ifgreater_3_out_2 = _e201;
    let _e205 = ifgreater_2_value1_tmp_2;
    let _e206 = value2_5;
    if (_e205 > _e206) {
        let _e208 = in11_5;
        local_39 = _e208;
    } else {
        let _e209 = ifgreater_3_out_2;
        local_39 = _e209;
    }
    let _e211 = local_39;
    ifgreater_2_out_2 = _e211;
    let _e215 = ifgreater_1_value1_tmp_2;
    let _e216 = value2_5;
    if (_e215 > _e216) {
        let _e218 = in1_9;
        local_40 = _e218;
    } else {
        let _e219 = ifgreater_2_out_2;
        local_40 = _e219;
    }
    let _e221 = local_40;
    ifgreater_1_out_2 = _e221;
    let _e223 = ifgreater_1_out_2;
    (*out1_6) = _e223;
    return;
}

fn mx_backface_util(backface_cullingbool: bool, opacity: f32, result_3: ptr<function, vec3<f32>>) {
    var backface_cullingbool_1: bool;
    var opacity_1: f32;
    var isBackface: bool;

    backface_cullingbool_1 = backface_cullingbool;
    opacity_1 = opacity;
    let _e101 = gl_FrontFacing_1;
    let _e103 = backface_cullingbool_1;
    isBackface = (!(_e101) && _e103);
    let _e106 = opacity_1;
    let _e108 = isBackface;
    (*result_3) = select(0f, 1f, (bool(_e106) || _e108));
    return;
}

fn mx_roughness_anisotropy(roughness: f32, anisotropy: f32, result_4: ptr<function, vec2<f32>>) {
    var roughness_1: f32;
    var anisotropy_1: f32;
    var roughness_sqr: f32;
    var aspect: f32;

    roughness_1 = roughness;
    anisotropy_1 = anisotropy;
    let _e101 = roughness_1;
    let _e102 = roughness_1;
    roughness_sqr = clamp((_e101 * _e102), 0.00000001f, 1f);
    let _e108 = anisotropy_1;
    if (_e108 > 0f) {
        {
            let _e112 = anisotropy_1;
            aspect = sqrt((1f - clamp(_e112, 0f, 0.98f)));
            let _e120 = roughness_sqr;
            let _e121 = aspect;
            (*result_4).x = min((_e120 / _e121), 1f);
            let _e126 = roughness_sqr;
            let _e127 = aspect;
            (*result_4).y = (_e126 * _e127);
            return;
        }
    } else {
        {
            let _e130 = roughness_sqr;
            (*result_4).x = _e130;
            let _e132 = roughness_sqr;
            (*result_4).y = _e132;
            return;
        }
    }
}

fn mx_rotationMatrix(axis: vec3<f32>, angle: f32) -> mat4x4<f32> {
    var axis_1: vec3<f32>;
    var angle_1: f32;
    var s: f32;
    var c_2: f32;
    var oc: f32;

    axis_1 = axis;
    angle_1 = angle;
    let _e100 = axis_1;
    axis_1 = normalize(_e100);
    let _e102 = angle_1;
    s = sin(_e102);
    let _e105 = angle_1;
    c_2 = cos(_e105);
    let _e109 = c_2;
    oc = (1f - _e109);
    let _e112 = oc;
    let _e113 = axis_1;
    let _e116 = axis_1;
    let _e119 = c_2;
    let _e121 = oc;
    let _e122 = axis_1;
    let _e125 = axis_1;
    let _e128 = axis_1;
    let _e130 = s;
    let _e133 = oc;
    let _e134 = axis_1;
    let _e137 = axis_1;
    let _e140 = axis_1;
    let _e142 = s;
    let _e146 = oc;
    let _e147 = axis_1;
    let _e150 = axis_1;
    let _e153 = axis_1;
    let _e155 = s;
    let _e158 = oc;
    let _e159 = axis_1;
    let _e162 = axis_1;
    let _e165 = c_2;
    let _e167 = oc;
    let _e168 = axis_1;
    let _e171 = axis_1;
    let _e174 = axis_1;
    let _e176 = s;
    let _e180 = oc;
    let _e181 = axis_1;
    let _e184 = axis_1;
    let _e187 = axis_1;
    let _e189 = s;
    let _e192 = oc;
    let _e193 = axis_1;
    let _e196 = axis_1;
    let _e199 = axis_1;
    let _e201 = s;
    let _e204 = oc;
    let _e205 = axis_1;
    let _e208 = axis_1;
    let _e211 = c_2;
    return mat4x4<f32>(vec4<f32>((((_e112 * _e113.x) * _e116.x) + _e119), (((_e121 * _e122.x) * _e125.y) - (_e128.z * _e130)), (((_e133 * _e134.z) * _e137.x) + (_e140.y * _e142)), 0f), vec4<f32>((((_e146 * _e147.x) * _e150.y) + (_e153.z * _e155)), (((_e158 * _e159.y) * _e162.y) + _e165), (((_e167 * _e168.y) * _e171.z) - (_e174.x * _e176)), 0f), vec4<f32>((((_e180 * _e181.z) * _e184.x) - (_e187.y * _e189)), (((_e192 * _e193.y) * _e196.z) + (_e199.x * _e201)), (((_e204 * _e205.z) * _e208.z) + _e211), 0f), vec4<f32>(0f, 0f, 0f, 1f));
}

fn mx_rotate_vector3_(_in_2: vec3<f32>, amount: f32, axis_2: vec3<f32>, result_5: ptr<function, vec3<f32>>) {
    var _in_3: vec3<f32>;
    var amount_1: f32;
    var axis_3: vec3<f32>;
    var rotationRadians: f32;
    var m_14: mat4x4<f32>;

    _in_3 = _in_2;
    amount_1 = amount;
    axis_3 = axis_2;
    let _e103 = amount_1;
    rotationRadians = radians(_e103);
    let _e106 = axis_3;
    let _e107 = rotationRadians;
    let _e108 = mx_rotationMatrix(_e106, _e107);
    m_14 = _e108;
    let _e110 = m_14;
    let _e111 = _in_3;
    let _e117 = mx_matrix_mul_5(_e110, vec4<f32>(_e111.x, _e111.y, _e111.z, 1f));
    (*result_5) = _e117.xyz;
    return;
}

fn mx_artistic_ior(reflectivity: vec3<f32>, edge_color: vec3<f32>, ior_12: ptr<function, vec3<f32>>, extinction_2: ptr<function, vec3<f32>>) {
    var reflectivity_1: vec3<f32>;
    var edge_color_1: vec3<f32>;
    var r_1: vec3<f32>;
    var r_sqrt: vec3<f32>;
    var n_min: vec3<f32>;
    var n_max: vec3<f32>;
    var np1_: vec3<f32>;
    var nm1_: vec3<f32>;
    var k2_2: vec3<f32>;

    reflectivity_1 = reflectivity;
    edge_color_1 = edge_color;
    let _e102 = reflectivity_1;
    r_1 = clamp(_e102, vec3(0f), vec3(0.99f));
    let _e109 = r_1;
    r_sqrt = sqrt(_e109);
    let _e113 = r_1;
    let _e117 = r_1;
    n_min = ((vec3(1f) - _e113) / (vec3(1f) + _e117));
    let _e123 = r_sqrt;
    let _e127 = r_sqrt;
    n_max = ((vec3(1f) + _e123) / (vec3(1f) - _e127));
    let _e132 = n_max;
    let _e133 = n_min;
    let _e134 = edge_color_1;
    (*ior_12) = mix(_e132, _e133, _e134);
    let _e136 = (*ior_12);
    np1_ = (_e136 + vec3(1f));
    let _e141 = (*ior_12);
    nm1_ = (_e141 - vec3(1f));
    let _e146 = np1_;
    let _e147 = np1_;
    let _e149 = r_1;
    let _e151 = nm1_;
    let _e152 = nm1_;
    let _e156 = r_1;
    k2_2 = ((((_e146 * _e147) * _e149) - (_e151 * _e152)) / (vec3(1f) - _e156));
    let _e161 = k2_2;
    k2_2 = max(_e161, vec3(0f));
    let _e165 = k2_2;
    (*extinction_2) = sqrt(_e165);
    return;
}

fn makeClosureData(closureType: i32, L_2: vec3<f32>, V_10: vec3<f32>, N_12: vec3<f32>, P: vec3<f32>, occlusion: f32) -> ClosureData {
    var closureType_1: i32;
    var L_3: vec3<f32>;
    var V_11: vec3<f32>;
    var N_13: vec3<f32>;
    var P_1: vec3<f32>;
    var occlusion_1: f32;

    closureType_1 = closureType;
    L_3 = L_2;
    V_11 = V_10;
    N_13 = N_12;
    P_1 = P;
    occlusion_1 = occlusion;
    let _e108 = closureType_1;
    let _e109 = L_3;
    let _e110 = V_11;
    let _e111 = N_13;
    let _e112 = P_1;
    let _e113 = occlusion_1;
    return ClosureData(_e108, _e109, _e110, _e111, _e112, _e113);
}

fn mx_mix_bsdf(closureData: ClosureData, fg: BSDF, bg: BSDF, mixValue: f32, result_6: ptr<function, BSDF>) {
    var closureData_1: ClosureData;
    var fg_1: BSDF;
    var bg_1: BSDF;
    var mixValue_1: f32;

    closureData_1 = closureData;
    fg_1 = fg;
    bg_1 = bg;
    mixValue_1 = mixValue;
    let _e106 = bg_1;
    let _e108 = fg_1;
    let _e110 = mixValue_1;
    (*result_6).response = mix(_e106.response, _e108.response, vec3(_e110));
    let _e114 = bg_1;
    let _e116 = fg_1;
    let _e118 = mixValue_1;
    (*result_6).throughput = mix(_e114.throughput, _e116.throughput, vec3(_e118));
    return;
}

fn mx_uniform_edf(closureData_2: ClosureData, color_2: vec3<f32>, result_7: ptr<function, vec3<f32>>) {
    var closureData_3: ClosureData;
    var color_3: vec3<f32>;

    closureData_3 = closureData_2;
    color_3 = color_2;
    let _e101 = closureData_3;
    if (_e101.closureType == 4i) {
        {
            let _e105 = color_3;
            (*result_7) = _e105;
            return;
        }
    } else {
        return;
    }
}

fn mx_multiply_edf_color3_(closureData_4: ClosureData, in1_10: vec3<f32>, in2_4: vec3<f32>, result_8: ptr<function, vec3<f32>>) {
    var closureData_5: ClosureData;
    var in1_11: vec3<f32>;
    var in2_5: vec3<f32>;

    closureData_5 = closureData_4;
    in1_11 = in1_10;
    in2_5 = in2_4;
    let _e103 = in1_11;
    let _e104 = in2_5;
    (*result_8) = (_e103 * _e104);
    return;
}

fn mx_dielectric_bsdf(closureData_6: ClosureData, weight: f32, tint_2: vec3<f32>, ior_13: f32, roughness_2: vec2<f32>, thinfilm_thickness: f32, thinfilm_ior: f32, N_14: vec3<f32>, X_5: vec3<f32>, distribution_4: i32, scatter_mode: i32, bsdf: ptr<function, BSDF>) {
    var closureData_7: ClosureData;
    var weight_1: f32;
    var tint_3: vec3<f32>;
    var ior_14: f32;
    var roughness_3: vec2<f32>;
    var thinfilm_thickness_1: f32;
    var thinfilm_ior_1: f32;
    var N_15: vec3<f32>;
    var X_6: vec3<f32>;
    var distribution_5: i32;
    var scatter_mode_1: i32;
    var V_12: vec3<f32>;
    var L_4: vec3<f32>;
    var NdotV_17: f32;
    var fd_13: FresnelData;
    var F0_28: f32;
    var safeAlpha: vec2<f32>;
    var avgAlpha_1: f32;
    var safeTint: vec3<f32>;
    var Y_2: vec3<f32>;
    var H_5: vec3<f32>;
    var NdotL_4: f32;
    var VdotH_2: f32;
    var Ht: vec3<f32>;
    var F_1: vec3<f32>;
    var D: f32;
    var G_1: f32;
    var comp: vec3<f32>;
    var dirAlbedo: vec3<f32>;
    var F_2: vec3<f32>;
    var comp_1: vec3<f32>;
    var dirAlbedo_1: vec3<f32>;
    var F_3: vec3<f32>;
    var comp_2: vec3<f32>;
    var dirAlbedo_2: vec3<f32>;
    var Li_1: vec3<f32>;

    closureData_7 = closureData_6;
    weight_1 = weight;
    tint_3 = tint_2;
    ior_14 = ior_13;
    roughness_3 = roughness_2;
    thinfilm_thickness_1 = thinfilm_thickness;
    thinfilm_ior_1 = thinfilm_ior;
    N_15 = N_14;
    X_6 = X_5;
    distribution_5 = distribution_4;
    scatter_mode_1 = scatter_mode;
    let _e119 = weight_1;
    if (_e119 < 0.00000001f) {
        {
            return;
        }
    }
    let _e122 = closureData_7;
    let _e126 = scatter_mode_1;
    if ((_e122.closureType != 2i) && (_e126 == 1i)) {
        {
            return;
        }
    }
    let _e130 = closureData_7;
    V_12 = _e130.V;
    let _e133 = closureData_7;
    L_4 = _e133.L;
    let _e136 = N_15;
    let _e137 = V_12;
    let _e138 = mx_forward_facing_normal(_e136, _e137);
    N_15 = _e138;
    let _e139 = N_15;
    let _e140 = V_12;
    NdotV_17 = clamp(dot(_e139, _e140), 0.00000001f, 1f);
    let _e146 = ior_14;
    let _e147 = thinfilm_thickness_1;
    let _e148 = thinfilm_ior_1;
    let _e149 = mx_init_fresnel_dielectric(_e146, _e147, _e148);
    fd_13 = _e149;
    let _e151 = ior_14;
    let _e152 = mx_ior_to_f0_(_e151);
    F0_28 = _e152;
    let _e154 = roughness_3;
    safeAlpha = clamp(_e154, vec2(0.00000001f), vec2(1f));
    let _e161 = safeAlpha;
    let _e162 = mx_average_alpha(_e161);
    avgAlpha_1 = _e162;
    let _e164 = tint_3;
    safeTint = max(_e164, vec3(0f));
    let _e169 = closureData_7;
    if (_e169.closureType == 1i) {
        {
            let _e173 = X_6;
            let _e174 = X_6;
            let _e175 = N_15;
            let _e177 = N_15;
            X_6 = normalize((_e173 - (dot(_e174, _e175) * _e177)));
            let _e181 = N_15;
            let _e182 = X_6;
            Y_2 = cross(_e181, _e182);
            let _e185 = L_4;
            let _e186 = V_12;
            H_5 = normalize((_e185 + _e186));
            let _e190 = N_15;
            let _e191 = L_4;
            NdotL_4 = clamp(dot(_e190, _e191), 0.00000001f, 1f);
            let _e197 = V_12;
            let _e198 = H_5;
            VdotH_2 = clamp(dot(_e197, _e198), 0.00000001f, 1f);
            let _e204 = H_5;
            let _e205 = X_6;
            let _e207 = H_5;
            let _e208 = Y_2;
            let _e210 = H_5;
            let _e211 = N_15;
            Ht = vec3<f32>(dot(_e204, _e205), dot(_e207, _e208), dot(_e210, _e211));
            let _e215 = VdotH_2;
            let _e216 = fd_13;
            let _e217 = mx_compute_fresnel(_e215, _e216);
            F_1 = _e217;
            let _e219 = Ht;
            let _e220 = safeAlpha;
            let _e221 = mx_ggx_NDF(_e219, _e220);
            D = _e221;
            let _e223 = NdotL_4;
            let _e224 = NdotV_17;
            let _e225 = avgAlpha_1;
            let _e226 = mx_ggx_smith_G2_(_e223, _e224, _e225);
            G_1 = _e226;
            let _e228 = NdotV_17;
            let _e229 = avgAlpha_1;
            let _e230 = F_1;
            let _e231 = mx_ggx_energy_compensation(_e228, _e229, _e230);
            comp = _e231;
            let _e233 = NdotV_17;
            let _e234 = avgAlpha_1;
            let _e235 = F0_28;
            let _e237 = mx_ggx_dir_albedo_1(_e233, _e234, _e235, 1f);
            let _e238 = comp;
            dirAlbedo = (_e237 * _e238);
            let _e243 = dirAlbedo;
            let _e244 = weight_1;
            (*bsdf).throughput = (vec3(1f) - (_e243 * _e244));
            let _e249 = D;
            let _e250 = F_1;
            let _e252 = G_1;
            let _e254 = comp;
            let _e256 = safeTint;
            let _e258 = closureData_7;
            let _e261 = weight_1;
            let _e264 = NdotV_17;
            (*bsdf).response = (((((((_e249 * _e250) * _e252) * _e254) * _e256) * _e258.occlusion) * _e261) / vec3((4f * _e264)));
            return;
        }
    } else {
        let _e268 = closureData_7;
        if (_e268.closureType == 2i) {
            {
                let _e272 = NdotV_17;
                let _e273 = fd_13;
                let _e274 = mx_compute_fresnel(_e272, _e273);
                F_2 = _e274;
                let _e276 = NdotV_17;
                let _e277 = avgAlpha_1;
                let _e278 = F_2;
                let _e279 = mx_ggx_energy_compensation(_e276, _e277, _e278);
                comp_1 = _e279;
                let _e281 = NdotV_17;
                let _e282 = avgAlpha_1;
                let _e283 = F0_28;
                let _e285 = mx_ggx_dir_albedo_1(_e281, _e282, _e283, 1f);
                let _e286 = comp_1;
                dirAlbedo_1 = (_e285 * _e286);
                let _e291 = dirAlbedo_1;
                let _e292 = weight_1;
                (*bsdf).throughput = (vec3(1f) - (_e291 * _e292));
                let _e296 = scatter_mode_1;
                if (_e296 != 0i) {
                    {
                        let _e300 = N_15;
                        let _e301 = V_12;
                        let _e302 = X_6;
                        let _e303 = safeAlpha;
                        let _e304 = distribution_5;
                        let _e305 = fd_13;
                        let _e306 = safeTint;
                        let _e307 = mx_surface_transmission(_e300, _e301, _e302, _e303, _e304, _e305, _e306);
                        let _e308 = weight_1;
                        (*bsdf).response = (_e307 * _e308);
                        return;
                    }
                } else {
                    return;
                }
            }
        } else {
            let _e310 = closureData_7;
            if (_e310.closureType == 3i) {
                {
                    let _e314 = NdotV_17;
                    let _e315 = fd_13;
                    let _e316 = mx_compute_fresnel(_e314, _e315);
                    F_3 = _e316;
                    let _e318 = NdotV_17;
                    let _e319 = avgAlpha_1;
                    let _e320 = F_3;
                    let _e321 = mx_ggx_energy_compensation(_e318, _e319, _e320);
                    comp_2 = _e321;
                    let _e323 = NdotV_17;
                    let _e324 = avgAlpha_1;
                    let _e325 = F0_28;
                    let _e327 = mx_ggx_dir_albedo_1(_e323, _e324, _e325, 1f);
                    let _e328 = comp_2;
                    dirAlbedo_2 = (_e327 * _e328);
                    let _e333 = dirAlbedo_2;
                    let _e334 = weight_1;
                    (*bsdf).throughput = (vec3(1f) - (_e333 * _e334));
                    let _e338 = N_15;
                    let _e339 = V_12;
                    let _e340 = X_6;
                    let _e341 = safeAlpha;
                    let _e342 = distribution_5;
                    let _e343 = fd_13;
                    let _e344 = mx_environment_radiance(_e338, _e339, _e340, _e341, _e342, _e343);
                    Li_1 = _e344;
                    let _e347 = Li_1;
                    let _e348 = safeTint;
                    let _e350 = comp_2;
                    let _e352 = weight_1;
                    (*bsdf).response = (((_e347 * _e348) * _e350) * _e352);
                    return;
                }
            } else {
                return;
            }
        }
    }
}

fn mx_conductor_bsdf(closureData_8: ClosureData, weight_2: f32, ior_n: vec3<f32>, ior_k: vec3<f32>, roughness_4: vec2<f32>, thinfilm_thickness_2: f32, thinfilm_ior_2: f32, N_16: vec3<f32>, X_7: vec3<f32>, distribution_6: i32, bsdf_1: ptr<function, BSDF>) {
    var closureData_9: ClosureData;
    var weight_3: f32;
    var ior_n_1: vec3<f32>;
    var ior_k_1: vec3<f32>;
    var roughness_5: vec2<f32>;
    var thinfilm_thickness_3: f32;
    var thinfilm_ior_3: f32;
    var N_17: vec3<f32>;
    var X_8: vec3<f32>;
    var distribution_7: i32;
    var V_13: vec3<f32>;
    var L_5: vec3<f32>;
    var NdotV_18: f32;
    var fd_14: FresnelData;
    var safeAlpha_1: vec2<f32>;
    var avgAlpha_2: f32;
    var Y_3: vec3<f32>;
    var H_6: vec3<f32>;
    var NdotL_5: f32;
    var VdotH_3: f32;
    var Ht_1: vec3<f32>;
    var F_4: vec3<f32>;
    var D_1: f32;
    var G_2: f32;
    var comp_3: vec3<f32>;
    var F_5: vec3<f32>;
    var comp_4: vec3<f32>;
    var Li_2: vec3<f32>;

    closureData_9 = closureData_8;
    weight_3 = weight_2;
    ior_n_1 = ior_n;
    ior_k_1 = ior_k;
    roughness_5 = roughness_4;
    thinfilm_thickness_3 = thinfilm_thickness_2;
    thinfilm_ior_3 = thinfilm_ior_2;
    N_17 = N_16;
    X_8 = X_7;
    distribution_7 = distribution_6;
    (*bsdf_1).throughput = vec3(0f);
    let _e120 = weight_3;
    if (_e120 < 0.00000001f) {
        {
            return;
        }
    }
    let _e123 = closureData_9;
    V_13 = _e123.V;
    let _e126 = closureData_9;
    L_5 = _e126.L;
    let _e129 = N_17;
    let _e130 = V_13;
    let _e131 = mx_forward_facing_normal(_e129, _e130);
    N_17 = _e131;
    let _e132 = N_17;
    let _e133 = V_13;
    NdotV_18 = clamp(dot(_e132, _e133), 0.00000001f, 1f);
    let _e139 = ior_n_1;
    let _e140 = ior_k_1;
    let _e141 = thinfilm_thickness_3;
    let _e142 = thinfilm_ior_3;
    let _e143 = mx_init_fresnel_conductor(_e139, _e140, _e141, _e142);
    fd_14 = _e143;
    let _e145 = roughness_5;
    safeAlpha_1 = clamp(_e145, vec2(0.00000001f), vec2(1f));
    let _e152 = safeAlpha_1;
    let _e153 = mx_average_alpha(_e152);
    avgAlpha_2 = _e153;
    let _e155 = closureData_9;
    if (_e155.closureType == 1i) {
        {
            let _e159 = X_8;
            let _e160 = X_8;
            let _e161 = N_17;
            let _e163 = N_17;
            X_8 = normalize((_e159 - (dot(_e160, _e161) * _e163)));
            let _e167 = N_17;
            let _e168 = X_8;
            Y_3 = cross(_e167, _e168);
            let _e171 = L_5;
            let _e172 = V_13;
            H_6 = normalize((_e171 + _e172));
            let _e176 = N_17;
            let _e177 = L_5;
            NdotL_5 = clamp(dot(_e176, _e177), 0.00000001f, 1f);
            let _e183 = V_13;
            let _e184 = H_6;
            VdotH_3 = clamp(dot(_e183, _e184), 0.00000001f, 1f);
            let _e190 = H_6;
            let _e191 = X_8;
            let _e193 = H_6;
            let _e194 = Y_3;
            let _e196 = H_6;
            let _e197 = N_17;
            Ht_1 = vec3<f32>(dot(_e190, _e191), dot(_e193, _e194), dot(_e196, _e197));
            let _e201 = VdotH_3;
            let _e202 = fd_14;
            let _e203 = mx_compute_fresnel(_e201, _e202);
            F_4 = _e203;
            let _e205 = Ht_1;
            let _e206 = safeAlpha_1;
            let _e207 = mx_ggx_NDF(_e205, _e206);
            D_1 = _e207;
            let _e209 = NdotL_5;
            let _e210 = NdotV_18;
            let _e211 = avgAlpha_2;
            let _e212 = mx_ggx_smith_G2_(_e209, _e210, _e211);
            G_2 = _e212;
            let _e214 = NdotV_18;
            let _e215 = avgAlpha_2;
            let _e216 = F_4;
            let _e217 = mx_ggx_energy_compensation(_e214, _e215, _e216);
            comp_3 = _e217;
            let _e220 = D_1;
            let _e221 = F_4;
            let _e223 = G_2;
            let _e225 = comp_3;
            let _e227 = closureData_9;
            let _e230 = weight_3;
            let _e233 = NdotV_18;
            (*bsdf_1).response = ((((((_e220 * _e221) * _e223) * _e225) * _e227.occlusion) * _e230) / vec3((4f * _e233)));
            return;
        }
    } else {
        let _e237 = closureData_9;
        if (_e237.closureType == 3i) {
            {
                let _e241 = NdotV_18;
                let _e242 = fd_14;
                let _e243 = mx_compute_fresnel(_e241, _e242);
                F_5 = _e243;
                let _e245 = NdotV_18;
                let _e246 = avgAlpha_2;
                let _e247 = F_5;
                let _e248 = mx_ggx_energy_compensation(_e245, _e246, _e247);
                comp_4 = _e248;
                let _e250 = N_17;
                let _e251 = V_13;
                let _e252 = X_8;
                let _e253 = safeAlpha_1;
                let _e254 = distribution_7;
                let _e255 = fd_14;
                let _e256 = mx_environment_radiance(_e250, _e251, _e252, _e253, _e254, _e255);
                Li_2 = _e256;
                let _e259 = Li_2;
                let _e260 = comp_4;
                let _e262 = weight_3;
                (*bsdf_1).response = ((_e259 * _e260) * _e262);
                return;
            }
        } else {
            return;
        }
    }
}

fn mx_oren_nayar_diffuse(NdotV_19: f32, NdotL_6: f32, LdotV: f32, roughness_6: f32) -> f32 {
    var NdotV_20: f32;
    var NdotL_7: f32;
    var LdotV_1: f32;
    var roughness_7: f32;
    var s_1: f32;
    var local_41: f32;
    var stinv: f32;
    var sigma2_: f32;
    var A_1: f32;
    var B_1: f32;

    NdotV_20 = NdotV_19;
    NdotL_7 = NdotL_6;
    LdotV_1 = LdotV;
    roughness_7 = roughness_6;
    let _e106 = LdotV_1;
    let _e107 = NdotL_7;
    let _e108 = NdotV_20;
    s_1 = (_e106 - (_e107 * _e108));
    let _e112 = s_1;
    if (_e112 > 0f) {
        let _e115 = s_1;
        let _e116 = NdotL_7;
        let _e117 = NdotV_20;
        local_41 = (_e115 / max(_e116, _e117));
    } else {
        local_41 = 0f;
    }
    let _e122 = local_41;
    stinv = _e122;
    let _e124 = roughness_7;
    let _e125 = mx_square(_e124);
    sigma2_ = _e125;
    let _e129 = sigma2_;
    let _e130 = sigma2_;
    A_1 = (1f - (0.5f * (_e129 / (_e130 + 0.33f))));
    let _e138 = sigma2_;
    let _e140 = sigma2_;
    B_1 = ((0.45f * _e138) / (_e140 + 0.09f));
    let _e145 = A_1;
    let _e146 = B_1;
    let _e147 = stinv;
    return (_e145 + (_e146 * _e147));
}

fn mx_oren_nayar_diffuse_dir_albedo_analytic(NdotV_21: f32, roughness_8: f32) -> f32 {
    var NdotV_22: f32;
    var roughness_9: f32;
    var r_2: vec2<f32>;

    NdotV_22 = NdotV_21;
    roughness_9 = roughness_8;
    let _e110 = roughness_9;
    let _e118 = NdotV_22;
    let _e120 = roughness_9;
    let _e126 = roughness_9;
    let _e127 = mx_square(_e126);
    r_2 = (((vec2<f32>(1f, 1f) + (vec2<f32>(-0.4297f, -0.6076f) * _e110)) + ((vec2<f32>(-0.7632f, -0.4993f) * _e118) * _e120)) + (vec2<f32>(1.4385f, 2.0315f) * _e127));
    let _e131 = r_2;
    let _e133 = r_2;
    return (_e131.x / _e133.y);
}

fn mx_oren_nayar_diffuse_dir_albedo_table_lookup(NdotV_23: f32, roughness_10: f32) -> f32 {
    var NdotV_24: f32;
    var roughness_11: f32;

    NdotV_24 = NdotV_23;
    roughness_11 = roughness_10;
    return 0f;
}

fn mx_oren_nayar_diffuse_dir_albedo_monte_carlo(NdotV_25: f32, roughness_12: f32) -> f32 {
    var NdotV_26: f32;
    var roughness_13: f32;
    var V_14: vec3<f32>;
    var radiance_1: f32 = 0f;
    var SAMPLE_COUNT_1: i32 = 64i;
    var i_6: i32 = 0i;
    var Xi_8: vec2<f32>;
    var L_6: vec3<f32>;
    var NdotL_8: f32;
    var LdotV_2: f32;
    var reflectance: f32;

    NdotV_26 = NdotV_25;
    roughness_13 = roughness_12;
    let _e102 = NdotV_26;
    NdotV_26 = clamp(_e102, 0.00000001f, 1f);
    let _e107 = NdotV_26;
    let _e108 = mx_square(_e107);
    let _e112 = NdotV_26;
    V_14 = vec3<f32>(sqrt((1f - _e108)), 0f, _e112);
    loop {
        let _e122 = i_6;
        let _e123 = SAMPLE_COUNT_1;
        if !((_e122 < _e123)) {
            break;
        }
        {
            let _e129 = i_6;
            let _e130 = SAMPLE_COUNT_1;
            let _e131 = mx_spherical_fibonacci(_e129, _e130);
            Xi_8 = _e131;
            let _e133 = Xi_8;
            let _e134 = mx_uniform_sample_hemisphere(_e133);
            L_6 = _e134;
            let _e136 = L_6;
            NdotL_8 = clamp(_e136.z, 0.00000001f, 1f);
            let _e142 = L_6;
            let _e143 = V_14;
            LdotV_2 = clamp(dot(_e142, _e143), 0.00000001f, 1f);
            let _e149 = NdotV_26;
            let _e150 = NdotL_8;
            let _e151 = LdotV_2;
            let _e152 = roughness_13;
            let _e153 = mx_oren_nayar_diffuse(_e149, _e150, _e151, _e152);
            reflectance = _e153;
            let _e155 = radiance_1;
            let _e156 = reflectance;
            let _e157 = NdotL_8;
            radiance_1 = (_e155 + (_e156 * _e157));
        }
        continuing {
            let _e126 = i_6;
            i_6 = (_e126 + 1i);
        }
    }
    let _e160 = radiance_1;
    let _e162 = SAMPLE_COUNT_1;
    radiance_1 = (_e160 * (2f / f32(_e162)));
    let _e166 = radiance_1;
    return _e166;
}

fn mx_oren_nayar_diffuse_dir_albedo(NdotV_27: f32, roughness_14: f32) -> f32 {
    var NdotV_28: f32;
    var roughness_15: f32;
    var dirAlbedo_3: f32;

    NdotV_28 = NdotV_27;
    roughness_15 = roughness_14;
    let _e102 = NdotV_28;
    let _e103 = roughness_15;
    let _e104 = mx_oren_nayar_diffuse_dir_albedo_analytic(_e102, _e103);
    dirAlbedo_3 = _e104;
    let _e106 = dirAlbedo_3;
    return clamp(_e106, 0f, 1f);
}

fn mx_oren_nayar_fujii_diffuse_dir_albedo(cosTheta_32: f32, roughness_16: f32) -> f32 {
    var cosTheta_33: f32;
    var roughness_17: f32;
    var A_2: f32;
    var B_2: f32;
    var Si: f32;
    var G_3: f32;

    cosTheta_33 = cosTheta_32;
    roughness_17 = roughness_16;
    let _e104 = roughness_17;
    A_2 = (1f / (1f + (FUJII_CONSTANT_1_ * _e104)));
    let _e109 = roughness_17;
    let _e110 = A_2;
    B_2 = (_e109 * _e110);
    let _e115 = cosTheta_33;
    let _e116 = mx_square(_e115);
    Si = sqrt(max(0f, (1f - _e116)));
    let _e121 = Si;
    let _e122 = cosTheta_33;
    let _e128 = Si;
    let _e129 = cosTheta_33;
    let _e134 = Si;
    let _e135 = cosTheta_33;
    let _e138 = Si;
    let _e139 = Si;
    let _e141 = Si;
    let _e145 = Si;
    G_3 = ((_e121 * (acos(clamp(_e122, -1f, 1f)) - (_e128 * _e129))) + ((2f * (((_e134 / _e135) * (1f - ((_e138 * _e139) * _e141))) - _e145)) / 3f));
    let _e152 = A_2;
    let _e153 = B_2;
    let _e154 = G_3;
    return (_e152 + ((_e153 * _e154) * 0.31830987f));
}

fn mx_oren_nayar_fujii_diffuse_avg_albedo(roughness_18: f32) -> f32 {
    var roughness_19: f32;
    var A_3: f32;

    roughness_19 = roughness_18;
    let _e102 = roughness_19;
    A_3 = (1f / (1f + (FUJII_CONSTANT_1_ * _e102)));
    let _e107 = A_3;
    let _e109 = roughness_19;
    return (_e107 * (1f + (FUJII_CONSTANT_2_ * _e109)));
}

fn mx_oren_nayar_compensated_diffuse(NdotV_29: f32, NdotL_9: f32, LdotV_3: f32, roughness_20: f32, color_4: vec3<f32>) -> vec3<f32> {
    var NdotV_30: f32;
    var NdotL_10: f32;
    var LdotV_4: f32;
    var roughness_21: f32;
    var color_5: vec3<f32>;
    var s_2: f32;
    var local_42: f32;
    var stinv_1: f32;
    var A_4: f32;
    var lobeSingleScatter: vec3<f32>;
    var dirAlbedoV: f32;
    var dirAlbedoL: f32;
    var avgAlbedo: f32;
    var colorMultiScatter: vec3<f32>;
    var lobeMultiScatter: vec3<f32>;

    NdotV_30 = NdotV_29;
    NdotL_10 = NdotL_9;
    LdotV_4 = LdotV_3;
    roughness_21 = roughness_20;
    color_5 = color_4;
    let _e108 = LdotV_4;
    let _e109 = NdotL_10;
    let _e110 = NdotV_30;
    s_2 = (_e108 - (_e109 * _e110));
    let _e114 = s_2;
    if (_e114 > 0f) {
        let _e117 = s_2;
        let _e118 = NdotL_10;
        let _e119 = NdotV_30;
        local_42 = (_e117 / max(_e118, _e119));
    } else {
        let _e122 = s_2;
        local_42 = _e122;
    }
    let _e124 = local_42;
    stinv_1 = _e124;
    let _e128 = roughness_21;
    A_4 = (1f / (1f + (FUJII_CONSTANT_1_ * _e128)));
    let _e133 = color_5;
    let _e134 = A_4;
    let _e137 = roughness_21;
    let _e138 = stinv_1;
    lobeSingleScatter = ((_e133 * _e134) * (1f + (_e137 * _e138)));
    let _e143 = NdotV_30;
    let _e144 = roughness_21;
    let _e145 = mx_oren_nayar_fujii_diffuse_dir_albedo(_e143, _e144);
    dirAlbedoV = _e145;
    let _e147 = NdotL_10;
    let _e148 = roughness_21;
    let _e149 = mx_oren_nayar_fujii_diffuse_dir_albedo(_e147, _e148);
    dirAlbedoL = _e149;
    let _e151 = roughness_21;
    let _e152 = mx_oren_nayar_fujii_diffuse_avg_albedo(_e151);
    avgAlbedo = _e152;
    let _e154 = color_5;
    let _e155 = mx_square_2(_e154);
    let _e156 = avgAlbedo;
    let _e160 = color_5;
    let _e163 = avgAlbedo;
    colorMultiScatter = ((_e155 * _e156) / (vec3(1f) - (_e160 * max(0f, (1f - _e163)))));
    let _e170 = colorMultiScatter;
    let _e173 = dirAlbedoV;
    let _e179 = dirAlbedoL;
    let _e185 = avgAlbedo;
    lobeMultiScatter = (((_e170 * max(0.00000001f, (1f - _e173))) * max(0.00000001f, (1f - _e179))) / vec3(max(0.00000001f, (1f - _e185))));
    let _e191 = lobeSingleScatter;
    let _e192 = lobeMultiScatter;
    return (_e191 + _e192);
}

fn mx_oren_nayar_compensated_diffuse_dir_albedo(cosTheta_34: f32, roughness_22: f32, color_6: vec3<f32>) -> vec3<f32> {
    var cosTheta_35: f32;
    var roughness_23: f32;
    var color_7: vec3<f32>;
    var dirAlbedo_4: f32;
    var avgAlbedo_1: f32;
    var colorMultiScatter_1: vec3<f32>;

    cosTheta_35 = cosTheta_34;
    roughness_23 = roughness_22;
    color_7 = color_6;
    let _e104 = cosTheta_35;
    let _e105 = roughness_23;
    let _e106 = mx_oren_nayar_fujii_diffuse_dir_albedo(_e104, _e105);
    dirAlbedo_4 = _e106;
    let _e108 = roughness_23;
    let _e109 = mx_oren_nayar_fujii_diffuse_avg_albedo(_e108);
    avgAlbedo_1 = _e109;
    let _e111 = color_7;
    let _e112 = mx_square_2(_e111);
    let _e113 = avgAlbedo_1;
    let _e117 = color_7;
    let _e120 = avgAlbedo_1;
    colorMultiScatter_1 = ((_e112 * _e113) / (vec3(1f) - (_e117 * max(0f, (1f - _e120)))));
    let _e127 = colorMultiScatter_1;
    let _e128 = color_7;
    let _e129 = dirAlbedo_4;
    return mix(_e127, _e128, vec3(_e129));
}

fn mx_burley_diffuse(NdotV_31: f32, NdotL_11: f32, LdotH: f32, roughness_24: f32) -> f32 {
    var NdotV_32: f32;
    var NdotL_12: f32;
    var LdotH_1: f32;
    var roughness_25: f32;
    var F90_20: f32;
    var refL: f32;
    var refV: f32;

    NdotV_32 = NdotV_31;
    NdotL_12 = NdotL_11;
    LdotH_1 = LdotH;
    roughness_25 = roughness_24;
    let _e108 = roughness_25;
    let _e110 = LdotH_1;
    let _e111 = mx_square(_e110);
    F90_20 = (0.5f + ((2f * _e108) * _e111));
    let _e115 = NdotL_12;
    let _e117 = F90_20;
    let _e118 = mx_fresnel_schlick_2(_e115, 1f, _e117);
    refL = _e118;
    let _e120 = NdotV_32;
    let _e122 = F90_20;
    let _e123 = mx_fresnel_schlick_2(_e120, 1f, _e122);
    refV = _e123;
    let _e125 = refL;
    let _e126 = refV;
    return (_e125 * _e126);
}

fn mx_burley_diffuse_dir_albedo(NdotV_33: f32, roughness_26: f32) -> f32 {
    var NdotV_34: f32;
    var roughness_27: f32;
    var x_19: f32;
    var fit0_: f32;
    var fit1_: f32;

    NdotV_34 = NdotV_33;
    roughness_27 = roughness_26;
    let _e102 = NdotV_34;
    x_19 = _e102;
    let _e107 = x_19;
    let _e109 = mx_pow5_((1f - _e107));
    fit0_ = (0.97619f - (0.488095f * _e109));
    let _e118 = x_19;
    let _e121 = x_19;
    let _e124 = x_19;
    fit1_ = (1.55754f + ((-2.02221f + ((2.56283f - (1.06244f * _e118)) * _e121)) * _e124));
    let _e128 = fit0_;
    let _e129 = fit1_;
    let _e130 = roughness_27;
    return mix(_e128, _e129, _e130);
}

fn mx_burley_diffusion_profile(dist: f32, shape: vec3<f32>) -> vec3<f32> {
    var dist_1: f32;
    var shape_1: vec3<f32>;
    var num1_: vec3<f32>;
    var num2_: vec3<f32>;
    var denom_1: f32;

    dist_1 = dist;
    shape_1 = shape;
    let _e102 = shape_1;
    let _e104 = dist_1;
    num1_ = exp((-(_e102) * _e104));
    let _e108 = shape_1;
    let _e110 = dist_1;
    num2_ = exp(((-(_e108) * _e110) / vec3(3f)));
    let _e117 = dist_1;
    denom_1 = max(_e117, 0.00000001f);
    let _e121 = num1_;
    let _e122 = num2_;
    let _e124 = denom_1;
    return ((_e121 + _e122) / vec3(_e124));
}

fn mx_integrate_burley_diffusion(N_18: vec3<f32>, L_7: vec3<f32>, radius: f32, mfp: vec3<f32>) -> vec3<f32> {
    var N_19: vec3<f32>;
    var L_8: vec3<f32>;
    var radius_1: f32;
    var mfp_1: vec3<f32>;
    var theta: f32;
    var shape_2: vec3<f32>;
    var sumD: vec3<f32> = vec3(0f);
    var sumR: vec3<f32> = vec3(0f);
    var SAMPLE_COUNT_2: i32 = 32i;
    var SAMPLE_WIDTH: f32;
    var i_7: i32 = 0i;
    var x_20: f32;
    var dist_2: f32;
    var R_2: vec3<f32>;

    N_19 = N_18;
    L_8 = L_7;
    radius_1 = radius;
    mfp_1 = mfp;
    let _e106 = N_19;
    let _e107 = L_8;
    theta = acos(dot(_e106, _e107));
    let _e113 = mfp_1;
    shape_2 = (vec3(1f) / max(_e113, vec3(0.1f)));
    let _e130 = SAMPLE_COUNT_2;
    SAMPLE_WIDTH = (6.2831855f / f32(_e130));
    loop {
        let _e136 = i_7;
        let _e137 = SAMPLE_COUNT_2;
        if !((_e136 < _e137)) {
            break;
        }
        {
            let _e145 = i_7;
            let _e149 = SAMPLE_WIDTH;
            x_20 = (-3.1415927f + ((f32(_e145) + 0.5f) * _e149));
            let _e153 = radius_1;
            let _e155 = x_20;
            dist_2 = (_e153 * abs((2f * sin((_e155 * 0.5f)))));
            let _e163 = dist_2;
            let _e164 = shape_2;
            let _e165 = mx_burley_diffusion_profile(_e163, _e164);
            R_2 = _e165;
            let _e167 = sumD;
            let _e168 = R_2;
            let _e169 = theta;
            let _e170 = x_20;
            sumD = (_e167 + (_e168 * max(cos((_e169 + _e170)), 0f)));
            let _e177 = sumR;
            let _e178 = R_2;
            sumR = (_e177 + _e178);
        }
        continuing {
            let _e140 = i_7;
            i_7 = (_e140 + 1i);
        }
    }
    let _e180 = sumD;
    let _e181 = sumR;
    return (_e180 / _e181);
}

fn mx_subsurface_scattering_approx(N_20: vec3<f32>, L_9: vec3<f32>, P_2: vec3<f32>, albedo: vec3<f32>, mfp_2: vec3<f32>, curvature: f32) -> vec3<f32> {
    var N_21: vec3<f32>;
    var L_10: vec3<f32>;
    var P_3: vec3<f32>;
    var albedo_1: vec3<f32>;
    var mfp_3: vec3<f32>;
    var curvature_1: f32;
    var radius_2: f32;

    N_21 = N_20;
    L_10 = L_9;
    P_3 = P_2;
    albedo_1 = albedo;
    mfp_3 = mfp_2;
    curvature_1 = curvature;
    let _e111 = curvature_1;
    radius_2 = (1f / max(_e111, 0.01f));
    let _e116 = albedo_1;
    let _e117 = N_21;
    let _e118 = L_10;
    let _e119 = radius_2;
    let _e120 = mfp_3;
    let _e121 = mx_integrate_burley_diffusion(_e117, _e118, _e119, _e120);
    return ((_e116 * _e121) / vec3(3.1415927f));
}

fn mx_oren_nayar_diffuse_bsdf(closureData_10: ClosureData, weight_4: f32, color_8: vec3<f32>, roughness_28: f32, N_22: vec3<f32>, energy_compensation: bool, bsdf_2: ptr<function, BSDF>) {
    var closureData_11: ClosureData;
    var weight_5: f32;
    var color_9: vec3<f32>;
    var roughness_29: f32;
    var N_23: vec3<f32>;
    var energy_compensation_1: bool;
    var V_15: vec3<f32>;
    var L_11: vec3<f32>;
    var NdotV_35: f32;
    var NdotL_13: f32;
    var LdotV_5: f32;
    var local_43: vec3<f32>;
    var diffuse: vec3<f32>;
    var local_44: vec3<f32>;
    var diffuse_1: vec3<f32>;
    var Li_3: vec3<f32>;

    closureData_11 = closureData_10;
    weight_5 = weight_4;
    color_9 = color_8;
    roughness_29 = roughness_28;
    N_23 = N_22;
    energy_compensation_1 = energy_compensation;
    (*bsdf_2).throughput = vec3(0f);
    let _e114 = weight_5;
    if (_e114 < 0.00000001f) {
        {
            return;
        }
    }
    let _e117 = closureData_11;
    V_15 = _e117.V;
    let _e120 = closureData_11;
    L_11 = _e120.L;
    let _e123 = N_23;
    let _e124 = V_15;
    let _e125 = mx_forward_facing_normal(_e123, _e124);
    N_23 = _e125;
    let _e126 = N_23;
    let _e127 = V_15;
    NdotV_35 = clamp(dot(_e126, _e127), 0.00000001f, 1f);
    let _e133 = closureData_11;
    if (_e133.closureType == 1i) {
        {
            let _e137 = N_23;
            let _e138 = L_11;
            NdotL_13 = clamp(dot(_e137, _e138), 0.00000001f, 1f);
            let _e144 = L_11;
            let _e145 = V_15;
            LdotV_5 = clamp(dot(_e144, _e145), 0.00000001f, 1f);
            let _e151 = energy_compensation_1;
            if _e151 {
                let _e152 = NdotV_35;
                let _e153 = NdotL_13;
                let _e154 = LdotV_5;
                let _e155 = roughness_29;
                let _e156 = color_9;
                let _e157 = mx_oren_nayar_compensated_diffuse(_e152, _e153, _e154, _e155, _e156);
                local_43 = _e157;
            } else {
                let _e158 = NdotV_35;
                let _e159 = NdotL_13;
                let _e160 = LdotV_5;
                let _e161 = roughness_29;
                let _e162 = mx_oren_nayar_diffuse(_e158, _e159, _e160, _e161);
                let _e163 = color_9;
                local_43 = (_e162 * _e163);
            }
            let _e166 = local_43;
            diffuse = _e166;
            let _e169 = diffuse;
            let _e170 = closureData_11;
            let _e173 = weight_5;
            let _e175 = NdotL_13;
            (*bsdf_2).response = ((((_e169 * _e170.occlusion) * _e173) * _e175) * 0.31830987f);
            return;
        }
    } else {
        let _e181 = closureData_11;
        if (_e181.closureType == 3i) {
            {
                let _e185 = energy_compensation_1;
                if _e185 {
                    let _e186 = NdotV_35;
                    let _e187 = roughness_29;
                    let _e188 = color_9;
                    let _e189 = mx_oren_nayar_compensated_diffuse_dir_albedo(_e186, _e187, _e188);
                    local_44 = _e189;
                } else {
                    let _e190 = NdotV_35;
                    let _e191 = roughness_29;
                    let _e192 = mx_oren_nayar_diffuse_dir_albedo(_e190, _e191);
                    let _e193 = color_9;
                    local_44 = (_e192 * _e193);
                }
                let _e196 = local_44;
                diffuse_1 = _e196;
                let _e198 = N_23;
                let _e199 = mx_environment_irradiance(_e198);
                Li_3 = _e199;
                let _e202 = Li_3;
                let _e203 = diffuse_1;
                let _e205 = weight_5;
                (*bsdf_2).response = ((_e202 * _e203) * _e205);
                return;
            }
        } else {
            return;
        }
    }
}

fn NG_convert_float_color3_(in1_12: f32, out1_7: ptr<function, vec3<f32>>) {
    var in1_13: f32;
    var combine_out: vec3<f32>;

    in1_13 = in1_12;
    let _e101 = in1_13;
    let _e102 = in1_13;
    let _e103 = in1_13;
    combine_out = vec3<f32>(_e101, _e102, _e103);
    let _e106 = combine_out;
    (*out1_7) = _e106;
    return;
}

fn mx_multiply_bsdf_float(closureData_12: ClosureData, in1_14: BSDF, in2_6: f32, result_9: ptr<function, BSDF>) {
    var closureData_13: ClosureData;
    var in1_15: BSDF;
    var in2_7: f32;
    var weight_6: f32;

    closureData_13 = closureData_12;
    in1_15 = in1_14;
    in2_7 = in2_6;
    let _e105 = in2_7;
    weight_6 = clamp(_e105, 0f, 1f);
    let _e111 = in1_15;
    let _e113 = weight_6;
    (*result_9).response = (_e111.response * _e113);
    let _e116 = in1_15;
    (*result_9).throughput = _e116.throughput;
    return;
}

fn mx_generalized_schlick_edf(closureData_14: ClosureData, color0_: vec3<f32>, color90_: vec3<f32>, exponent_6: f32, base: vec3<f32>, result_10: ptr<function, vec3<f32>>) {
    var closureData_15: ClosureData;
    var color0_1: vec3<f32>;
    var color90_1: vec3<f32>;
    var exponent_7: f32;
    var base_1: vec3<f32>;
    var N_24: vec3<f32>;
    var NdotV_36: f32;
    var f_1: vec3<f32>;

    closureData_15 = closureData_14;
    color0_1 = color0_;
    color90_1 = color90_;
    exponent_7 = exponent_6;
    base_1 = base;
    let _e109 = closureData_15;
    if (_e109.closureType == 4i) {
        {
            let _e113 = closureData_15;
            let _e115 = closureData_15;
            let _e117 = mx_forward_facing_normal(_e113.N, _e115.V);
            N_24 = _e117;
            let _e119 = N_24;
            let _e120 = closureData_15;
            NdotV_36 = clamp(dot(_e119, _e120.V), 0.00000001f, 1f);
            let _e127 = NdotV_36;
            let _e128 = color0_1;
            let _e129 = color90_1;
            let _e130 = exponent_7;
            let _e131 = mx_fresnel_schlick_5(_e127, _e128, _e129, _e130);
            f_1 = _e131;
            let _e133 = base_1;
            let _e134 = f_1;
            (*result_10) = (_e133 * _e134);
            return;
        }
    } else {
        return;
    }
}

fn mx_add_bsdf(closureData_16: ClosureData, in1_16: BSDF, in2_8: BSDF, result_11: ptr<function, BSDF>) {
    var closureData_17: ClosureData;
    var in1_17: BSDF;
    var in2_9: BSDF;

    closureData_17 = closureData_16;
    in1_17 = in1_16;
    in2_9 = in2_8;
    let _e106 = in1_17;
    let _e108 = in2_9;
    (*result_11).response = (_e106.response + _e108.response);
    let _e112 = in1_17;
    let _e114 = in2_9;
    (*result_11).throughput = max(((_e112.throughput + _e114.throughput) - vec3(1f)), vec3(0f));
    return;
}

fn mx_mix_edf(closureData_18: ClosureData, fg_2: vec3<f32>, bg_2: vec3<f32>, mixValue_2: f32, result_12: ptr<function, vec3<f32>>) {
    var closureData_19: ClosureData;
    var fg_3: vec3<f32>;
    var bg_3: vec3<f32>;
    var mixValue_3: f32;

    closureData_19 = closureData_18;
    fg_3 = fg_2;
    bg_3 = bg_2;
    mixValue_3 = mixValue_2;
    let _e107 = bg_3;
    let _e108 = fg_3;
    let _e109 = mixValue_3;
    (*result_12) = mix(_e107, _e108, vec3(_e109));
    return;
}

fn mx_layer_bsdf(closureData_20: ClosureData, top: BSDF, base_2: BSDF, result_13: ptr<function, BSDF>) {
    var closureData_21: ClosureData;
    var top_1: BSDF;
    var base_3: BSDF;

    closureData_21 = closureData_20;
    top_1 = top;
    base_3 = base_2;
    let _e105 = top_1;
    let _e110 = top_1;
    if ((length(_e105.response) < 0.0001f) && (length((_e110.throughput - vec3(1f))) < 0.0001f)) {
        {
            let _e119 = base_3;
            (*result_13) = _e119;
            return;
        }
    } else {
        {
            let _e121 = top_1;
            let _e123 = base_3;
            let _e125 = top_1;
            (*result_13).response = (_e121.response + (_e123.response * _e125.throughput));
            let _e130 = top_1;
            let _e132 = base_3;
            (*result_13).throughput = (_e130.throughput * _e132.throughput);
            return;
        }
    }
}

fn mx_multiply_bsdf_color3_(closureData_22: ClosureData, in1_18: BSDF, in2_10: vec3<f32>, result_14: ptr<function, BSDF>) {
    var closureData_23: ClosureData;
    var in1_19: BSDF;
    var in2_11: vec3<f32>;
    var tint_4: vec3<f32>;

    closureData_23 = closureData_22;
    in1_19 = in1_18;
    in2_11 = in2_10;
    let _e105 = in2_11;
    tint_4 = clamp(_e105, vec3(0f), vec3(1f));
    let _e113 = in1_19;
    let _e115 = tint_4;
    (*result_14).response = (_e113.response * _e115);
    let _e118 = in1_19;
    (*result_14).throughput = _e118.throughput;
    return;
}

fn NG_standard_surface_surfaceshader_optim(in2_12: f32, in1_20: vec3<f32>, roughness_30: f32, weight_7: f32, in21_2: f32, in11_6: vec3<f32>, bg_4: f32, ior_15: f32, anisotropy_2: f32, in12_6: f32, in13_6: f32, transmission_color: vec3<f32>, transmission_depth: f32, transmission_scatter: vec3<f32>, transmission_scatter_anisotropy: f32, transmission_dispersion: f32, in22_: f32, in14_6: f32, in15_6: vec3<f32>, in16_6: vec3<f32>, in23_: f32, subsurface_anisotropy: f32, sheen: f32, sheen_color: vec3<f32>, sheen_roughness: f32, weight1_: f32, fg_4: vec3<f32>, roughness1_: f32, anisotropy1_: f32, in17_6: f32, ior1_: f32, normal: vec3<f32>, in24_: f32, in18_6: f32, thinfilm_thickness_4: f32, thinfilm_ior_4: f32, in25_: f32, in19_6: vec3<f32>, in110_: vec3<f32>, in111_: bool, normal1_: vec3<f32>, in26_: vec3<f32>, out1_8: ptr<function, surfaceshader>) {
    var in2_13: f32;
    var in1_21: vec3<f32>;
    var roughness_31: f32;
    var weight_8: f32;
    var in21_3: f32;
    var in11_7: vec3<f32>;
    var bg_5: f32;
    var ior_16: f32;
    var anisotropy_3: f32;
    var in12_7: f32;
    var in13_7: f32;
    var transmission_color_1: vec3<f32>;
    var transmission_depth_1: f32;
    var transmission_scatter_1: vec3<f32>;
    var transmission_scatter_anisotropy_1: f32;
    var transmission_dispersion_1: f32;
    var in22_1: f32;
    var in14_7: f32;
    var in15_7: vec3<f32>;
    var in16_7: vec3<f32>;
    var in23_1: f32;
    var subsurface_anisotropy_1: f32;
    var sheen_1: f32;
    var sheen_color_1: vec3<f32>;
    var sheen_roughness_1: f32;
    var weight1_1: f32;
    var fg_5: vec3<f32>;
    var roughness1_1: f32;
    var anisotropy1_1: f32;
    var in17_7: f32;
    var ior1_1: f32;
    var normal_1: vec3<f32>;
    var in24_1: f32;
    var in18_7: f32;
    var thinfilm_thickness_5: f32;
    var thinfilm_ior_5: f32;
    var in25_1: f32;
    var in19_7: vec3<f32>;
    var in110_1: vec3<f32>;
    var in111_1: bool;
    var normal1_1: vec3<f32>;
    var in26_1: vec3<f32>;
    var coat_roughness_vector_out: vec2<f32> = vec2(0f);
    var coat_tangent_rotate_degree_in2_tmp: f32 = 360f;
    var coat_tangent_rotate_degree_out: f32;
    var metal_reflectivity_out: vec3<f32>;
    var metal_edgecolor_out: vec3<f32>;
    var coat_affect_roughness_multiply1_out: f32;
    var tangent_rotate_degree_in2_tmp: f32 = 360f;
    var tangent_rotate_degree_out: f32;
    var transmission_roughness_add_out: f32;
    var subsurface_color_nonnegative_in2_tmp: f32 = 0f;
    var subsurface_color_nonnegative_out: vec3<f32>;
    var coat_clamped_low_tmp: f32 = 0f;
    var coat_clamped_high_tmp: f32 = 1f;
    var coat_clamped_out: f32;
    var subsurface_radius_scaled_out: vec3<f32>;
    var subsurface_selector_out: f32;
    var base_color_nonnegative_in2_tmp: f32 = 0f;
    var base_color_nonnegative_out: vec3<f32>;
    var subsurface_inv_amount_tmp: f32 = 1f;
    var subsurface_inv_out: f32;
    var transmission_inv_amount_tmp: f32 = 1f;
    var transmission_inv_out: f32;
    var metalness_inv_amount_tmp: f32 = 1f;
    var metalness_inv_out: f32;
    var coat_attenuation_bg_tmp: vec3<f32> = vec3<f32>(1f, 1f, 1f);
    var coat_attenuation_out: vec3<f32>;
    var one_minus_coat_ior_in1_tmp: f32 = 1f;
    var one_minus_coat_ior_out: f32;
    var one_plus_coat_ior_in1_tmp: f32 = 1f;
    var one_plus_coat_ior_out: f32;
    var emission_weight_out: vec3<f32>;
    var opacity_luminance_out: vec3<f32> = vec3(0f);
    var coat_tangent_rotate_out: vec3<f32> = vec3(0f);
    var artistic_ior_ior: vec3<f32> = vec3(0f);
    var artistic_ior_extinction: vec3<f32> = vec3(0f);
    var coat_affect_roughness_multiply2_out: f32;
    var tangent_rotate_out: vec3<f32> = vec3(0f);
    var transmission_roughness_clamped_low_tmp: f32 = 0f;
    var transmission_roughness_clamped_high_tmp: f32 = 1f;
    var transmission_roughness_clamped_out: f32;
    var coat_gamma_multiply_out: f32;
    var coat_ior_to_F0_sqrt_out: f32;
    var opacity_luminance_float_index_tmp: i32 = 0i;
    var opacity_luminance_float_out: f32;
    var coat_tangent_rotate_normalize_out: vec3<f32>;
    var coat_affected_roughness_fg_tmp: f32 = 1f;
    var coat_affected_roughness_out: f32;
    var tangent_rotate_normalize_out: vec3<f32>;
    var coat_affected_transmission_roughness_fg_tmp: f32 = 1f;
    var coat_affected_transmission_roughness_out: f32;
    var coat_gamma_in2_tmp: f32 = 1f;
    var coat_gamma_out: f32;
    var coat_ior_to_F0_out: f32;
    var coat_tangent_value2_tmp: f32 = 0f;
    var local_45: vec3<f32>;
    var coat_tangent_out: vec3<f32>;
    var main_roughness_out: vec2<f32> = vec2(0f);
    var main_tangent_value2_tmp: f32 = 0f;
    var local_46: vec3<f32>;
    var main_tangent_out: vec3<f32>;
    var transmission_roughness_out: vec2<f32> = vec2(0f);
    var coat_affected_subsurface_color_out: vec3<f32>;
    var coat_affected_diffuse_color_out: vec3<f32>;
    var one_minus_coat_ior_to_F0_in1_tmp: f32 = 1f;
    var one_minus_coat_ior_to_F0_out: f32;
    var emission_color0_out: vec3<f32> = vec3(0f);
    var surfaceshader_out: surfaceshader = surfaceshader(vec3(0f), vec3(0f));
    var N_25: vec3<f32>;
    var V_16: vec3<f32>;
    var P_4: vec3<f32>;
    var L_12: vec3<f32> = vec3<f32>(0f, 0f, 0f);
    var occlusion_2: f32 = 1f;
    var surfaceOpacity: f32;
    var numLights: i32;
    var lightShader: lightshader;
    var activeLightIndex: i32 = 0i;
    var closureData_24: ClosureData;
    var coat_bsdf_out: BSDF;
    var metal_bsdf_out: BSDF;
    var specular_bsdf_out: BSDF;
    var selected_subsurface_bsdf_out: BSDF;
    var diffuse_bsdf_out: BSDF;
    var diffuse_bsdf_non_subsurface_out: BSDF;
    var subsurface_blend_out: BSDF;
    var sheen_layer_out: BSDF;
    var sheen_layer_non_transmission_out: BSDF;
    var transmission_blend_out: BSDF;
    var specular_layer_out: BSDF;
    var specular_layer_non_metal_out: BSDF;
    var metalness_blend_out: BSDF;
    var thin_film_layer_attenuated_out: BSDF;
    var coat_layer_out: BSDF;
    var closureData_25: ClosureData;
    var coat_bsdf_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var metal_bsdf_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var specular_bsdf_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var selected_subsurface_bsdf_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var diffuse_bsdf_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var diffuse_bsdf_non_subsurface_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var subsurface_blend_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var sheen_layer_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var sheen_layer_non_transmission_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var transmission_blend_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var specular_layer_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var specular_layer_non_metal_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var metalness_blend_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var thin_film_layer_attenuated_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var coat_layer_out_1: BSDF = BSDF(vec3(0f), vec3(1f));
    var closureData_26: ClosureData;
    var emission_edf_out: vec3<f32> = vec3(0f);
    var coat_tinted_emission_edf_out: vec3<f32> = vec3(0f);
    var coat_emission_edf_out: vec3<f32> = vec3(0f);
    var blended_coat_emission_edf_out: vec3<f32> = vec3(0f);
    var closureData_27: ClosureData;
    var coat_bsdf_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var metal_bsdf_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var specular_bsdf_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var selected_subsurface_bsdf_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var diffuse_bsdf_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var diffuse_bsdf_non_subsurface_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var subsurface_blend_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var sheen_layer_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var sheen_layer_non_transmission_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var transmission_blend_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var specular_layer_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var specular_layer_non_metal_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var metalness_blend_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var thin_film_layer_attenuated_out_2: BSDF = BSDF(vec3(0f), vec3(1f));
    var coat_layer_out_2: BSDF = BSDF(vec3(0f), vec3(1f));

    in2_13 = in2_12;
    in1_21 = in1_20;
    roughness_31 = roughness_30;
    weight_8 = weight_7;
    in21_3 = in21_2;
    in11_7 = in11_6;
    bg_5 = bg_4;
    ior_16 = ior_15;
    anisotropy_3 = anisotropy_2;
    in12_7 = in12_6;
    in13_7 = in13_6;
    transmission_color_1 = transmission_color;
    transmission_depth_1 = transmission_depth;
    transmission_scatter_1 = transmission_scatter;
    transmission_scatter_anisotropy_1 = transmission_scatter_anisotropy;
    transmission_dispersion_1 = transmission_dispersion;
    in22_1 = in22_;
    in14_7 = in14_6;
    in15_7 = in15_6;
    in16_7 = in16_6;
    in23_1 = in23_;
    subsurface_anisotropy_1 = subsurface_anisotropy;
    sheen_1 = sheen;
    sheen_color_1 = sheen_color;
    sheen_roughness_1 = sheen_roughness;
    weight1_1 = weight1_;
    fg_5 = fg_4;
    roughness1_1 = roughness1_;
    anisotropy1_1 = anisotropy1_;
    in17_7 = in17_6;
    ior1_1 = ior1_;
    normal_1 = normal;
    in24_1 = in24_;
    in18_7 = in18_6;
    thinfilm_thickness_5 = thinfilm_thickness_4;
    thinfilm_ior_5 = thinfilm_ior_4;
    in25_1 = in25_;
    in19_7 = in19_6;
    in110_1 = in110_;
    in111_1 = in111_;
    normal1_1 = normal1_;
    in26_1 = in26_;
    let _e186 = roughness1_1;
    let _e187 = anisotropy1_1;
    mx_roughness_anisotropy(_e186, _e187, (&coat_roughness_vector_out));
    let _e192 = in17_7;
    let _e193 = coat_tangent_rotate_degree_in2_tmp;
    coat_tangent_rotate_degree_out = (_e192 * _e193);
    let _e196 = in1_21;
    let _e197 = in2_13;
    metal_reflectivity_out = (_e196 * _e197);
    let _e200 = in11_7;
    let _e201 = in21_3;
    metal_edgecolor_out = (_e200 * _e201);
    let _e204 = in18_7;
    let _e205 = weight1_1;
    coat_affect_roughness_multiply1_out = (_e204 * _e205);
    let _e210 = in12_7;
    let _e211 = tangent_rotate_degree_in2_tmp;
    tangent_rotate_degree_out = (_e210 * _e211);
    let _e214 = bg_5;
    let _e215 = in22_1;
    transmission_roughness_add_out = (_e214 + _e215);
    let _e220 = in15_7;
    let _e221 = subsurface_color_nonnegative_in2_tmp;
    subsurface_color_nonnegative_out = max(_e220, vec3(_e221));
    let _e229 = weight1_1;
    let _e230 = coat_clamped_low_tmp;
    let _e231 = coat_clamped_high_tmp;
    coat_clamped_out = clamp(_e229, _e230, _e231);
    let _e234 = in16_7;
    let _e235 = in23_1;
    subsurface_radius_scaled_out = (_e234 * _e235);
    let _e238 = in111_1;
    subsurface_selector_out = select(0f, 1f, _e238);
    let _e245 = in1_21;
    let _e246 = base_color_nonnegative_in2_tmp;
    base_color_nonnegative_out = max(_e245, vec3(_e246));
    let _e252 = subsurface_inv_amount_tmp;
    let _e253 = in14_7;
    subsurface_inv_out = (_e252 - _e253);
    let _e258 = transmission_inv_amount_tmp;
    let _e259 = in13_7;
    transmission_inv_out = (_e258 - _e259);
    let _e264 = metalness_inv_amount_tmp;
    let _e265 = weight_8;
    metalness_inv_out = (_e264 - _e265);
    let _e273 = coat_attenuation_bg_tmp;
    let _e274 = fg_5;
    let _e275 = weight1_1;
    coat_attenuation_out = mix(_e273, _e274, vec3(_e275));
    let _e281 = one_minus_coat_ior_in1_tmp;
    let _e282 = ior1_1;
    one_minus_coat_ior_out = (_e281 - _e282);
    let _e287 = one_plus_coat_ior_in1_tmp;
    let _e288 = ior1_1;
    one_plus_coat_ior_out = (_e287 + _e288);
    let _e291 = in19_7;
    let _e292 = in25_1;
    emission_weight_out = (_e291 * _e292);
    let _e298 = in110_1;
    mx_luminance_color3_(_e298, vec3<f32>(0.272229f, 0.674082f, 0.053689f), (&opacity_luminance_out));
    let _e308 = in26_1;
    let _e309 = coat_tangent_rotate_degree_out;
    let _e310 = normal_1;
    mx_rotate_vector3_(_e308, _e309, _e310, (&coat_tangent_rotate_out));
    let _e319 = metal_reflectivity_out;
    let _e320 = metal_edgecolor_out;
    mx_artistic_ior(_e319, _e320, (&artistic_ior_ior), (&artistic_ior_extinction));
    let _e325 = coat_affect_roughness_multiply1_out;
    let _e326 = roughness1_1;
    coat_affect_roughness_multiply2_out = (_e325 * _e326);
    let _e332 = in26_1;
    let _e333 = tangent_rotate_degree_out;
    let _e334 = normal1_1;
    mx_rotate_vector3_(_e332, _e333, _e334, (&tangent_rotate_out));
    let _e341 = transmission_roughness_add_out;
    let _e342 = transmission_roughness_clamped_low_tmp;
    let _e343 = transmission_roughness_clamped_high_tmp;
    transmission_roughness_clamped_out = clamp(_e341, _e342, _e343);
    let _e346 = coat_clamped_out;
    let _e347 = in24_1;
    coat_gamma_multiply_out = (_e346 * _e347);
    let _e350 = one_minus_coat_ior_out;
    let _e351 = one_plus_coat_ior_out;
    coat_ior_to_F0_sqrt_out = (_e350 / _e351);
    let _e356 = opacity_luminance_float_index_tmp;
    let _e358 = opacity_luminance_out[_e356];
    opacity_luminance_float_out = _e358;
    let _e360 = coat_tangent_rotate_out;
    coat_tangent_rotate_normalize_out = normalize(_e360);
    let _e365 = bg_5;
    let _e366 = coat_affected_roughness_fg_tmp;
    let _e367 = coat_affect_roughness_multiply2_out;
    coat_affected_roughness_out = mix(_e365, _e366, _e367);
    let _e370 = tangent_rotate_out;
    tangent_rotate_normalize_out = normalize(_e370);
    let _e375 = transmission_roughness_clamped_out;
    let _e376 = coat_affected_transmission_roughness_fg_tmp;
    let _e377 = coat_affect_roughness_multiply2_out;
    coat_affected_transmission_roughness_out = mix(_e375, _e376, _e377);
    let _e382 = coat_gamma_multiply_out;
    let _e383 = coat_gamma_in2_tmp;
    coat_gamma_out = (_e382 + _e383);
    let _e386 = coat_ior_to_F0_sqrt_out;
    let _e387 = coat_ior_to_F0_sqrt_out;
    coat_ior_to_F0_out = (_e386 * _e387);
    let _e392 = anisotropy1_1;
    let _e393 = coat_tangent_value2_tmp;
    if (_e392 > _e393) {
        let _e395 = coat_tangent_rotate_normalize_out;
        local_45 = _e395;
    } else {
        let _e396 = in26_1;
        local_45 = _e396;
    }
    let _e398 = local_45;
    coat_tangent_out = _e398;
    let _e403 = coat_affected_roughness_out;
    let _e404 = anisotropy_3;
    mx_roughness_anisotropy(_e403, _e404, (&main_roughness_out));
    let _e409 = anisotropy_3;
    let _e410 = main_tangent_value2_tmp;
    if (_e409 > _e410) {
        let _e412 = tangent_rotate_normalize_out;
        local_46 = _e412;
    } else {
        let _e413 = in26_1;
        local_46 = _e413;
    }
    let _e415 = local_46;
    main_tangent_out = _e415;
    let _e420 = coat_affected_transmission_roughness_out;
    let _e421 = anisotropy_3;
    mx_roughness_anisotropy(_e420, _e421, (&transmission_roughness_out));
    let _e424 = subsurface_color_nonnegative_out;
    let _e425 = coat_gamma_out;
    coat_affected_subsurface_color_out = pow(_e424, vec3(_e425));
    let _e429 = base_color_nonnegative_out;
    let _e430 = coat_gamma_out;
    coat_affected_diffuse_color_out = pow(_e429, vec3(_e430));
    let _e436 = one_minus_coat_ior_to_F0_in1_tmp;
    let _e437 = coat_ior_to_F0_out;
    one_minus_coat_ior_to_F0_out = (_e436 - _e437);
    let _e443 = one_minus_coat_ior_to_F0_out;
    NG_convert_float_color3_(_e443, (&emission_color0_out));
    {
        let _e452 = vd;
        N_25 = normalize(_e452.normalWorld);
        let _e456 = global.u_viewPosition;
        let _e457 = vd;
        V_16 = normalize((_e456 - _e457.positionWorld));
        let _e462 = vd;
        P_4 = _e462.positionWorld;
        let _e472 = opacity_luminance_float_out;
        surfaceOpacity = _e472;
        let _e474 = numActiveLightSources();
        numLights = _e474;
        loop {
            let _e479 = activeLightIndex;
            let _e480 = numLights;
            if !((_e479 < _e480)) {
                break;
            }
            {
                let _e486 = activeLightIndex;
                let _e488 = global_2.u_lightData[_e486];
                let _e489 = vd;
                sampleLightSource(_e488, _e489.positionWorld, (&lightShader));
                let _e493 = lightShader;
                L_12 = _e493.direction;
                let _e496 = L_12;
                let _e497 = V_16;
                let _e498 = N_25;
                let _e499 = P_4;
                let _e500 = occlusion_2;
                let _e501 = makeClosureData(1i, _e496, _e497, _e498, _e499, _e500);
                closureData_24 = _e501;
                coat_bsdf_out = BSDF(vec3(0f), vec3(1f));
                let _e509 = closureData_24;
                let _e510 = weight1_1;
                let _e515 = ior1_1;
                let _e516 = coat_roughness_vector_out;
                let _e519 = normal_1;
                let _e520 = coat_tangent_out;
                mx_dielectric_bsdf(_e509, _e510, vec3<f32>(1f, 1f, 1f), _e515, _e516, 0f, 1.5f, _e519, _e520, 0i, 0i, (&coat_bsdf_out));
                metal_bsdf_out = BSDF(vec3(0f), vec3(1f));
                let _e531 = closureData_24;
                let _e532 = weight_8;
                let _e533 = artistic_ior_ior;
                let _e534 = artistic_ior_extinction;
                let _e535 = main_roughness_out;
                let _e536 = thinfilm_thickness_5;
                let _e537 = thinfilm_ior_5;
                let _e538 = normal1_1;
                let _e539 = main_tangent_out;
                mx_conductor_bsdf(_e531, _e532, _e533, _e534, _e535, _e536, _e537, _e538, _e539, 0i, (&metal_bsdf_out));
                specular_bsdf_out = BSDF(vec3(0f), vec3(1f));
                let _e549 = closureData_24;
                let _e550 = in21_3;
                let _e551 = in11_7;
                let _e552 = ior_16;
                let _e553 = main_roughness_out;
                let _e554 = thinfilm_thickness_5;
                let _e555 = thinfilm_ior_5;
                let _e556 = normal1_1;
                let _e557 = main_tangent_out;
                mx_dielectric_bsdf(_e549, _e550, _e551, _e552, _e553, _e554, _e555, _e556, _e557, 0i, 0i, (&specular_bsdf_out));
                selected_subsurface_bsdf_out = BSDF(vec3(0f), vec3(1f));
                let _e568 = closureData_24;
                let _e579 = subsurface_selector_out;
                mx_mix_bsdf(_e568, BSDF(vec3(0f), vec3(1f)), BSDF(vec3(0f), vec3(1f)), _e579, (&selected_subsurface_bsdf_out));
                diffuse_bsdf_out = BSDF(vec3(0f), vec3(1f));
                let _e588 = closureData_24;
                let _e589 = in2_13;
                let _e590 = coat_affected_diffuse_color_out;
                let _e591 = roughness_31;
                let _e592 = normal1_1;
                mx_oren_nayar_diffuse_bsdf(_e588, _e589, _e590, _e591, _e592, false, (&diffuse_bsdf_out));
                diffuse_bsdf_non_subsurface_out = BSDF(vec3(0f), vec3(1f));
                let _e602 = closureData_24;
                let _e603 = diffuse_bsdf_out;
                let _e604 = subsurface_inv_out;
                mx_multiply_bsdf_float(_e602, _e603, _e604, (&diffuse_bsdf_non_subsurface_out));
                subsurface_blend_out = BSDF(vec3(0f), vec3(1f));
                let _e613 = closureData_24;
                let _e614 = selected_subsurface_bsdf_out;
                let _e615 = diffuse_bsdf_non_subsurface_out;
                mx_add_bsdf(_e613, _e614, _e615, (&subsurface_blend_out));
                sheen_layer_out = BSDF(vec3(0f), vec3(1f));
                let _e624 = closureData_24;
                let _e630 = subsurface_blend_out;
                mx_layer_bsdf(_e624, BSDF(vec3(0f), vec3(1f)), _e630, (&sheen_layer_out));
                sheen_layer_non_transmission_out = BSDF(vec3(0f), vec3(1f));
                let _e639 = closureData_24;
                let _e640 = sheen_layer_out;
                let _e641 = transmission_inv_out;
                mx_multiply_bsdf_float(_e639, _e640, _e641, (&sheen_layer_non_transmission_out));
                transmission_blend_out = BSDF(vec3(0f), vec3(1f));
                let _e650 = closureData_24;
                let _e656 = sheen_layer_non_transmission_out;
                mx_add_bsdf(_e650, BSDF(vec3(0f), vec3(1f)), _e656, (&transmission_blend_out));
                specular_layer_out = BSDF(vec3(0f), vec3(1f));
                let _e665 = closureData_24;
                let _e666 = specular_bsdf_out;
                let _e667 = transmission_blend_out;
                mx_layer_bsdf(_e665, _e666, _e667, (&specular_layer_out));
                specular_layer_non_metal_out = BSDF(vec3(0f), vec3(1f));
                let _e676 = closureData_24;
                let _e677 = specular_layer_out;
                let _e678 = metalness_inv_out;
                mx_multiply_bsdf_float(_e676, _e677, _e678, (&specular_layer_non_metal_out));
                metalness_blend_out = BSDF(vec3(0f), vec3(1f));
                let _e687 = closureData_24;
                let _e688 = metal_bsdf_out;
                let _e689 = specular_layer_non_metal_out;
                mx_add_bsdf(_e687, _e688, _e689, (&metalness_blend_out));
                thin_film_layer_attenuated_out = BSDF(vec3(0f), vec3(1f));
                let _e698 = closureData_24;
                let _e699 = metalness_blend_out;
                let _e700 = coat_attenuation_out;
                mx_multiply_bsdf_color3_(_e698, _e699, _e700, (&thin_film_layer_attenuated_out));
                coat_layer_out = BSDF(vec3(0f), vec3(1f));
                let _e709 = closureData_24;
                let _e710 = coat_bsdf_out;
                let _e711 = thin_film_layer_attenuated_out;
                mx_layer_bsdf(_e709, _e710, _e711, (&coat_layer_out));
                let _e715 = surfaceshader_out;
                let _e717 = lightShader;
                let _e719 = coat_layer_out;
                surfaceshader_out.color = (_e715.color + (_e717.intensity * _e719.response));
                occlusion_2 = 1f;
            }
            continuing {
                let _e483 = activeLightIndex;
                activeLightIndex = (_e483 + 1i);
            }
        }
        occlusion_2 = 1f;
        {
            let _e726 = L_12;
            let _e727 = V_16;
            let _e728 = N_25;
            let _e729 = P_4;
            let _e730 = occlusion_2;
            let _e731 = makeClosureData(3i, _e726, _e727, _e728, _e729, _e730);
            closureData_25 = _e731;
            let _e739 = closureData_25;
            let _e740 = weight1_1;
            let _e745 = ior1_1;
            let _e746 = coat_roughness_vector_out;
            let _e749 = normal_1;
            let _e750 = coat_tangent_out;
            mx_dielectric_bsdf(_e739, _e740, vec3<f32>(1f, 1f, 1f), _e745, _e746, 0f, 1.5f, _e749, _e750, 0i, 0i, (&coat_bsdf_out_1));
            let _e761 = closureData_25;
            let _e762 = weight_8;
            let _e763 = artistic_ior_ior;
            let _e764 = artistic_ior_extinction;
            let _e765 = main_roughness_out;
            let _e766 = thinfilm_thickness_5;
            let _e767 = thinfilm_ior_5;
            let _e768 = normal1_1;
            let _e769 = main_tangent_out;
            mx_conductor_bsdf(_e761, _e762, _e763, _e764, _e765, _e766, _e767, _e768, _e769, 0i, (&metal_bsdf_out_1));
            let _e779 = closureData_25;
            let _e780 = in21_3;
            let _e781 = in11_7;
            let _e782 = ior_16;
            let _e783 = main_roughness_out;
            let _e784 = thinfilm_thickness_5;
            let _e785 = thinfilm_ior_5;
            let _e786 = normal1_1;
            let _e787 = main_tangent_out;
            mx_dielectric_bsdf(_e779, _e780, _e781, _e782, _e783, _e784, _e785, _e786, _e787, 0i, 0i, (&specular_bsdf_out_1));
            let _e798 = closureData_25;
            let _e809 = subsurface_selector_out;
            mx_mix_bsdf(_e798, BSDF(vec3(0f), vec3(1f)), BSDF(vec3(0f), vec3(1f)), _e809, (&selected_subsurface_bsdf_out_1));
            let _e818 = closureData_25;
            let _e819 = in2_13;
            let _e820 = coat_affected_diffuse_color_out;
            let _e821 = roughness_31;
            let _e822 = normal1_1;
            mx_oren_nayar_diffuse_bsdf(_e818, _e819, _e820, _e821, _e822, false, (&diffuse_bsdf_out_1));
            let _e832 = closureData_25;
            let _e833 = diffuse_bsdf_out_1;
            let _e834 = subsurface_inv_out;
            mx_multiply_bsdf_float(_e832, _e833, _e834, (&diffuse_bsdf_non_subsurface_out_1));
            let _e843 = closureData_25;
            let _e844 = selected_subsurface_bsdf_out_1;
            let _e845 = diffuse_bsdf_non_subsurface_out_1;
            mx_add_bsdf(_e843, _e844, _e845, (&subsurface_blend_out_1));
            let _e854 = closureData_25;
            let _e860 = subsurface_blend_out_1;
            mx_layer_bsdf(_e854, BSDF(vec3(0f), vec3(1f)), _e860, (&sheen_layer_out_1));
            let _e869 = closureData_25;
            let _e870 = sheen_layer_out_1;
            let _e871 = transmission_inv_out;
            mx_multiply_bsdf_float(_e869, _e870, _e871, (&sheen_layer_non_transmission_out_1));
            let _e880 = closureData_25;
            let _e886 = sheen_layer_non_transmission_out_1;
            mx_add_bsdf(_e880, BSDF(vec3(0f), vec3(1f)), _e886, (&transmission_blend_out_1));
            let _e895 = closureData_25;
            let _e896 = specular_bsdf_out_1;
            let _e897 = transmission_blend_out_1;
            mx_layer_bsdf(_e895, _e896, _e897, (&specular_layer_out_1));
            let _e906 = closureData_25;
            let _e907 = specular_layer_out_1;
            let _e908 = metalness_inv_out;
            mx_multiply_bsdf_float(_e906, _e907, _e908, (&specular_layer_non_metal_out_1));
            let _e917 = closureData_25;
            let _e918 = metal_bsdf_out_1;
            let _e919 = specular_layer_non_metal_out_1;
            mx_add_bsdf(_e917, _e918, _e919, (&metalness_blend_out_1));
            let _e928 = closureData_25;
            let _e929 = metalness_blend_out_1;
            let _e930 = coat_attenuation_out;
            mx_multiply_bsdf_color3_(_e928, _e929, _e930, (&thin_film_layer_attenuated_out_1));
            let _e939 = closureData_25;
            let _e940 = coat_bsdf_out_1;
            let _e941 = thin_film_layer_attenuated_out_1;
            mx_layer_bsdf(_e939, _e940, _e941, (&coat_layer_out_1));
            let _e945 = surfaceshader_out;
            let _e947 = occlusion_2;
            let _e948 = coat_layer_out_1;
            surfaceshader_out.color = (_e945.color + (_e947 * _e948.response));
        }
        {
            let _e953 = L_12;
            let _e954 = V_16;
            let _e955 = N_25;
            let _e956 = P_4;
            let _e957 = occlusion_2;
            let _e958 = makeClosureData(4i, _e953, _e954, _e955, _e956, _e957);
            closureData_26 = _e958;
            let _e963 = closureData_26;
            let _e964 = emission_weight_out;
            mx_uniform_edf(_e963, _e964, (&emission_edf_out));
            let _e970 = closureData_26;
            let _e971 = emission_edf_out;
            let _e972 = fg_5;
            mx_multiply_edf_color3_(_e970, _e971, _e972, (&coat_tinted_emission_edf_out));
            let _e978 = closureData_26;
            let _e979 = emission_color0_out;
            let _e985 = coat_tinted_emission_edf_out;
            mx_generalized_schlick_edf(_e978, _e979, vec3<f32>(0f, 0f, 0f), 5f, _e985, (&coat_emission_edf_out));
            let _e991 = closureData_26;
            let _e992 = coat_emission_edf_out;
            let _e993 = emission_edf_out;
            let _e994 = weight1_1;
            mx_mix_edf(_e991, _e992, _e993, _e994, (&blended_coat_emission_edf_out));
            let _e998 = surfaceshader_out;
            let _e1000 = blended_coat_emission_edf_out;
            surfaceshader_out.color = (_e998.color + _e1000);
        }
        let _e1003 = L_12;
        let _e1004 = V_16;
        let _e1005 = N_25;
        let _e1006 = P_4;
        let _e1007 = occlusion_2;
        let _e1008 = makeClosureData(2i, _e1003, _e1004, _e1005, _e1006, _e1007);
        closureData_27 = _e1008;
        let _e1016 = closureData_27;
        let _e1017 = weight1_1;
        let _e1022 = ior1_1;
        let _e1023 = coat_roughness_vector_out;
        let _e1026 = normal_1;
        let _e1027 = coat_tangent_out;
        mx_dielectric_bsdf(_e1016, _e1017, vec3<f32>(1f, 1f, 1f), _e1022, _e1023, 0f, 1.5f, _e1026, _e1027, 0i, 0i, (&coat_bsdf_out_2));
        let _e1038 = closureData_27;
        let _e1039 = weight_8;
        let _e1040 = artistic_ior_ior;
        let _e1041 = artistic_ior_extinction;
        let _e1042 = main_roughness_out;
        let _e1043 = thinfilm_thickness_5;
        let _e1044 = thinfilm_ior_5;
        let _e1045 = normal1_1;
        let _e1046 = main_tangent_out;
        mx_conductor_bsdf(_e1038, _e1039, _e1040, _e1041, _e1042, _e1043, _e1044, _e1045, _e1046, 0i, (&metal_bsdf_out_2));
        let _e1056 = closureData_27;
        let _e1057 = in21_3;
        let _e1058 = in11_7;
        let _e1059 = ior_16;
        let _e1060 = main_roughness_out;
        let _e1061 = thinfilm_thickness_5;
        let _e1062 = thinfilm_ior_5;
        let _e1063 = normal1_1;
        let _e1064 = main_tangent_out;
        mx_dielectric_bsdf(_e1056, _e1057, _e1058, _e1059, _e1060, _e1061, _e1062, _e1063, _e1064, 0i, 0i, (&specular_bsdf_out_2));
        let _e1075 = closureData_27;
        let _e1086 = subsurface_selector_out;
        mx_mix_bsdf(_e1075, BSDF(vec3(0f), vec3(1f)), BSDF(vec3(0f), vec3(1f)), _e1086, (&selected_subsurface_bsdf_out_2));
        let _e1095 = closureData_27;
        let _e1096 = in2_13;
        let _e1097 = coat_affected_diffuse_color_out;
        let _e1098 = roughness_31;
        let _e1099 = normal1_1;
        mx_oren_nayar_diffuse_bsdf(_e1095, _e1096, _e1097, _e1098, _e1099, false, (&diffuse_bsdf_out_2));
        let _e1109 = closureData_27;
        let _e1110 = diffuse_bsdf_out_2;
        let _e1111 = subsurface_inv_out;
        mx_multiply_bsdf_float(_e1109, _e1110, _e1111, (&diffuse_bsdf_non_subsurface_out_2));
        let _e1120 = closureData_27;
        let _e1121 = selected_subsurface_bsdf_out_2;
        let _e1122 = diffuse_bsdf_non_subsurface_out_2;
        mx_add_bsdf(_e1120, _e1121, _e1122, (&subsurface_blend_out_2));
        let _e1131 = closureData_27;
        let _e1137 = subsurface_blend_out_2;
        mx_layer_bsdf(_e1131, BSDF(vec3(0f), vec3(1f)), _e1137, (&sheen_layer_out_2));
        let _e1146 = closureData_27;
        let _e1147 = sheen_layer_out_2;
        let _e1148 = transmission_inv_out;
        mx_multiply_bsdf_float(_e1146, _e1147, _e1148, (&sheen_layer_non_transmission_out_2));
        let _e1157 = closureData_27;
        let _e1163 = sheen_layer_non_transmission_out_2;
        mx_add_bsdf(_e1157, BSDF(vec3(0f), vec3(1f)), _e1163, (&transmission_blend_out_2));
        let _e1172 = closureData_27;
        let _e1173 = specular_bsdf_out_2;
        let _e1174 = transmission_blend_out_2;
        mx_layer_bsdf(_e1172, _e1173, _e1174, (&specular_layer_out_2));
        let _e1183 = closureData_27;
        let _e1184 = specular_layer_out_2;
        let _e1185 = metalness_inv_out;
        mx_multiply_bsdf_float(_e1183, _e1184, _e1185, (&specular_layer_non_metal_out_2));
        let _e1194 = closureData_27;
        let _e1195 = metal_bsdf_out_2;
        let _e1196 = specular_layer_non_metal_out_2;
        mx_add_bsdf(_e1194, _e1195, _e1196, (&metalness_blend_out_2));
        let _e1205 = closureData_27;
        let _e1206 = metalness_blend_out_2;
        let _e1207 = coat_attenuation_out;
        mx_multiply_bsdf_color3_(_e1205, _e1206, _e1207, (&thin_film_layer_attenuated_out_2));
        let _e1216 = closureData_27;
        let _e1217 = coat_bsdf_out_2;
        let _e1218 = thin_film_layer_attenuated_out_2;
        mx_layer_bsdf(_e1216, _e1217, _e1218, (&coat_layer_out_2));
        let _e1222 = surfaceshader_out;
        let _e1224 = coat_layer_out_2;
        surfaceshader_out.color = (_e1222.color + _e1224.response);
        {
            let _e1228 = surfaceshader_out;
            let _e1230 = surfaceOpacity;
            surfaceshader_out.color = (_e1228.color * _e1230);
            let _e1237 = surfaceshader_out;
            let _e1239 = surfaceOpacity;
            surfaceshader_out.transparency = mix(vec3<f32>(1f, 1f, 1f), _e1237.transparency, vec3(_e1239));
        }
    }
    let _e1242 = surfaceshader_out;
    (*out1_8) = _e1242;
    return;
}

fn adsk_NG_adsk_unified_material(which: i32, in1_22: vec3<f32>, in11_8: vec3<f32>, in12_8: vec3<f32>, roughness_32: f32, anisotropy_4: f32, rotation: f32, in2_14: vec3<f32>, in21_4: vec3<f32>, f0_: f32, value1_: bool, in22_2: vec3<f32>, in23_2: f32, value11_: bool, in13_8: f32, in24_2: vec3<f32>, in3_: f32, in31_: vec3<f32>, in32_: f32, in4_: vec3<f32>, f01_: f32, roughness1_2: f32, anisotropy1_2: f32, rotation1_: f32, in25_2: f32, value12_: bool, in14_8: vec3<f32>, in15_8: vec3<f32>, in16_8: vec3<f32>, in17_8: f32, in18_8: vec3<f32>, backface_cullingbool_2: bool, out1_9: ptr<function, surfaceshader>) {
    var which_1: i32;
    var in1_23: vec3<f32>;
    var in11_9: vec3<f32>;
    var in12_9: vec3<f32>;
    var roughness_33: f32;
    var anisotropy_5: f32;
    var rotation_1: f32;
    var in2_15: vec3<f32>;
    var in21_5: vec3<f32>;
    var f0_1: f32;
    var value1_1: bool;
    var in22_3: vec3<f32>;
    var in23_3: f32;
    var value11_1: bool;
    var in13_9: f32;
    var in24_3: vec3<f32>;
    var in3_1: f32;
    var in31_1: vec3<f32>;
    var in32_1: f32;
    var in4_1: vec3<f32>;
    var f01_1: f32;
    var roughness1_3: f32;
    var anisotropy1_3: f32;
    var rotation1_1: f32;
    var in25_3: f32;
    var value12_1: bool;
    var in14_9: vec3<f32>;
    var in15_9: vec3<f32>;
    var in16_9: vec3<f32>;
    var in17_9: f32;
    var in18_9: vec3<f32>;
    var backface_cullingbool_3: bool;
    var metal_base_color_out: vec3<f32>;
    var layered_f0_weighted_out: vec3<f32>;
    var layered_oneminus_fraction_in1_tmp: f32 = 1f;
    var layered_oneminus_fraction_out: f32;
    var switch_specular_out: f32 = 0f;
    var surface_roughness_adjusted_out: f32 = 0f;
    var layered_roughness_adjusted_out: f32 = 0f;
    var opaque_ior_out: f32 = 0f;
    var glazing_f0_luminance_out: vec3<f32> = vec3(0f);
    var surface_anisotropy_adjusted_out: f32 = 0f;
    var layered_anisotropy_adjusted_out: f32 = 0f;
    var surface_rotation_normalized_out: f32 = 0f;
    var layered_rotation_normalized_out: f32 = 0f;
    var switch_transmission_out: f32 = 0f;
    var switch_transmission_depth_out: f32 = 0f;
    var glazing_transmission_extra_roughness_out: f32;
    var opaque_subsurface_value2_tmp: bool = true;
    var opaque_subsurface_in1_tmp: f32 = 1f;
    var opaque_subsurface_in2_tmp: f32 = 0f;
    var local_47: f32;
    var opaque_subsurface_out: f32;
    var switch_subsurface_color_out: vec3<f32> = vec3(0f);
    var switch_subsurface_radius_out: vec3<f32> = vec3(0f);
    var switch_subsurface_scale_out: f32 = 0f;
    var layered_coat_value_color3_out: vec3<f32> = vec3(0f);
    var layered_coat_ior_out: f32 = 0f;
    var switch_coat_normal_out: vec3<f32> = vec3(0f);
    var opaque_emission_value_value2_tmp: bool = true;
    var opaque_emission_value_in2_tmp: f32 = 0f;
    var local_48: f32;
    var opaque_emission_value_out: f32;
    var switch_emission_color_out: vec3<f32> = vec3(0f);
    var glazing_cutout_float_index_tmp: i32 = 0i;
    var glazing_cutout_float_out: f32;
    var switch_thin_walled_value2_tmp: i32 = 4i;
    var local_49: bool;
    var switch_thin_walled_out: bool;
    var switch_normal_out: vec3<f32> = vec3(0f);
    var geomprop_Tworld_out1_: vec3<f32>;
    var layered_metalness_value2_tmp: bool = true;
    var local_50: f32;
    var layered_metalness_out: f32;
    var layered_diffuse_weighted_out: vec3<f32>;
    var switch_coat_roughness_out: f32 = 0f;
    var switch_specular_roughness_out: f32 = 0f;
    var glazing_f0_float_index_tmp: i32 = 0i;
    var glazing_f0_float_out: f32;
    var glazing_oneminus_f0_in1_tmp: vec3<f32> = vec3<f32>(1f, 1f, 1f);
    var glazing_oneminus_f0_out: vec3<f32>;
    var switch_coat_anisotropy_out: f32 = 0f;
    var switch_specular_anisotropy_out: f32 = 0f;
    var switch_coat_rotation_out: f32 = 0f;
    var switch_specular_rotation_out: f32 = 0f;
    var switch_transmission_extra_roughness_out: f32 = 0f;
    var switch_subsurface_out: f32 = 0f;
    var layered_coat_value_index_tmp: i32 = 0i;
    var layered_coat_value_out: f32;
    var switch_coat_ior_out: f32 = 0f;
    var opaque_emission_scaled_in1_tmp: f32 = 0.001953f;
    var opaque_emission_scaled_out: f32;
    var glazing_opacity_out: vec3<f32> = vec3(0f);
    var switch_metalness_out: f32 = 0f;
    var layered_base_color_out: vec3<f32>;
    var glazing_ior_out: f32 = 0f;
    var glazing_transmission_color_computed_out: vec3<f32>;
    var switch_coat_out: f32 = 0f;
    var switch_emission_out: f32 = 0f;
    var switch_opacity_out: vec3<f32> = vec3(0f);
    var switch_base_color_out: vec3<f32> = vec3(0f);
    var switch_specular_ior_out: f32 = 0f;
    var switch_transmission_color_out: vec3<f32> = vec3(0f);
    var surfaceshader_out_1: surfaceshader = surfaceshader(vec3(0f), vec3(0f));

    which_1 = which;
    in1_23 = in1_22;
    in11_9 = in11_8;
    in12_9 = in12_8;
    roughness_33 = roughness_32;
    anisotropy_5 = anisotropy_4;
    rotation_1 = rotation;
    in2_15 = in2_14;
    in21_5 = in21_4;
    f0_1 = f0_;
    value1_1 = value1_;
    in22_3 = in22_2;
    in23_3 = in23_2;
    value11_1 = value11_;
    in13_9 = in13_8;
    in24_3 = in24_2;
    in3_1 = in3_;
    in31_1 = in31_;
    in32_1 = in32_;
    in4_1 = in4_;
    f01_1 = f01_;
    roughness1_3 = roughness1_2;
    anisotropy1_3 = anisotropy1_2;
    rotation1_1 = rotation1_;
    in25_3 = in25_2;
    value12_1 = value12_;
    in14_9 = in14_8;
    in15_9 = in15_8;
    in16_9 = in16_8;
    in17_9 = in17_8;
    in18_9 = in18_8;
    backface_cullingbool_3 = backface_cullingbool_2;
    let _e163 = in12_9;
    let _e164 = in2_15;
    metal_base_color_out = (_e163 * _e164);
    let _e167 = in15_9;
    let _e168 = in25_3;
    layered_f0_weighted_out = (_e167 * _e168);
    let _e173 = layered_oneminus_fraction_in1_tmp;
    let _e174 = in25_3;
    layered_oneminus_fraction_out = (_e173 - _e174);
    let _e189 = which_1;
    NG_switch_floatI(1f, 1f, 1f, 0f, 1f, 0f, 0f, 0f, 0f, 0f, _e189, (&switch_specular_out));
    let _e194 = roughness_33;
    let _e195 = anisotropy_5;
    adsk_NG_adsk_roughness_adjust(_e194, _e195, (&surface_roughness_adjusted_out));
    let _e200 = roughness1_3;
    let _e201 = anisotropy1_3;
    adsk_NG_adsk_roughness_adjust(_e200, _e201, (&layered_roughness_adjusted_out));
    let _e206 = f0_1;
    adsk_NG_adsk_f0_to_ior(_e206, (&opaque_ior_out));
    let _e212 = in18_9;
    mx_luminance_color3_(_e212, vec3<f32>(0.272229f, 0.674082f, 0.053689f), (&glazing_f0_luminance_out));
    let _e221 = anisotropy_5;
    adsk_NG_adsk_anisotropy_adjust(_e221, (&surface_anisotropy_adjusted_out));
    let _e226 = anisotropy1_3;
    adsk_NG_adsk_anisotropy_adjust(_e226, (&layered_anisotropy_adjusted_out));
    let _e231 = rotation_1;
    adsk_NG_adsk_rotation_normalize(_e231, (&surface_rotation_normalized_out));
    let _e236 = rotation1_1;
    adsk_NG_adsk_rotation_normalize(_e236, (&layered_rotation_normalized_out));
    let _e251 = which_1;
    NG_switch_floatI(0f, 0f, 1f, 0f, 1f, 0f, 0f, 0f, 0f, 0f, _e251, (&switch_transmission_out));
    let _e258 = in32_1;
    let _e266 = which_1;
    NG_switch_floatI(0f, 0f, _e258, 0f, 0f, 0f, 0f, 0f, 0f, 0f, _e266, (&switch_transmission_depth_out));
    let _e269 = in17_9;
    let _e270 = roughness_33;
    glazing_transmission_extra_roughness_out = (_e269 - _e270);
    let _e279 = value1_1;
    let _e280 = opaque_subsurface_value2_tmp;
    if (_e279 == _e280) {
        let _e282 = opaque_subsurface_in1_tmp;
        local_47 = _e282;
    } else {
        let _e283 = opaque_subsurface_in2_tmp;
        local_47 = _e283;
    }
    let _e285 = local_47;
    opaque_subsurface_out = _e285;
    let _e294 = in21_5;
    let _e327 = which_1;
    NG_switch_color3I(vec3<f32>(1f, 1f, 1f), _e294, vec3<f32>(1f, 1f, 1f), vec3<f32>(1f, 1f, 1f), vec3<f32>(1f, 1f, 1f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), _e327, (&switch_subsurface_color_out));
    let _e337 = in22_3;
    let _e370 = which_1;
    NG_switch_color3I(vec3<f32>(1f, 1f, 1f), _e337, vec3<f32>(1f, 1f, 1f), vec3<f32>(1f, 1f, 1f), vec3<f32>(1f, 1f, 1f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), _e370, (&switch_subsurface_radius_out));
    let _e376 = in23_3;
    let _e385 = which_1;
    NG_switch_floatI(1f, _e376, 1f, 1f, 1f, 0f, 0f, 0f, 0f, 0f, _e385, (&switch_subsurface_scale_out));
    let _e391 = in12_9;
    mx_luminance_color3_(_e391, vec3<f32>(0.272229f, 0.674082f, 0.053689f), (&layered_coat_value_color3_out));
    let _e400 = f01_1;
    adsk_NG_adsk_f0_to_ior(_e400, (&layered_coat_ior_out));
    let _e418 = in1_23;
    let _e443 = which_1;
    NG_switch_vector3I(vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), _e418, vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), _e443, (&switch_coat_normal_out));
    let _e450 = value11_1;
    let _e451 = opaque_emission_value_value2_tmp;
    if (_e450 == _e451) {
        let _e453 = in13_9;
        local_48 = _e453;
    } else {
        let _e454 = opaque_emission_value_in2_tmp;
        local_48 = _e454;
    }
    let _e456 = local_48;
    opaque_emission_value_out = _e456;
    let _e465 = in24_3;
    let _e498 = which_1;
    NG_switch_color3I(vec3<f32>(1f, 1f, 1f), _e465, vec3<f32>(1f, 1f, 1f), vec3<f32>(1f, 1f, 1f), vec3<f32>(1f, 1f, 1f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), _e498, (&switch_emission_color_out));
    let _e503 = glazing_cutout_float_index_tmp;
    let _e505 = in11_9[_e503];
    glazing_cutout_float_out = _e505;
    let _e509 = which_1;
    let _e510 = switch_thin_walled_value2_tmp;
    if (_e509 == _e510) {
        local_49 = true;
    } else {
        local_49 = false;
    }
    let _e515 = local_49;
    switch_thin_walled_out = _e515;
    let _e520 = in1_23;
    let _e521 = in1_23;
    let _e522 = in1_23;
    let _e523 = in4_1;
    let _e524 = in1_23;
    let _e545 = which_1;
    NG_switch_vector3I(_e520, _e521, _e522, _e523, _e524, vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), _e545, (&switch_normal_out));
    let _e548 = vd;
    geomprop_Tworld_out1_ = normalize(_e548.tangentWorld);
    let _e554 = value12_1;
    let _e555 = layered_metalness_value2_tmp;
    if (_e554 == _e555) {
        let _e557 = layered_oneminus_fraction_out;
        local_50 = _e557;
    } else {
        let _e558 = in25_3;
        local_50 = _e558;
    }
    let _e560 = local_50;
    layered_metalness_out = _e560;
    let _e562 = in14_9;
    let _e563 = layered_oneminus_fraction_out;
    layered_diffuse_weighted_out = (_e562 * _e563);
    let _e571 = surface_roughness_adjusted_out;
    let _e578 = which_1;
    NG_switch_floatI(0f, 0f, 0f, _e571, 0f, 0f, 0f, 0f, 0f, 0f, _e578, (&switch_coat_roughness_out));
    let _e583 = surface_roughness_adjusted_out;
    let _e584 = surface_roughness_adjusted_out;
    let _e585 = surface_roughness_adjusted_out;
    let _e586 = layered_roughness_adjusted_out;
    let _e587 = surface_roughness_adjusted_out;
    let _e593 = which_1;
    NG_switch_floatI(_e583, _e584, _e585, _e586, _e587, 0f, 0f, 0f, 0f, 0f, _e593, (&switch_specular_roughness_out));
    let _e598 = glazing_f0_float_index_tmp;
    let _e600 = glazing_f0_luminance_out[_e598];
    glazing_f0_float_out = _e600;
    let _e607 = glazing_oneminus_f0_in1_tmp;
    let _e608 = glazing_f0_luminance_out;
    glazing_oneminus_f0_out = (_e607 - _e608);
    let _e616 = surface_anisotropy_adjusted_out;
    let _e623 = which_1;
    NG_switch_floatI(0f, 0f, 0f, _e616, 0f, 0f, 0f, 0f, 0f, 0f, _e623, (&switch_coat_anisotropy_out));
    let _e628 = surface_anisotropy_adjusted_out;
    let _e629 = surface_anisotropy_adjusted_out;
    let _e630 = surface_anisotropy_adjusted_out;
    let _e631 = layered_anisotropy_adjusted_out;
    let _e632 = surface_anisotropy_adjusted_out;
    let _e638 = which_1;
    NG_switch_floatI(_e628, _e629, _e630, _e631, _e632, 0f, 0f, 0f, 0f, 0f, _e638, (&switch_specular_anisotropy_out));
    let _e646 = surface_rotation_normalized_out;
    let _e653 = which_1;
    NG_switch_floatI(0f, 0f, 0f, _e646, 0f, 0f, 0f, 0f, 0f, 0f, _e653, (&switch_coat_rotation_out));
    let _e658 = surface_rotation_normalized_out;
    let _e659 = surface_rotation_normalized_out;
    let _e660 = surface_rotation_normalized_out;
    let _e661 = layered_rotation_normalized_out;
    let _e662 = surface_rotation_normalized_out;
    let _e668 = which_1;
    NG_switch_floatI(_e658, _e659, _e660, _e661, _e662, 0f, 0f, 0f, 0f, 0f, _e668, (&switch_specular_rotation_out));
    let _e677 = glazing_transmission_extra_roughness_out;
    let _e683 = which_1;
    NG_switch_floatI(0f, 0f, 0f, 0f, _e677, 0f, 0f, 0f, 0f, 0f, _e683, (&switch_transmission_extra_roughness_out));
    let _e689 = opaque_subsurface_out;
    let _e698 = which_1;
    NG_switch_floatI(0f, _e689, 0f, 0f, 0f, 0f, 0f, 0f, 0f, 0f, _e698, (&switch_subsurface_out));
    let _e703 = layered_coat_value_index_tmp;
    let _e705 = layered_coat_value_color3_out[_e703];
    layered_coat_value_out = _e705;
    let _e712 = layered_coat_ior_out;
    let _e719 = which_1;
    NG_switch_floatI(1.5f, 1.5f, 1.5f, _e712, 1.5f, 0f, 0f, 0f, 0f, 0f, _e719, (&switch_coat_ior_out));
    let _e724 = opaque_emission_scaled_in1_tmp;
    let _e725 = opaque_emission_value_out;
    opaque_emission_scaled_out = (_e724 * _e725);
    let _e731 = backface_cullingbool_3;
    let _e732 = glazing_cutout_float_out;
    mx_backface_util(_e731, _e732, (&glazing_opacity_out));
    let _e740 = layered_metalness_out;
    let _e747 = which_1;
    NG_switch_floatI(1f, 0f, 0f, _e740, 0f, 0f, 0f, 0f, 0f, 0f, _e747, (&switch_metalness_out));
    let _e750 = layered_f0_weighted_out;
    let _e751 = layered_diffuse_weighted_out;
    layered_base_color_out = (_e750 + _e751);
    let _e756 = glazing_f0_float_out;
    adsk_NG_adsk_f0_to_ior(_e756, (&glazing_ior_out));
    let _e759 = in16_9;
    let _e760 = glazing_oneminus_f0_out;
    glazing_transmission_color_computed_out = (_e759 / _e760);
    let _e768 = layered_coat_value_out;
    let _e775 = which_1;
    NG_switch_floatI(0f, 0f, 0f, _e768, 0f, 0f, 0f, 0f, 0f, 0f, _e775, (&switch_coat_out));
    let _e781 = opaque_emission_scaled_out;
    let _e790 = which_1;
    NG_switch_floatI(0f, _e781, 0f, 0f, 0f, 0f, 0f, 0f, 0f, 0f, _e790, (&switch_emission_out));
    let _e796 = in11_9;
    let _e797 = in11_9;
    let _e798 = in11_9;
    let _e799 = in11_9;
    let _e800 = glazing_opacity_out;
    let _e821 = which_1;
    NG_switch_color3I(_e796, _e797, _e798, _e799, _e800, vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), _e821, (&switch_opacity_out));
    let _e827 = metal_base_color_out;
    let _e828 = in21_5;
    let _e833 = layered_base_color_out;
    let _e858 = which_1;
    NG_switch_color3I(_e827, _e828, vec3<f32>(1f, 1f, 1f), _e833, vec3<f32>(1f, 1f, 1f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), _e858, (&switch_base_color_out));
    let _e864 = opaque_ior_out;
    let _e865 = in3_1;
    let _e867 = glazing_ior_out;
    let _e873 = which_1;
    NG_switch_floatI(1.5f, _e864, _e865, 1.5f, _e867, 0f, 0f, 0f, 0f, 0f, _e873, (&switch_specular_ior_out));
    let _e887 = in31_1;
    let _e892 = glazing_transmission_color_computed_out;
    let _e913 = which_1;
    NG_switch_color3I(vec3<f32>(1f, 1f, 1f), vec3<f32>(1f, 1f, 1f), _e887, vec3<f32>(1f, 1f, 1f), _e892, vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), vec3<f32>(0f, 0f, 0f), _e913, (&switch_transmission_color_out));
    let _e923 = switch_base_color_out;
    let _e925 = switch_metalness_out;
    let _e926 = switch_specular_out;
    let _e927 = in12_9;
    let _e928 = switch_specular_roughness_out;
    let _e929 = switch_specular_ior_out;
    let _e930 = switch_specular_anisotropy_out;
    let _e931 = switch_specular_rotation_out;
    let _e932 = switch_transmission_out;
    let _e933 = switch_transmission_color_out;
    let _e934 = switch_transmission_depth_out;
    let _e941 = switch_transmission_extra_roughness_out;
    let _e942 = switch_subsurface_out;
    let _e943 = switch_subsurface_color_out;
    let _e944 = switch_subsurface_radius_out;
    let _e945 = switch_subsurface_scale_out;
    let _e953 = switch_coat_out;
    let _e958 = switch_coat_roughness_out;
    let _e959 = switch_coat_anisotropy_out;
    let _e960 = switch_coat_rotation_out;
    let _e961 = switch_coat_ior_out;
    let _e962 = switch_coat_normal_out;
    let _e967 = switch_emission_out;
    let _e968 = switch_emission_color_out;
    let _e969 = switch_opacity_out;
    let _e970 = switch_thin_walled_out;
    let _e971 = switch_normal_out;
    let _e972 = geomprop_Tworld_out1_;
    NG_standard_surface_surfaceshader_optim(1f, _e923, 0f, _e925, _e926, _e927, _e928, _e929, _e930, _e931, _e932, _e933, _e934, vec3<f32>(0f, 0f, 0f), 0f, 0f, _e941, _e942, _e943, _e944, _e945, 0f, 0f, vec3<f32>(1f, 1f, 1f), 0.3f, _e953, vec3<f32>(1f, 1f, 1f), _e958, _e959, _e960, _e961, _e962, 0f, 0f, 0f, 1.5f, _e967, _e968, _e969, _e970, _e971, _e972, (&surfaceshader_out_1));
    let _e975 = surfaceshader_out_1;
    (*out1_9) = _e975;
    return;
}

fn main_1() {
    var geomprop_Nworld_out1_: vec3<f32>;
    var surfaceshader_out_2: surfaceshader = surfaceshader(vec3(0f), vec3(0f));
    var material_out: surfaceshader;

    let _e98 = vd;
    geomprop_Nworld_out1_ = normalize(_e98.normalWorld);
    let _e108 = global_1.material_type;
    let _e109 = geomprop_Nworld_out1_;
    let _e110 = global_1.surface_cutout;
    let _e111 = global_1.surface_albedo;
    let _e112 = global_1.surface_roughness;
    let _e113 = global_1.surface_anisotropy;
    let _e114 = global_1.surface_rotation;
    let _e115 = global_1.metal_f0_;
    let _e116 = global_1.opaque_albedo;
    let _e117 = global_1.opaque_f0_;
    let _e118 = global_1.opaque_translucency;
    let _e120 = global_1.opaque_mfp_modifier;
    let _e121 = global_1.opaque_mfp;
    let _e122 = global_1.opaque_emission;
    let _e124 = global_1.opaque_luminance;
    let _e125 = global_1.opaque_luminance_modifier;
    let _e126 = global_1.transparent_ior;
    let _e127 = global_1.transparent_color;
    let _e128 = global_1.transparent_distance;
    let _e129 = geomprop_Nworld_out1_;
    let _e130 = global_1.layered_f0_;
    let _e131 = global_1.layered_roughness;
    let _e132 = global_1.layered_anisotropy;
    let _e133 = global_1.layered_rotation;
    let _e134 = global_1.layered_fraction;
    let _e135 = global_1.layered_fraction_invert;
    let _e137 = global_1.layered_diffuse;
    let _e138 = global_1.layered_bottom_f0_;
    let _e139 = global_1.glazing_transmission_color;
    let _e140 = global_1.glazing_transmission_roughness;
    let _e141 = global_1.glazing_f0_;
    let _e142 = global_1.glazing_backface_culling;
    adsk_NG_adsk_unified_material(_e108, _e109, _e110, _e111, _e112, _e113, _e114, _e115, _e116, _e117, bool(_e118), _e120, _e121, bool(_e122), _e124, _e125, _e126, _e127, _e128, _e129, _e130, _e131, _e132, _e133, _e134, bool(_e135), _e137, _e138, _e139, _e140, _e141, bool(_e142), (&surfaceshader_out_2));
    let _e146 = surfaceshader_out_2;
    material_out = _e146;
    let _e148 = material_out;
    out1_10 = vec4<f32>(_e148.color.x, _e148.color.y, _e148.color.z, 1f);
    return;
}

@fragment 
fn main(@location(0) normalWorld: vec3<f32>, @location(1) tangentWorld: vec3<f32>, @location(2) positionWorld: vec3<f32>, @builtin(front_facing) gl_FrontFacing: bool) -> FragmentOutput {
    vd.normalWorld = normalWorld;
    vd.tangentWorld = tangentWorld;
    vd.positionWorld = positionWorld;
    gl_FrontFacing_1 = gl_FrontFacing;
    main_1();
    let _e117 = out1_10;
    return FragmentOutput(_e117);
}
