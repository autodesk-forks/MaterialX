fn mx_dodge_float(fg: f32, bg: f32, mixval: f32, result: ptr<function, f32>) {
    var fg_1: f32;
    var bg_1: f32;
    var mixval_1: f32;

    fg_1 = fg;
    bg_1 = bg;
    mixval_1 = mixval;
    let _e9 = fg_1;
    if (abs((1f - _e9)) < 0.00000001f) {
        {
            (*result) = 0f;
            return;
        }
    }
    let _e15 = mixval_1;
    let _e16 = bg_1;
    let _e18 = fg_1;
    let _e23 = mixval_1;
    let _e25 = bg_1;
    (*result) = ((_e15 * (_e16 / (1f - _e18))) + ((1f - _e23) * _e25));
    return;
}