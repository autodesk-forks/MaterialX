fn mx_creatematrix_vector3_matrix44_(in1_: vec3<f32>, in2_: vec3<f32>, in3_: vec3<f32>, in4_: vec3<f32>, result: ptr<function, mat4x4<f32>>) {
    var in1_1: vec3<f32>;
    var in2_1: vec3<f32>;
    var in3_1: vec3<f32>;
    var in4_1: vec3<f32>;

    in1_1 = in1_;
    in2_1 = in2_;
    in3_1 = in3_;
    in4_1 = in4_;
    let _e10 = in1_1;
    let _e12 = in1_1;
    let _e14 = in1_1;
    let _e17 = in2_1;
    let _e19 = in2_1;
    let _e21 = in2_1;
    let _e24 = in3_1;
    let _e26 = in3_1;
    let _e28 = in3_1;
    let _e31 = in4_1;
    let _e33 = in4_1;
    let _e35 = in4_1;
    (*result) = mat4x4<f32>(vec4<f32>(_e10.x, _e12.y, _e14.z, 0f), vec4<f32>(_e17.x, _e19.y, _e21.z, 0f), vec4<f32>(_e24.x, _e26.y, _e28.z, 0f), vec4<f32>(_e31.x, _e33.y, _e35.z, 1f));
    return;
}