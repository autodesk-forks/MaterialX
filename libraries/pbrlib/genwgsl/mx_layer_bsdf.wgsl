// Main function: mx_layer_bsdf
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

// Main function from mx_layer_bsdf.glsl
fn mx_layer_bsdf(closureData: ClosureData, top: BSDF, base: BSDF, result: ptr<function, BSDF>) {
    var closureData_1: ClosureData;
    var top_1: BSDF;
    var base_1: BSDF;

    closureData_1 = closureData;
    top_1 = top;
    base_1 = base;
    let _e8 = top_1;
    let _e13 = top_1;
    if ((length(_e8.response) < 0.0001f) && (length((_e13.throughput - vec3(1f))) < 0.0001f)) {
        {
            let _e22 = base_1;
            (*result) = _e22;
            return;
        }
    } else {
        {
            let _e24 = top_1;
            let _e26 = base_1;
            let _e28 = top_1;
            (*result).response = (_e24.response + (_e26.response * _e28.throughput));
            let _e33 = top_1;
            let _e35 = base_1;
            (*result).throughput = (_e33.throughput * _e35.throughput);
            return;
        }
    }
}