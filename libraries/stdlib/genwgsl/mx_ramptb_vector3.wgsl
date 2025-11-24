fn mx_ramptb_vector3_(valuet: vec3<f32>, valueb: vec3<f32>, texcoord: vec2<f32>, result: ptr<function, vec3<f32>>) {
    (*result) = mix(valuet, valueb, vec3(clamp(texcoord.y, 0f, 1f)));
    return;
}
