fn mx_roughness_dual(roughness: vec2<f32>, result: ptr<function, vec2<f32>>) {
    var r: vec2<f32> = roughness;
    if (r.y < 0f) {
        r.y = r.x;
    }
    (*result).x = clamp(r.x * r.x, 0.00000001f, 1f);
    (*result).y = clamp(r.y * r.y, 0.00000001f, 1f);
    return;
}
