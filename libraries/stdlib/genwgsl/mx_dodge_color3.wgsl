fn mx_dodge_float(fg: f32, bg: f32, mixval: f32, result: ptr<function, f32>) {
    if (abs(1f - fg) < 0.00000001f) {
        (*result) = 0f;
        return;
    }
    (*result) = (mixval * (bg / (1f - fg))) + ((1f - mixval) * bg);
    return;
}

fn mx_dodge_color3_(fg_2: vec3<f32>, bg_2: vec3<f32>, mixval_2: f32, result_1: ptr<function, vec3<f32>>) {
    var f: f32;
    mx_dodge_float(fg_2.x, bg_2.x, mixval_2, (&f));
    (*result_1).x = f;
    mx_dodge_float(fg_2.y, bg_2.y, mixval_2, (&f));
    (*result_1).y = f;
    mx_dodge_float(fg_2.z, bg_2.z, mixval_2, (&f));
    (*result_1).z = f;
    return;
}
