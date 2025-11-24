fn mx_smoothstep_float(val: f32, low: f32, high: f32, result: ptr<function, f32>) {
    if (val >= high) {
        (*result) = 1f;
        return;
    } else {
        if (val <= low) {
            (*result) = 0f;
            return;
        } else {
            (*result) = smoothstep(low, high, val);
            return;
        }
    }
}
