fn mx_aastep(threshold: f32, value: f32) -> f32 {
    let afwidth = length(vec2<f32>(dpdx(value), dpdy(value))) * 0.70710677f;
    return smoothstep(threshold - afwidth, threshold + afwidth, value);
}
