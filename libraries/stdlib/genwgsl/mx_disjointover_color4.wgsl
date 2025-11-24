fn mx_disjointover_color4_(fg: vec4<f32>, bg: vec4<f32>, mixval: f32, result: ptr<function, vec4<f32>>) {
    let summedAlpha: f32 = fg.w + bg.w;
    if (summedAlpha <= 1f) {
        (*result).xyz = fg.xyz + bg.xyz;
    } else {
        if (abs(bg.w) < 0.00000001f) {
            (*result).xyz = vec3(0f);
        } else {
            let scale: f32 = (1f - fg.w) / bg.w;
            (*result).xyz = fg.xyz + (bg.xyz * scale);
        }
    }
    (*result).w = min(summedAlpha, 1f);
    (*result).xyz = ((*result).xyz * mixval) + ((1f - mixval) * bg.xyz);
    (*result).w = ((*result).w * mixval) + ((1f - mixval) * bg.w);
    return;
}
