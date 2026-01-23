// Main function: mx_anisotropic_vdf
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

// Main function from mx_anisotropic_vdf.glsl
fn mx_anisotropic_vdf(closureData: ClosureData, absorption: vec3<f32>, scattering: vec3<f32>, anisotropy: f32, bsdf: ptr<function, BSDF>) {
    var closureData_1: ClosureData;
    var absorption_1: vec3<f32>;
    var scattering_1: vec3<f32>;
    var anisotropy_1: f32;

    closureData_1 = closureData;
    absorption_1 = absorption;
    scattering_1 = scattering;
    anisotropy_1 = anisotropy;
    return;
}