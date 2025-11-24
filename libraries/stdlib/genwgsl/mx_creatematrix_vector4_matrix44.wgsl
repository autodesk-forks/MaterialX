fn mx_creatematrix_vector4_matrix44_(in1: vec4<f32>, in2: vec4<f32>, in3: vec4<f32>, in4: vec4<f32>, result: ptr<function, mat4x4<f32>>) {
    (*result) = mat4x4<f32>(in1, in2, in3, in4);
    return;
}
