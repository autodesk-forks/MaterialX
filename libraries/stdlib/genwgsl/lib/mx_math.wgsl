fn mx_matrix_mul(v: vec2<f32>, m: mat2x2<f32>) -> vec2<f32> {
    return v * m;
}

fn mx_matrix_mul_1(v: vec3<f32>, m: mat3x3<f32>) -> vec3<f32> {
    return v * m;
}

fn mx_matrix_mul_2(v: vec4<f32>, m: mat4x4<f32>) -> vec4<f32> {
    return v * m;
}

fn mx_matrix_mul_3(m: mat2x2<f32>, v: vec2<f32>) -> vec2<f32> {
    return m * v;
}

fn mx_matrix_mul_4(m: mat3x3<f32>, v: vec3<f32>) -> vec3<f32> {
    return m * v;
}

fn mx_matrix_mul_5(m: mat4x4<f32>, v: vec4<f32>) -> vec4<f32> {
    return m * v;
}

fn mx_matrix_mul_6(m1: mat2x2<f32>, m2: mat2x2<f32>) -> mat2x2<f32> {
    return m1 * m2;
}

fn mx_matrix_mul_7(m1: mat3x3<f32>, m2: mat3x3<f32>) -> mat3x3<f32> {
    return m1 * m2;
}

fn mx_matrix_mul_8(m1: mat4x4<f32>, m2: mat4x4<f32>) -> mat4x4<f32> {
    return m1 * m2;
}

fn mx_square(x: f32) -> f32 {
    return x * x;
}

fn mx_square_1(x: vec2<f32>) -> vec2<f32> {
    return x * x;
}

fn mx_square_2(x: vec3<f32>) -> vec3<f32> {
    return x * x;
}

fn mx_srgb_encode(color: vec3<f32>) -> vec3<f32> {
    let isAbove: vec3<bool> = color > vec3(0.0031308f);
    let linSeg: vec3<f32> = color * 12.92f;
    let powSeg: vec3<f32> = (1.055f * pow(max(color, vec3(0f)), vec3(0.41666666f))) - vec3(0.055f);
    return select(linSeg, powSeg, isAbove);
}
