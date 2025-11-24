fn mx_splittb_vector3_(valuet: vec3<f32>, valueb: vec3<f32>, center: f32, texcoord: vec2<f32>, result: ptr<function, vec3<f32>>) {
    (*result) = mix(valuet, valueb, vec3(mx_aastep(center, texcoord.y)));
    return;
}
