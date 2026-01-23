// Main function: mx_burn_color3
// Included from mx_burn_float.glsl (WGSL)
fn mx_burn_float(fg: f32, bg: f32, mixval: f32, result: ptr<function, f32>) {
    var fg_1: f32;
    var bg_1: f32;
    var mixval_1: f32;

    fg_1 = fg;
    bg_1 = bg;
    mixval_1 = mixval;
    let _e8 = fg_1;
    if (abs(_e8) < 0.00000001f) {
        {
            (*result) = 0f;
            return;
        }
    }
    let _e13 = mixval_1;
    let _e16 = bg_1;
    let _e18 = fg_1;
    let _e23 = mixval_1;
    let _e25 = bg_1;
    (*result) = ((_e13 * (1f - ((1f - _e16) / _e18))) + ((1f - _e23) * _e25));
    return;
}

// Main function from mx_burn_color3.glsl
fn mx_burn_color3_(fg_2: vec3<f32>, bg_2: vec3<f32>, mixval_2: f32, result_1: ptr<function, vec3<f32>>) {
    var fg_3: vec3<f32>;
    var bg_3: vec3<f32>;
    var mixval_3: f32;
    var f: f32;

    fg_3 = fg_2;
    bg_3 = bg_2;
    mixval_3 = mixval_2;
    let _e9 = fg_3;
    let _e11 = bg_3;
    let _e13 = mixval_3;
    mx_burn_float(_e9.x, _e11.x, _e13, (&f));
    let _e17 = f;
    (*result_1).x = _e17;
    let _e18 = fg_3;
    let _e20 = bg_3;
    let _e22 = mixval_3;
    mx_burn_float(_e18.y, _e20.y, _e22, (&f));
    let _e26 = f;
    (*result_1).y = _e26;
    let _e27 = fg_3;
    let _e29 = bg_3;
    let _e31 = mixval_3;
    mx_burn_float(_e27.z, _e29.z, _e31, (&f));
    let _e35 = f;
    (*result_1).z = _e35;
    return;
}