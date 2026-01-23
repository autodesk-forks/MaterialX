fn mx_smoothstep_float(val: f32, low: f32, high: f32, result: ptr<function, f32>) {
    var val_1: f32;
    var low_1: f32;
    var high_1: f32;

    val_1 = val;
    low_1 = low;
    high_1 = high;
    let _e8 = val_1;
    let _e9 = high_1;
    if (_e8 >= _e9) {
        (*result) = 1f;
        return;
    } else {
        let _e12 = val_1;
        let _e13 = low_1;
        if (_e12 <= _e13) {
            (*result) = 0f;
            return;
        } else {
            let _e16 = low_1;
            let _e17 = high_1;
            let _e18 = val_1;
            (*result) = smoothstep(_e16, _e17, _e18);
            return;
        }
    }
}