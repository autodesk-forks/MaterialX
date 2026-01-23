fn mx_matrix_mul(v: vec2<f32>, m: mat2x2<f32>) -> vec2<f32> {
    var v_1: vec2<f32>;
    var m_1: mat2x2<f32>;

    v_1 = v;
    m_1 = m;
    let _e4 = v_1;
    let _e5 = m_1;
    return (_e4 * _e5);
}

fn mx_matrix_mul_1(v_2: vec3<f32>, m_2: mat3x3<f32>) -> vec3<f32> {
    var v_3: vec3<f32>;
    var m_3: mat3x3<f32>;

    v_3 = v_2;
    m_3 = m_2;
    let _e4 = v_3;
    let _e5 = m_3;
    return (_e4 * _e5);
}

fn mx_matrix_mul_2(v_4: vec4<f32>, m_4: mat4x4<f32>) -> vec4<f32> {
    var v_5: vec4<f32>;
    var m_5: mat4x4<f32>;

    v_5 = v_4;
    m_5 = m_4;
    let _e4 = v_5;
    let _e5 = m_5;
    return (_e4 * _e5);
}

fn mx_matrix_mul_3(m_6: mat2x2<f32>, v_6: vec2<f32>) -> vec2<f32> {
    var m_7: mat2x2<f32>;
    var v_7: vec2<f32>;

    m_7 = m_6;
    v_7 = v_6;
    let _e4 = m_7;
    let _e5 = v_7;
    return (_e4 * _e5);
}

fn mx_matrix_mul_4(m_8: mat3x3<f32>, v_8: vec3<f32>) -> vec3<f32> {
    var m_9: mat3x3<f32>;
    var v_9: vec3<f32>;

    m_9 = m_8;
    v_9 = v_8;
    let _e4 = m_9;
    let _e5 = v_9;
    return (_e4 * _e5);
}

fn mx_matrix_mul_5(m_10: mat4x4<f32>, v_10: vec4<f32>) -> vec4<f32> {
    var m_11: mat4x4<f32>;
    var v_11: vec4<f32>;

    m_11 = m_10;
    v_11 = v_10;
    let _e4 = m_11;
    let _e5 = v_11;
    return (_e4 * _e5);
}

fn mx_matrix_mul_6(m1_: mat2x2<f32>, m2_: mat2x2<f32>) -> mat2x2<f32> {
    var m1_1: mat2x2<f32>;
    var m2_1: mat2x2<f32>;

    m1_1 = m1_;
    m2_1 = m2_;
    let _e4 = m1_1;
    let _e5 = m2_1;
    return (_e4 * _e5);
}

fn mx_matrix_mul_7(m1_2: mat3x3<f32>, m2_2: mat3x3<f32>) -> mat3x3<f32> {
    var m1_3: mat3x3<f32>;
    var m2_3: mat3x3<f32>;

    m1_3 = m1_2;
    m2_3 = m2_2;
    let _e4 = m1_3;
    let _e5 = m2_3;
    return (_e4 * _e5);
}

fn mx_matrix_mul_8(m1_4: mat4x4<f32>, m2_4: mat4x4<f32>) -> mat4x4<f32> {
    var m1_5: mat4x4<f32>;
    var m2_5: mat4x4<f32>;

    m1_5 = m1_4;
    m2_5 = m2_4;
    let _e4 = m1_5;
    let _e5 = m2_5;
    return (_e4 * _e5);
}

fn mx_square(x: f32) -> f32 {
    var x_1: f32;

    x_1 = x;
    let _e2 = x_1;
    let _e3 = x_1;
    return (_e2 * _e3);
}

fn mx_square_1(x_2: vec2<f32>) -> vec2<f32> {
    var x_3: vec2<f32>;

    x_3 = x_2;
    let _e2 = x_3;
    let _e3 = x_3;
    return (_e2 * _e3);
}

fn mx_square_2(x_4: vec3<f32>) -> vec3<f32> {
    var x_5: vec3<f32>;

    x_5 = x_4;
    let _e2 = x_5;
    let _e3 = x_5;
    return (_e2 * _e3);
}

fn mx_transformmatrix_vector3M4_(val: vec3<f32>, transform: mat4x4<f32>, result: ptr<function, vec3<f32>>) {
    var val_1: vec3<f32>;
    var transform_1: mat4x4<f32>;
    var res: vec4<f32>;

    val_1 = val;
    transform_1 = transform;
    let _e6 = transform_1;
    let _e7 = val_1;
    let _e13 = mx_matrix_mul_5(_e6, vec4<f32>(_e7.x, _e7.y, _e7.z, 1f));
    res = _e13;
    let _e15 = res;
    (*result) = _e15.xyz;
    return;
}