// Main function: mx_splitlr_vector2
// Included from mx_aastep.glsl (WGSL)
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

// Main function from mx_splitlr_vector2.glsl
fn mx_splitlr_vector2_(valuel: vec2<f32>, valuer: vec2<f32>, center: f32, texcoord: vec2<f32>, result: ptr<function, vec2<f32>>) {
    var valuel_1: vec2<f32>;
    var valuer_1: vec2<f32>;
    var center_1: f32;
    var texcoord_1: vec2<f32>;

    valuel_1 = valuel;
    valuer_1 = valuer;
    center_1 = center;
    texcoord_1 = texcoord;
    let _e10 = valuel_1;
    let _e11 = valuer_1;
    let _e12 = center_1;
    let _e13 = texcoord_1;
    let _e15 = mx_aastep(_e12, _e13.x);
    (*result) = mix(_e10, _e11, vec2(_e15));
    return;
}