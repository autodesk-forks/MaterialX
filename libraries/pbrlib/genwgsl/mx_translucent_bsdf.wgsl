// Main function: mx_translucent_bsdf
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

// Main function from mx_translucent_bsdf.glsl
fn mx_translucent_bsdf(closureData: ClosureData, weight: f32, color: vec3<f32>, N_6: vec3<f32>, bsdf: ptr<function, BSDF>) {
    var closureData_1: ClosureData;
    var weight_1: f32;
    var color_1: vec3<f32>;
    var N_7: vec3<f32>;
    var V_4: vec3<f32>;
    var L_2: vec3<f32>;
    var NdotL: f32;
    var Li: vec3<f32>;

    closureData_1 = closureData;
    weight_1 = weight;
    color_1 = color;
    N_7 = N_6;
    (*bsdf).throughput = vec3(0f);
    let _e13 = weight_1;
    if (_e13 < 0.00000001f) {
        {
            return;
        }
    }
    let _e16 = closureData_1;
    V_4 = _e16.V;
    let _e19 = closureData_1;
    L_2 = _e19.L;
    let _e22 = N_7;
    N_7 = -(_e22);
    let _e24 = closureData_1;
    if (_e24.closureType == 1i) {
        {
            let _e28 = N_7;
            let _e29 = L_2;
            NdotL = clamp(dot(_e28, _e29), 0f, 1f);
            let _e36 = color_1;
            let _e37 = weight_1;
            let _e39 = NdotL;
            (*bsdf).response = (((_e36 * _e37) * _e39) * 0.31830987f);
            return;
        }
    } else {
        let _e45 = closureData_1;
        if (_e45.closureType == 3i) {
            {
                let _e49 = N_7;
                let _e50 = mx_environment_irradiance(_e49);
                Li = _e50;
                let _e53 = Li;
                let _e54 = color_1;
                let _e56 = weight_1;
                (*bsdf).response = ((_e53 * _e54) * _e56);
                return;
            }
        } else {
            return;
        }
    }
}