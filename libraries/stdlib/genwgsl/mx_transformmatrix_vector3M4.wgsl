fn mx_transformmatrix_vector3M4_(val: vec3<f32>, transform: mat4x4<f32>, result: ptr<function, vec3<f32>>) {
    (*result) = mx_matrix_mul_5(transform, vec4<f32>(val.x, val.y, val.z, 1f)).xyz;
    return;
}
