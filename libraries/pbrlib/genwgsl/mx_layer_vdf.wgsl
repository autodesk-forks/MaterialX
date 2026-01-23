// Main function: mx_layer_vdf
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

// Main function from mx_layer_vdf.glsl
fn mx_layer_vdf(closureData: ClosureData, top: BSDF, base: BSDF, result: ptr<function, BSDF>) {
    var closureData_1: ClosureData;
    var top_1: BSDF;
    var base_1: BSDF;

    closureData_1 = closureData;
    top_1 = top;
    base_1 = base;
    let _e9 = top_1;
    let _e11 = base_1;
    (*result).response = (_e9.response + _e11.response);
    let _e15 = top_1;
    let _e17 = base_1;
    (*result).throughput = (_e15.throughput + _e17.throughput);
    return;
}