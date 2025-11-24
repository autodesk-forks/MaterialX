fn mx_dodge_float(fg: f32, bg: f32, mixval: f32, result: ptr<function, f32>) {
    if (abs(1f - fg) < 0.00000001f) {
        (*result) = 0f;
        return;
    }
    (*result) = (mixval * (bg / (1f - fg))) + ((1f - mixval) * bg);
    return;
}
