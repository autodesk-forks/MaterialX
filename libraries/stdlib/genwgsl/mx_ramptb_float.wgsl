fn mx_ramptb_float(valuet: f32, valueb: f32, texcoord: vec2<f32>, result: ptr<function, f32>) {
    (*result) = mix(valuet, valueb, clamp(texcoord.y, 0f, 1f));
    return;
}
