const XYZ_to_RGB: mat3x3<f32> = mat3x3<f32>(vec3<f32>(3.2406f, -0.9689f, 0.0557f), vec3<f32>(-1.5372f, 1.8758f, -0.204f), vec3<f32>(-0.4986f, 0.0415f, 1.057f));

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

fn mx_blackbody(temperatureKelvin: f32, colorValue: ptr<function, vec3<f32>>) {
    var temperatureKelvin_1: f32;
    var xc: f32;
    var yc: f32;
    var t: f32;
    var t2_: f32;
    var t3_: f32;
    var xc2_: f32;
    var xc3_: f32;
    var XYZ: vec3<f32>;

    temperatureKelvin_1 = temperatureKelvin;
    let _e12 = temperatureKelvin_1;
    temperatureKelvin_1 = clamp(_e12, 1667f, 25000f);
    let _e17 = temperatureKelvin_1;
    t = (1000f / _e17);
    let _e19 = t;
    let _e20 = t;
    t2_ = (_e19 * _e20);
    let _e22 = t;
    let _e23 = t;
    let _e25 = t;
    t3_ = ((_e22 * _e23) * _e25);
    let _e27 = temperatureKelvin_1;
    if (_e27 < 4000f) {
        {
            let _e32 = t3_;
            let _e35 = t2_;
            let _e39 = t;
            xc = ((((-0.2661239f * _e32) - (0.234358f * _e35)) + (0.8776956f * _e39)) + 0.17991f);
        }
    } else {
        {
            let _e46 = t3_;
            let _e49 = t2_;
            let _e53 = t;
            xc = ((((-3.025847f * _e46) + (2.1070378f * _e49)) + (0.2226347f * _e53)) + 0.24039f);
        }
    }
    let _e58 = xc;
    let _e59 = xc;
    xc2_ = (_e58 * _e59);
    let _e61 = xc;
    let _e62 = xc;
    let _e64 = xc;
    xc3_ = ((_e61 * _e62) * _e64);
    let _e66 = temperatureKelvin_1;
    if (_e66 < 2222f) {
        {
            let _e71 = xc3_;
            let _e74 = xc2_;
            let _e78 = xc;
            yc = ((((-1.1063814f * _e71) - (1.3481102f * _e74)) + (2.1855583f * _e78)) - 0.20219684f);
        }
    } else {
        let _e83 = temperatureKelvin_1;
        if (_e83 < 4000f) {
            {
                let _e88 = xc3_;
                let _e91 = xc2_;
                let _e95 = xc;
                yc = ((((-0.9549476f * _e88) - (1.3741859f * _e91)) + (2.09137f * _e95)) - 0.16748866f);
            }
        } else {
            {
                let _e101 = xc3_;
                let _e104 = xc2_;
                let _e108 = xc;
                yc = ((((3.081758f * _e101) - (5.873387f * _e104)) + (3.7511299f * _e108)) - 0.37001482f);
            }
        }
    }
    let _e113 = yc;
    if (_e113 <= 0f) {
        {
            (*colorValue) = vec3(1f);
            return;
        }
    }
    let _e118 = xc;
    let _e119 = yc;
    let _e123 = xc;
    let _e125 = yc;
    let _e127 = yc;
    XYZ = vec3<f32>((_e118 / _e119), 1f, (((1f - _e123) - _e125) / _e127));
    let _e131 = XYZ;
    let _e132 = mx_matrix_mul_4(XYZ_to_RGB, _e131);
    (*colorValue) = _e132;
    let _e133 = (*colorValue);
    (*colorValue) = max(_e133, vec3(0f));
    return;
}