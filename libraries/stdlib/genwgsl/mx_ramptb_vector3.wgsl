fn mx_ramptb_vector3_(valuet: vec3<f32>, valueb: vec3<f32>, texcoord: vec2<f32>, result: ptr<function, vec3<f32>>) {
    var valuet_1: vec3<f32>;
    var valueb_1: vec3<f32>;
    var texcoord_1: vec2<f32>;

    valuet_1 = valuet;
    valueb_1 = valueb;
    texcoord_1 = texcoord;
    let _e8 = valuet_1;
    let _e9 = valueb_1;
    let _e10 = texcoord_1;
    (*result) = mix(_e8, _e9, vec3(clamp(_e10.y, 0f, 1f)));
    return;
}