fn mx_creatematrix_vector4_matrix44_(in1_: vec4<f32>, in2_: vec4<f32>, in3_: vec4<f32>, in4_: vec4<f32>, result: ptr<function, mat4x4<f32>>) {
    var in1_1: vec4<f32>;
    var in2_1: vec4<f32>;
    var in3_1: vec4<f32>;
    var in4_1: vec4<f32>;

    in1_1 = in1_;
    in2_1 = in2_;
    in3_1 = in3_;
    in4_1 = in4_;
    let _e10 = in1_1;
    let _e12 = in1_1;
    let _e14 = in1_1;
    let _e16 = in1_1;
    let _e18 = in2_1;
    let _e20 = in2_1;
    let _e22 = in2_1;
    let _e24 = in2_1;
    let _e26 = in3_1;
    let _e28 = in3_1;
    let _e30 = in3_1;
    let _e32 = in3_1;
    let _e34 = in4_1;
    let _e36 = in4_1;
    let _e38 = in4_1;
    let _e40 = in4_1;
    (*result) = mat4x4<f32>(vec4<f32>(_e10.x, _e12.y, _e14.z, _e16.w), vec4<f32>(_e18.x, _e20.y, _e22.z, _e24.w), vec4<f32>(_e26.x, _e28.y, _e30.z, _e32.w), vec4<f32>(_e34.x, _e36.y, _e38.z, _e40.w));
    return;
}