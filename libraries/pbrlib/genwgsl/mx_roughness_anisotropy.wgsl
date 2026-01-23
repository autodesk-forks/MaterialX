fn mx_roughness_anisotropy(roughness: f32, anisotropy: f32, result: ptr<function, vec2<f32>>) {
    var roughness_1: f32;
    var anisotropy_1: f32;
    var roughness_sqr: f32;
    var aspect: f32;

    roughness_1 = roughness;
    anisotropy_1 = anisotropy;
    let _e6 = roughness_1;
    let _e7 = roughness_1;
    roughness_sqr = clamp((_e6 * _e7), 0.00000001f, 1f);
    let _e13 = anisotropy_1;
    if (_e13 > 0f) {
        {
            let _e17 = anisotropy_1;
            aspect = sqrt((1f - clamp(_e17, 0f, 0.98f)));
            let _e25 = roughness_sqr;
            let _e26 = aspect;
            (*result).x = min((_e25 / _e26), 1f);
            let _e31 = roughness_sqr;
            let _e32 = aspect;
            (*result).y = (_e31 * _e32);
            return;
        }
    } else {
        {
            let _e35 = roughness_sqr;
            (*result).x = _e35;
            let _e37 = roughness_sqr;
            (*result).y = _e37;
            return;
        }
    }
}