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

fn mx_rotationMatrix(axis: vec3<f32>, angle: f32) -> mat4x4<f32> {
    var axis_1: vec3<f32>;
    var angle_1: f32;
    var s: f32;
    var c: f32;
    var oc: f32;

    axis_1 = axis;
    angle_1 = angle;
    let _e5 = axis_1;
    axis_1 = normalize(_e5);
    let _e7 = angle_1;
    s = sin(_e7);
    let _e10 = angle_1;
    c = cos(_e10);
    let _e14 = c;
    oc = (1f - _e14);
    let _e17 = oc;
    let _e18 = axis_1;
    let _e21 = axis_1;
    let _e24 = c;
    let _e26 = oc;
    let _e27 = axis_1;
    let _e30 = axis_1;
    let _e33 = axis_1;
    let _e35 = s;
    let _e38 = oc;
    let _e39 = axis_1;
    let _e42 = axis_1;
    let _e45 = axis_1;
    let _e47 = s;
    let _e51 = oc;
    let _e52 = axis_1;
    let _e55 = axis_1;
    let _e58 = axis_1;
    let _e60 = s;
    let _e63 = oc;
    let _e64 = axis_1;
    let _e67 = axis_1;
    let _e70 = c;
    let _e72 = oc;
    let _e73 = axis_1;
    let _e76 = axis_1;
    let _e79 = axis_1;
    let _e81 = s;
    let _e85 = oc;
    let _e86 = axis_1;
    let _e89 = axis_1;
    let _e92 = axis_1;
    let _e94 = s;
    let _e97 = oc;
    let _e98 = axis_1;
    let _e101 = axis_1;
    let _e104 = axis_1;
    let _e106 = s;
    let _e109 = oc;
    let _e110 = axis_1;
    let _e113 = axis_1;
    let _e116 = c;
    return mat4x4<f32>(vec4<f32>((((_e17 * _e18.x) * _e21.x) + _e24), (((_e26 * _e27.x) * _e30.y) - (_e33.z * _e35)), (((_e38 * _e39.z) * _e42.x) + (_e45.y * _e47)), 0f), vec4<f32>((((_e51 * _e52.x) * _e55.y) + (_e58.z * _e60)), (((_e63 * _e64.y) * _e67.y) + _e70), (((_e72 * _e73.y) * _e76.z) - (_e79.x * _e81)), 0f), vec4<f32>((((_e85 * _e86.z) * _e89.x) - (_e92.y * _e94)), (((_e97 * _e98.y) * _e101.z) + (_e104.x * _e106)), (((_e109 * _e110.z) * _e113.z) + _e116), 0f), vec4<f32>(0f, 0f, 0f, 1f));
}

fn mx_rotate_vector3_(_in: vec3<f32>, amount: f32, axis_2: vec3<f32>, result: ptr<function, vec3<f32>>) {
    var _in_1: vec3<f32>;
    var amount_1: f32;
    var axis_3: vec3<f32>;
    var rotationRadians: f32;
    var m_12: mat4x4<f32>;

    _in_1 = _in;
    amount_1 = amount;
    axis_3 = axis_2;
    let _e8 = amount_1;
    rotationRadians = radians(_e8);
    let _e11 = axis_3;
    let _e12 = rotationRadians;
    let _e13 = mx_rotationMatrix(_e11, _e12);
    m_12 = _e13;
    let _e15 = m_12;
    let _e16 = _in_1;
    let _e22 = mx_matrix_mul_5(_e15, vec4<f32>(_e16.x, _e16.y, _e16.z, 1f));
    (*result) = _e22.xyz;
    return;
}