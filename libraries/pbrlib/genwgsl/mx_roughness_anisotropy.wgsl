fn mx_roughness_anisotropy(roughness: f32, anisotropy: f32, result: ptr<function, vec2<f32>>) {
    let roughness_sqr: f32 = clamp(roughness * roughness, 0.00000001f, 1f);
    if (anisotropy > 0f) {
        let aspect: f32 = sqrt(1f - clamp(anisotropy, 0f, 0.98f));
        (*result).x = min(roughness_sqr / aspect, 1f);
        (*result).y = roughness_sqr * aspect;
    } else {
        (*result).x = roughness_sqr;
        (*result).y = roughness_sqr;
    }
    return;
}
