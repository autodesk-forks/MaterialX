// Main function: mx_add_bsdf
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

// Main function from mx_add_bsdf.glsl
fn mx_add_bsdf(closureData: ClosureData, in1_: BSDF, in2_: BSDF, result: ptr<function, BSDF>) {
    var closureData_1: ClosureData;
    var in1_1: BSDF;
    var in2_1: BSDF;

    closureData_1 = closureData;
    in1_1 = in1_;
    in2_1 = in2_;
    let _e9 = in1_1;
    let _e11 = in2_1;
    (*result).response = (_e9.response + _e11.response);
    let _e15 = in1_1;
    let _e17 = in2_1;
    (*result).throughput = max(((_e15.throughput + _e17.throughput) - vec3(1f)), vec3(0f));
    return;
}