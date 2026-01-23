// Main function: mx_mix_edf
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

// Main function from mx_mix_edf.glsl
fn mx_mix_edf(closureData: ClosureData, fg: vec3<f32>, bg: vec3<f32>, mixValue: f32, result: ptr<function, vec3<f32>>) {
    var closureData_1: ClosureData;
    var fg_1: vec3<f32>;
    var bg_1: vec3<f32>;
    var mixValue_1: f32;

    closureData_1 = closureData;
    fg_1 = fg;
    bg_1 = bg;
    mixValue_1 = mixValue;
    let _e10 = bg_1;
    let _e11 = fg_1;
    let _e12 = mixValue_1;
    (*result) = mix(_e10, _e11, vec3(_e12));
    return;
}