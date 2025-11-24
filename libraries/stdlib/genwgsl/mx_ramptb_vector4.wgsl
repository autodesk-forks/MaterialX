fn mx_ramptb_vector4_(valuet: vec4<f32>, valueb: vec4<f32>, texcoord: vec2<f32>, result: ptr<function, vec4<f32>>) {
    (*result) = mix(valuet, valueb, vec4(clamp(texcoord.y, 0f, 1f)));
    return;
}
