fn mx_roughness_dual(roughness: vec2<f32>, result: ptr<function, vec2<f32>>) {
    var roughness_1: vec2<f32>;

    roughness_1 = roughness;
    let _e4 = roughness_1;
    if (_e4.y < 0f) {
        {
            let _e9 = roughness_1;
            roughness_1.y = _e9.x;
        }
    }
    let _e12 = roughness_1;
    let _e14 = roughness_1;
    (*result).x = clamp((_e12.x * _e14.x), 0.00000001f, 1f);
    let _e21 = roughness_1;
    let _e23 = roughness_1;
    (*result).y = clamp((_e21.y * _e23.y), 0.00000001f, 1f);
    return;
}