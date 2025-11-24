fn mx_creatematrix_vector3_matrix44_(in1: vec3<f32>, in2: vec3<f32>, in3: vec3<f32>, in4: vec3<f32>, result: ptr<function, mat4x4<f32>>) {
    (*result) = mat4x4<f32>(vec4<f32>(in1, 0f), vec4<f32>(in2, 0f), vec4<f32>(in3, 0f), vec4<f32>(in4, 1f));
    return;
}
