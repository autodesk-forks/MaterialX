fn mx_ramptb_vector2_(valuet: vec2<f32>, valueb: vec2<f32>, texcoord: vec2<f32>, result: ptr<function, vec2<f32>>) {
    (*result) = mix(valuet, valueb, vec2(clamp(texcoord.y, 0f, 1f)));
    return;
}
