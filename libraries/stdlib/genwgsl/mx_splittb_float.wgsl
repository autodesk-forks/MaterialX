fn mx_splittb_float(valuet: f32, valueb: f32, center: f32, texcoord: vec2<f32>, result: ptr<function, f32>) {
    (*result) = mix(valuet, valueb, mx_aastep(center, texcoord.y));
    return;
}
