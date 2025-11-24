fn mx_transformmatrix_vector2M3_(val: vec2<f32>, transform: mat3x3<f32>, result: ptr<function, vec2<f32>>) {
    (*result) = mx_matrix_mul_4(transform, vec3<f32>(val.x, val.y, 1f)).xy;
    return;
}
