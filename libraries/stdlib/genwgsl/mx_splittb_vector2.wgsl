fn mx_splittb_vector2_(valuet: vec2<f32>, valueb: vec2<f32>, center: f32, texcoord: vec2<f32>, result: ptr<function, vec2<f32>>) {
    (*result) = mix(valuet, valueb, vec2(mx_aastep(center, texcoord.y)));
    return;
}
