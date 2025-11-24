fn mx_creatematrix_vector3_matrix33_(in1: vec3<f32>, in2: vec3<f32>, in3: vec3<f32>, result: ptr<function, mat3x3<f32>>) {
    (*result) = mat3x3<f32>(in1, in2, in3);
    return;
}
