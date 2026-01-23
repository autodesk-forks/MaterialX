fn mx_creatematrix_vector3_matrix33_(in1_: vec3<f32>, in2_: vec3<f32>, in3_: vec3<f32>, result: ptr<function, mat3x3<f32>>) {
    var in1_1: vec3<f32>;
    var in2_1: vec3<f32>;
    var in3_1: vec3<f32>;

    in1_1 = in1_;
    in2_1 = in2_;
    in3_1 = in3_;
    let _e8 = in1_1;
    let _e10 = in1_1;
    let _e12 = in1_1;
    let _e14 = in2_1;
    let _e16 = in2_1;
    let _e18 = in2_1;
    let _e20 = in3_1;
    let _e22 = in3_1;
    let _e24 = in3_1;
    (*result) = mat3x3<f32>(vec3<f32>(_e8.x, _e10.y, _e12.z), vec3<f32>(_e14.x, _e16.y, _e18.z), vec3<f32>(_e20.x, _e22.y, _e24.z));
    return;
}