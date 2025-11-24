fn mx_burn_float(fg: f32, bg: f32, mixval: f32, result: ptr<function, f32>) {
    if (abs(fg) < 0.00000001f) {
        (*result) = 0f;
        return;
    }
    (*result) = (mixval * (1f - ((1f - bg) / fg))) + ((1f - mixval) * bg);
    return;
}
