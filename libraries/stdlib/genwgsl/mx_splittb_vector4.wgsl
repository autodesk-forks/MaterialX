fn mx_splittb_vector4_(valuet: vec4<f32>, valueb: vec4<f32>, center: f32, texcoord: vec2<f32>, result: ptr<function, vec4<f32>>) {
    (*result) = mix(valuet, valueb, vec4(mx_aastep(center, texcoord.y)));
    return;
}
