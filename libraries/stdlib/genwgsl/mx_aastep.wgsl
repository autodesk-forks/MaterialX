fn mx_aastep(threshold: f32, value: f32) -> f32 {
    var threshold_1: f32;
    var value_1: f32;
    var afwidth: f32;

    threshold_1 = threshold;
    value_1 = value;
    let _e5 = value_1;
    let _e6 = dpdx(_e5);
    let _e7 = value_1;
    let _e8 = dpdy(_e7);
    afwidth = (length(vec2<f32>(_e6, _e8)) * 0.70710677f);
    let _e14 = threshold_1;
    let _e15 = afwidth;
    let _e17 = threshold_1;
    let _e18 = afwidth;
    let _e20 = value_1;
    return smoothstep((_e14 - _e15), (_e17 + _e18), _e20);
}