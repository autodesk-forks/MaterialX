// Main function: mx_fractal2d_vector4
// Included from lib/mx_noise.glsl (WGSL)
fn mx_select(b: bool, t: f32, f: f32) -> f32 {
    var b_1: bool;
    var t_1: f32;
    var f_1: f32;
    var local: f32;

    b_1 = b;
    t_1 = t;
    f_1 = f;
    let _e7 = b_1;
    if _e7 {
        let _e8 = t_1;
        local = _e8;
    } else {
        let _e9 = f_1;
        local = _e9;
    }
    let _e11 = local;
    return _e11;
}

fn mx_negate_if(val: f32, b_2: bool) -> f32 {
    var val_1: f32;
    var b_3: bool;
    var local_1: f32;

    val_1 = val;
    b_3 = b_2;
    let _e5 = b_3;
    if _e5 {
        let _e6 = val_1;
        local_1 = -(_e6);
    } else {
        let _e8 = val_1;
        local_1 = _e8;
    }
    let _e10 = local_1;
    return _e10;
}

fn mx_floor(x: f32) -> i32 {
    var x_1: f32;

    x_1 = x;
    let _e3 = x_1;
    return i32(floor(_e3));
}

fn mx_floorfrac(x_2: f32, i: ptr<function, i32>) -> f32 {
    var x_3: f32;

    x_3 = x_2;
    let _e4 = x_3;
    let _e5 = mx_floor(_e4);
    (*i) = _e5;
    let _e6 = x_3;
    let _e7 = (*i);
    return (_e6 - f32(_e7));
}

fn mx_bilerp(v0_: f32, v1_: f32, v2_: f32, v3_: f32, s: f32, t_2: f32) -> f32 {
    var v0_1: f32;
    var v1_1: f32;
    var v2_1: f32;
    var v3_1: f32;
    var s_1: f32;
    var t_3: f32;
    var s1_: f32;

    v0_1 = v0_;
    v1_1 = v1_;
    v2_1 = v2_;
    v3_1 = v3_;
    s_1 = s;
    t_3 = t_2;
    let _e14 = s_1;
    s1_ = (1f - _e14);
    let _e18 = t_3;
    let _e20 = v0_1;
    let _e21 = s1_;
    let _e23 = v1_1;
    let _e24 = s_1;
    let _e28 = t_3;
    let _e29 = v2_1;
    let _e30 = s1_;
    let _e32 = v3_1;
    let _e33 = s_1;
    return (((1f - _e18) * ((_e20 * _e21) + (_e23 * _e24))) + (_e28 * ((_e29 * _e30) + (_e32 * _e33))));
}

fn mx_bilerp_1(v0_2: vec3<f32>, v1_2: vec3<f32>, v2_2: vec3<f32>, v3_2: vec3<f32>, s_2: f32, t_4: f32) -> vec3<f32> {
    var v0_3: vec3<f32>;
    var v1_3: vec3<f32>;
    var v2_3: vec3<f32>;
    var v3_3: vec3<f32>;
    var s_3: f32;
    var t_5: f32;
    var s1_1: f32;

    v0_3 = v0_2;
    v1_3 = v1_2;
    v2_3 = v2_2;
    v3_3 = v3_2;
    s_3 = s_2;
    t_5 = t_4;
    let _e14 = s_3;
    s1_1 = (1f - _e14);
    let _e18 = t_5;
    let _e20 = v0_3;
    let _e21 = s1_1;
    let _e23 = v1_3;
    let _e24 = s_3;
    let _e28 = t_5;
    let _e29 = v2_3;
    let _e30 = s1_1;
    let _e32 = v3_3;
    let _e33 = s_3;
    return (((1f - _e18) * ((_e20 * _e21) + (_e23 * _e24))) + (_e28 * ((_e29 * _e30) + (_e32 * _e33))));
}

fn mx_trilerp(v0_4: f32, v1_4: f32, v2_4: f32, v3_4: f32, v4_: f32, v5_: f32, v6_: f32, v7_: f32, s_4: f32, t_6: f32, r: f32) -> f32 {
    var v0_5: f32;
    var v1_5: f32;
    var v2_5: f32;
    var v3_5: f32;
    var v4_1: f32;
    var v5_1: f32;
    var v6_1: f32;
    var v7_1: f32;
    var s_5: f32;
    var t_7: f32;
    var r_1: f32;
    var s1_2: f32;
    var t1_: f32;
    var r1_: f32;

    v0_5 = v0_4;
    v1_5 = v1_4;
    v2_5 = v2_4;
    v3_5 = v3_4;
    v4_1 = v4_;
    v5_1 = v5_;
    v6_1 = v6_;
    v7_1 = v7_;
    s_5 = s_4;
    t_7 = t_6;
    r_1 = r;
    let _e24 = s_5;
    s1_2 = (1f - _e24);
    let _e28 = t_7;
    t1_ = (1f - _e28);
    let _e32 = r_1;
    r1_ = (1f - _e32);
    let _e35 = r1_;
    let _e36 = t1_;
    let _e37 = v0_5;
    let _e38 = s1_2;
    let _e40 = v1_5;
    let _e41 = s_5;
    let _e45 = t_7;
    let _e46 = v2_5;
    let _e47 = s1_2;
    let _e49 = v3_5;
    let _e50 = s_5;
    let _e56 = r_1;
    let _e57 = t1_;
    let _e58 = v4_1;
    let _e59 = s1_2;
    let _e61 = v5_1;
    let _e62 = s_5;
    let _e66 = t_7;
    let _e67 = v6_1;
    let _e68 = s1_2;
    let _e70 = v7_1;
    let _e71 = s_5;
    return ((_e35 * ((_e36 * ((_e37 * _e38) + (_e40 * _e41))) + (_e45 * ((_e46 * _e47) + (_e49 * _e50))))) + (_e56 * ((_e57 * ((_e58 * _e59) + (_e61 * _e62))) + (_e66 * ((_e67 * _e68) + (_e70 * _e71))))));
}

fn mx_trilerp_1(v0_6: vec3<f32>, v1_6: vec3<f32>, v2_6: vec3<f32>, v3_6: vec3<f32>, v4_2: vec3<f32>, v5_2: vec3<f32>, v6_2: vec3<f32>, v7_2: vec3<f32>, s_6: f32, t_8: f32, r_2: f32) -> vec3<f32> {
    var v0_7: vec3<f32>;
    var v1_7: vec3<f32>;
    var v2_7: vec3<f32>;
    var v3_7: vec3<f32>;
    var v4_3: vec3<f32>;
    var v5_3: vec3<f32>;
    var v6_3: vec3<f32>;
    var v7_3: vec3<f32>;
    var s_7: f32;
    var t_9: f32;
    var r_3: f32;
    var s1_3: f32;
    var t1_1: f32;
    var r1_1: f32;

    v0_7 = v0_6;
    v1_7 = v1_6;
    v2_7 = v2_6;
    v3_7 = v3_6;
    v4_3 = v4_2;
    v5_3 = v5_2;
    v6_3 = v6_2;
    v7_3 = v7_2;
    s_7 = s_6;
    t_9 = t_8;
    r_3 = r_2;
    let _e24 = s_7;
    s1_3 = (1f - _e24);
    let _e28 = t_9;
    t1_1 = (1f - _e28);
    let _e32 = r_3;
    r1_1 = (1f - _e32);
    let _e35 = r1_1;
    let _e36 = t1_1;
    let _e37 = v0_7;
    let _e38 = s1_3;
    let _e40 = v1_7;
    let _e41 = s_7;
    let _e45 = t_9;
    let _e46 = v2_7;
    let _e47 = s1_3;
    let _e49 = v3_7;
    let _e50 = s_7;
    let _e56 = r_3;
    let _e57 = t1_1;
    let _e58 = v4_3;
    let _e59 = s1_3;
    let _e61 = v5_3;
    let _e62 = s_7;
    let _e66 = t_9;
    let _e67 = v6_3;
    let _e68 = s1_3;
    let _e70 = v7_3;
    let _e71 = s_7;
    return ((_e35 * ((_e36 * ((_e37 * _e38) + (_e40 * _e41))) + (_e45 * ((_e46 * _e47) + (_e49 * _e50))))) + (_e56 * ((_e57 * ((_e58 * _e59) + (_e61 * _e62))) + (_e66 * ((_e67 * _e68) + (_e70 * _e71))))));
}

fn mx_gradient_float(hash: u32, x_4: f32, y: f32) -> f32 {
    var hash_1: u32;
    var x_5: f32;
    var y_1: f32;
    var h: u32;
    var u: f32;
    var v: f32;

    hash_1 = hash;
    x_5 = x_4;
    y_1 = y;
    let _e7 = hash_1;
    h = (_e7 & 7u);
    let _e11 = h;
    let _e14 = x_5;
    let _e15 = y_1;
    let _e16 = mx_select((_e11 < 4u), _e14, _e15);
    u = _e16;
    let _e19 = h;
    let _e22 = y_1;
    let _e23 = x_5;
    let _e24 = mx_select((_e19 < 4u), _e22, _e23);
    v = (2f * _e24);
    let _e27 = u;
    let _e28 = h;
    let _e32 = mx_negate_if(_e27, bool((_e28 & 1u)));
    let _e33 = v;
    let _e34 = h;
    let _e38 = mx_negate_if(_e33, bool((_e34 & 2u)));
    return (_e32 + _e38);
}

fn mx_gradient_float_1(hash_2: u32, x_6: f32, y_2: f32, z: f32) -> f32 {
    var hash_3: u32;
    var x_7: f32;
    var y_3: f32;
    var z_1: f32;
    var h_1: u32;
    var u_1: f32;
    var v_1: f32;

    hash_3 = hash_2;
    x_7 = x_6;
    y_3 = y_2;
    z_1 = z;
    let _e9 = hash_3;
    h_1 = (_e9 & 15u);
    let _e13 = h_1;
    let _e16 = x_7;
    let _e17 = y_3;
    let _e18 = mx_select((_e13 < 8u), _e16, _e17);
    u_1 = _e18;
    let _e20 = h_1;
    let _e23 = y_3;
    let _e24 = h_1;
    let _e27 = h_1;
    let _e31 = x_7;
    let _e32 = z_1;
    let _e33 = mx_select(((_e24 == 12u) || (_e27 == 14u)), _e31, _e32);
    let _e34 = mx_select((_e20 < 4u), _e23, _e33);
    v_1 = _e34;
    let _e36 = u_1;
    let _e37 = h_1;
    let _e41 = mx_negate_if(_e36, bool((_e37 & 1u)));
    let _e42 = v_1;
    let _e43 = h_1;
    let _e47 = mx_negate_if(_e42, bool((_e43 & 2u)));
    return (_e41 + _e47);
}

fn mx_gradient_vec3_(hash_4: vec3<u32>, x_8: f32, y_4: f32) -> vec3<f32> {
    var hash_5: vec3<u32>;
    var x_9: f32;
    var y_5: f32;

    hash_5 = hash_4;
    x_9 = x_8;
    y_5 = y_4;
    let _e7 = hash_5;
    let _e9 = x_9;
    let _e10 = y_5;
    let _e11 = mx_gradient_float(_e7.x, _e9, _e10);
    let _e12 = hash_5;
    let _e14 = x_9;
    let _e15 = y_5;
    let _e16 = mx_gradient_float(_e12.y, _e14, _e15);
    let _e17 = hash_5;
    let _e19 = x_9;
    let _e20 = y_5;
    let _e21 = mx_gradient_float(_e17.z, _e19, _e20);
    return vec3<f32>(_e11, _e16, _e21);
}

fn mx_gradient_vec3_1(hash_6: vec3<u32>, x_10: f32, y_6: f32, z_2: f32) -> vec3<f32> {
    var hash_7: vec3<u32>;
    var x_11: f32;
    var y_7: f32;
    var z_3: f32;

    hash_7 = hash_6;
    x_11 = x_10;
    y_7 = y_6;
    z_3 = z_2;
    let _e9 = hash_7;
    let _e11 = x_11;
    let _e12 = y_7;
    let _e13 = z_3;
    let _e14 = mx_gradient_float_1(_e9.x, _e11, _e12, _e13);
    let _e15 = hash_7;
    let _e17 = x_11;
    let _e18 = y_7;
    let _e19 = z_3;
    let _e20 = mx_gradient_float_1(_e15.y, _e17, _e18, _e19);
    let _e21 = hash_7;
    let _e23 = x_11;
    let _e24 = y_7;
    let _e25 = z_3;
    let _e26 = mx_gradient_float_1(_e21.z, _e23, _e24, _e25);
    return vec3<f32>(_e14, _e20, _e26);
}

fn mx_gradient_scale2d(v_2: f32) -> f32 {
    var v_3: f32;

    v_3 = v_2;
    let _e4 = v_3;
    return (0.6616f * _e4);
}

fn mx_gradient_scale3d(v_4: f32) -> f32 {
    var v_5: f32;

    v_5 = v_4;
    let _e4 = v_5;
    return (0.982f * _e4);
}

fn mx_gradient_scale2d_1(v_6: vec3<f32>) -> vec3<f32> {
    var v_7: vec3<f32>;

    v_7 = v_6;
    let _e4 = v_7;
    return (0.6616f * _e4);
}

fn mx_gradient_scale3d_1(v_8: vec3<f32>) -> vec3<f32> {
    var v_9: vec3<f32>;

    v_9 = v_8;
    let _e4 = v_9;
    return (0.982f * _e4);
}

fn mx_rotl32_(x_12: u32, k: i32) -> u32 {
    var x_13: u32;
    var k_1: i32;

    x_13 = x_12;
    k_1 = k;
    let _e5 = x_13;
    let _e6 = k_1;
    let _e9 = x_13;
    let _e11 = k_1;
    return ((_e5 << u32(_e6)) | (_e9 >> u32((32i - _e11))));
}

fn mx_bjmix(a: ptr<function, u32>, b_4: ptr<function, u32>, c: ptr<function, u32>) {
    let _e4 = (*a);
    let _e5 = (*c);
    (*a) = (_e4 - _e5);
    let _e7 = (*a);
    let _e8 = (*c);
    let _e10 = mx_rotl32_(_e8, 4i);
    (*a) = (_e7 ^ _e10);
    let _e12 = (*c);
    let _e13 = (*b_4);
    (*c) = (_e12 + _e13);
    let _e15 = (*b_4);
    let _e16 = (*a);
    (*b_4) = (_e15 - _e16);
    let _e18 = (*b_4);
    let _e19 = (*a);
    let _e21 = mx_rotl32_(_e19, 6i);
    (*b_4) = (_e18 ^ _e21);
    let _e23 = (*a);
    let _e24 = (*c);
    (*a) = (_e23 + _e24);
    let _e26 = (*c);
    let _e27 = (*b_4);
    (*c) = (_e26 - _e27);
    let _e29 = (*c);
    let _e30 = (*b_4);
    let _e32 = mx_rotl32_(_e30, 8i);
    (*c) = (_e29 ^ _e32);
    let _e34 = (*b_4);
    let _e35 = (*a);
    (*b_4) = (_e34 + _e35);
    let _e37 = (*a);
    let _e38 = (*c);
    (*a) = (_e37 - _e38);
    let _e40 = (*a);
    let _e41 = (*c);
    let _e43 = mx_rotl32_(_e41, 16i);
    (*a) = (_e40 ^ _e43);
    let _e45 = (*c);
    let _e46 = (*b_4);
    (*c) = (_e45 + _e46);
    let _e48 = (*b_4);
    let _e49 = (*a);
    (*b_4) = (_e48 - _e49);
    let _e51 = (*b_4);
    let _e52 = (*a);
    let _e54 = mx_rotl32_(_e52, 19i);
    (*b_4) = (_e51 ^ _e54);
    let _e56 = (*a);
    let _e57 = (*c);
    (*a) = (_e56 + _e57);
    let _e59 = (*c);
    let _e60 = (*b_4);
    (*c) = (_e59 - _e60);
    let _e62 = (*c);
    let _e63 = (*b_4);
    let _e65 = mx_rotl32_(_e63, 4i);
    (*c) = (_e62 ^ _e65);
    let _e67 = (*b_4);
    let _e68 = (*a);
    (*b_4) = (_e67 + _e68);
    return;
}

fn mx_bjfinal(a_1: u32, b_5: u32, c_1: u32) -> u32 {
    var a_2: u32;
    var b_6: u32;
    var c_2: u32;

    a_2 = a_1;
    b_6 = b_5;
    c_2 = c_1;
    let _e7 = c_2;
    let _e8 = b_6;
    c_2 = (_e7 ^ _e8);
    let _e10 = c_2;
    let _e11 = b_6;
    let _e13 = mx_rotl32_(_e11, 14i);
    c_2 = (_e10 - _e13);
    let _e15 = a_2;
    let _e16 = c_2;
    a_2 = (_e15 ^ _e16);
    let _e18 = a_2;
    let _e19 = c_2;
    let _e21 = mx_rotl32_(_e19, 11i);
    a_2 = (_e18 - _e21);
    let _e23 = b_6;
    let _e24 = a_2;
    b_6 = (_e23 ^ _e24);
    let _e26 = b_6;
    let _e27 = a_2;
    let _e29 = mx_rotl32_(_e27, 25i);
    b_6 = (_e26 - _e29);
    let _e31 = c_2;
    let _e32 = b_6;
    c_2 = (_e31 ^ _e32);
    let _e34 = c_2;
    let _e35 = b_6;
    let _e37 = mx_rotl32_(_e35, 16i);
    c_2 = (_e34 - _e37);
    let _e39 = a_2;
    let _e40 = c_2;
    a_2 = (_e39 ^ _e40);
    let _e42 = a_2;
    let _e43 = c_2;
    let _e45 = mx_rotl32_(_e43, 4i);
    a_2 = (_e42 - _e45);
    let _e47 = b_6;
    let _e48 = a_2;
    b_6 = (_e47 ^ _e48);
    let _e50 = b_6;
    let _e51 = a_2;
    let _e53 = mx_rotl32_(_e51, 14i);
    b_6 = (_e50 - _e53);
    let _e55 = c_2;
    let _e56 = b_6;
    c_2 = (_e55 ^ _e56);
    let _e58 = c_2;
    let _e59 = b_6;
    let _e61 = mx_rotl32_(_e59, 24i);
    c_2 = (_e58 - _e61);
    let _e63 = c_2;
    return _e63;
}

fn mx_bits_to_01_(bits: u32) -> f32 {
    var bits_1: u32;

    bits_1 = bits;
    let _e3 = bits_1;
    return (f32(_e3) / 4294967300f);
}

fn mx_fade(t_10: f32) -> f32 {
    var t_11: f32;

    t_11 = t_10;
    let _e3 = t_11;
    let _e4 = t_11;
    let _e6 = t_11;
    let _e8 = t_11;
    let _e9 = t_11;
    return (((_e3 * _e4) * _e6) * ((_e8 * ((_e9 * 6f) - 15f)) + 10f));
}

fn mx_hash_int(x_14: i32) -> u32 {
    var x_15: i32;
    var len: u32 = 1u;
    var seed: u32;

    x_15 = x_14;
    let _e7 = len;
    seed = ((3735928559u + (_e7 << 2u)) + 13u);
    let _e14 = seed;
    let _e15 = x_15;
    let _e18 = seed;
    let _e19 = seed;
    let _e20 = mx_bjfinal((_e14 + u32(_e15)), _e18, _e19);
    return _e20;
}

fn mx_hash_int_1(x_16: i32, y_8: i32) -> u32 {
    var x_17: i32;
    var y_9: i32;
    var len_1: u32 = 2u;
    var a_3: u32;
    var b_7: u32;
    var c_3: u32;

    x_17 = x_16;
    y_9 = y_8;
    let _e12 = len_1;
    let _e17 = ((3735928559u + (_e12 << 2u)) + 13u);
    c_3 = _e17;
    b_7 = _e17;
    a_3 = _e17;
    let _e18 = a_3;
    let _e19 = x_17;
    a_3 = (_e18 + u32(_e19));
    let _e22 = b_7;
    let _e23 = y_9;
    b_7 = (_e22 + u32(_e23));
    let _e26 = a_3;
    let _e27 = b_7;
    let _e28 = c_3;
    let _e29 = mx_bjfinal(_e26, _e27, _e28);
    return _e29;
}

fn mx_hash_int_2(x_18: i32, y_10: i32, z_4: i32) -> u32 {
    var x_19: i32;
    var y_11: i32;
    var z_5: i32;
    var len_2: u32 = 3u;
    var a_4: u32;
    var b_8: u32;
    var c_4: u32;

    x_19 = x_18;
    y_11 = y_10;
    z_5 = z_4;
    let _e14 = len_2;
    let _e19 = ((3735928559u + (_e14 << 2u)) + 13u);
    c_4 = _e19;
    b_8 = _e19;
    a_4 = _e19;
    let _e20 = a_4;
    let _e21 = x_19;
    a_4 = (_e20 + u32(_e21));
    let _e24 = b_8;
    let _e25 = y_11;
    b_8 = (_e24 + u32(_e25));
    let _e28 = c_4;
    let _e29 = z_5;
    c_4 = (_e28 + u32(_e29));
    let _e32 = a_4;
    let _e33 = b_8;
    let _e34 = c_4;
    let _e35 = mx_bjfinal(_e32, _e33, _e34);
    return _e35;
}

fn mx_hash_int_3(x_20: i32, y_12: i32, z_6: i32, xx: i32) -> u32 {
    var x_21: i32;
    var y_13: i32;
    var z_7: i32;
    var xx_1: i32;
    var len_3: u32 = 4u;
    var a_5: u32;
    var b_9: u32;
    var c_5: u32;

    x_21 = x_20;
    y_13 = y_12;
    z_7 = z_6;
    xx_1 = xx;
    let _e16 = len_3;
    let _e21 = ((3735928559u + (_e16 << 2u)) + 13u);
    c_5 = _e21;
    b_9 = _e21;
    a_5 = _e21;
    let _e22 = a_5;
    let _e23 = x_21;
    a_5 = (_e22 + u32(_e23));
    let _e26 = b_9;
    let _e27 = y_13;
    b_9 = (_e26 + u32(_e27));
    let _e30 = c_5;
    let _e31 = z_7;
    c_5 = (_e30 + u32(_e31));
    mx_bjmix((&a_5), (&b_9), (&c_5));
    let _e40 = a_5;
    let _e41 = xx_1;
    a_5 = (_e40 + u32(_e41));
    let _e44 = a_5;
    let _e45 = b_9;
    let _e46 = c_5;
    let _e47 = mx_bjfinal(_e44, _e45, _e46);
    return _e47;
}

fn mx_hash_int_4(x_22: i32, y_14: i32, z_8: i32, xx_2: i32, yy: i32) -> u32 {
    var x_23: i32;
    var y_15: i32;
    var z_9: i32;
    var xx_3: i32;
    var yy_1: i32;
    var len_4: u32 = 5u;
    var a_6: u32;
    var b_10: u32;
    var c_6: u32;

    x_23 = x_22;
    y_15 = y_14;
    z_9 = z_8;
    xx_3 = xx_2;
    yy_1 = yy;
    let _e18 = len_4;
    let _e23 = ((3735928559u + (_e18 << 2u)) + 13u);
    c_6 = _e23;
    b_10 = _e23;
    a_6 = _e23;
    let _e24 = a_6;
    let _e25 = x_23;
    a_6 = (_e24 + u32(_e25));
    let _e28 = b_10;
    let _e29 = y_15;
    b_10 = (_e28 + u32(_e29));
    let _e32 = c_6;
    let _e33 = z_9;
    c_6 = (_e32 + u32(_e33));
    mx_bjmix((&a_6), (&b_10), (&c_6));
    let _e42 = a_6;
    let _e43 = xx_3;
    a_6 = (_e42 + u32(_e43));
    let _e46 = b_10;
    let _e47 = yy_1;
    b_10 = (_e46 + u32(_e47));
    let _e50 = a_6;
    let _e51 = b_10;
    let _e52 = c_6;
    let _e53 = mx_bjfinal(_e50, _e51, _e52);
    return _e53;
}

fn mx_hash_vec3_(x_24: i32, y_16: i32) -> vec3<u32> {
    var x_25: i32;
    var y_17: i32;
    var h_2: u32;
    var result: vec3<u32>;

    x_25 = x_24;
    y_17 = y_16;
    let _e5 = x_25;
    let _e6 = y_17;
    let _e7 = mx_hash_int_1(_e5, _e6);
    h_2 = _e7;
    let _e11 = h_2;
    result.x = (_e11 & 255u);
    let _e15 = h_2;
    result.y = ((_e15 >> 8u) & 255u);
    let _e22 = h_2;
    result.z = ((_e22 >> 16u) & 255u);
    let _e28 = result;
    return _e28;
}

fn mx_hash_vec3_1(x_26: i32, y_18: i32, z_10: i32) -> vec3<u32> {
    var x_27: i32;
    var y_19: i32;
    var z_11: i32;
    var h_3: u32;
    var result_1: vec3<u32>;

    x_27 = x_26;
    y_19 = y_18;
    z_11 = z_10;
    let _e7 = x_27;
    let _e8 = y_19;
    let _e9 = z_11;
    let _e10 = mx_hash_int_2(_e7, _e8, _e9);
    h_3 = _e10;
    let _e14 = h_3;
    result_1.x = (_e14 & 255u);
    let _e18 = h_3;
    result_1.y = ((_e18 >> 8u) & 255u);
    let _e25 = h_3;
    result_1.z = ((_e25 >> 16u) & 255u);
    let _e31 = result_1;
    return _e31;
}

fn mx_perlin_noise_float(p: vec2<f32>) -> f32 {
    var p_1: vec2<f32>;
    var X: i32;
    var Y: i32;
    var fx: f32;
    var fy: f32;
    var u_2: f32;
    var v_10: f32;
    var result_2: f32;

    p_1 = p;
    let _e5 = p_1;
    let _e9 = mx_floorfrac(_e5.x, (&X));
    fx = _e9;
    let _e11 = p_1;
    let _e15 = mx_floorfrac(_e11.y, (&Y));
    fy = _e15;
    let _e17 = fx;
    let _e18 = mx_fade(_e17);
    u_2 = _e18;
    let _e20 = fy;
    let _e21 = mx_fade(_e20);
    v_10 = _e21;
    let _e23 = X;
    let _e24 = Y;
    let _e25 = mx_hash_int_1(_e23, _e24);
    let _e26 = fx;
    let _e27 = fy;
    let _e28 = mx_gradient_float(_e25, _e26, _e27);
    let _e29 = X;
    let _e32 = Y;
    let _e33 = mx_hash_int_1((_e29 + 1i), _e32);
    let _e34 = fx;
    let _e37 = fy;
    let _e38 = mx_gradient_float(_e33, (_e34 - 1f), _e37);
    let _e39 = X;
    let _e40 = Y;
    let _e43 = mx_hash_int_1(_e39, (_e40 + 1i));
    let _e44 = fx;
    let _e45 = fy;
    let _e48 = mx_gradient_float(_e43, _e44, (_e45 - 1f));
    let _e49 = X;
    let _e52 = Y;
    let _e55 = mx_hash_int_1((_e49 + 1i), (_e52 + 1i));
    let _e56 = fx;
    let _e59 = fy;
    let _e62 = mx_gradient_float(_e55, (_e56 - 1f), (_e59 - 1f));
    let _e63 = u_2;
    let _e64 = v_10;
    let _e65 = mx_bilerp(_e28, _e38, _e48, _e62, _e63, _e64);
    result_2 = _e65;
    let _e67 = result_2;
    let _e68 = mx_gradient_scale2d(_e67);
    return _e68;
}

fn mx_perlin_noise_float_1(p_2: vec3<f32>) -> f32 {
    var p_3: vec3<f32>;
    var X_1: i32;
    var Y_1: i32;
    var Z: i32;
    var fx_1: f32;
    var fy_1: f32;
    var fz: f32;
    var u_3: f32;
    var v_11: f32;
    var w: f32;
    var result_3: f32;

    p_3 = p_2;
    let _e6 = p_3;
    let _e10 = mx_floorfrac(_e6.x, (&X_1));
    fx_1 = _e10;
    let _e12 = p_3;
    let _e16 = mx_floorfrac(_e12.y, (&Y_1));
    fy_1 = _e16;
    let _e18 = p_3;
    let _e22 = mx_floorfrac(_e18.z, (&Z));
    fz = _e22;
    let _e24 = fx_1;
    let _e25 = mx_fade(_e24);
    u_3 = _e25;
    let _e27 = fy_1;
    let _e28 = mx_fade(_e27);
    v_11 = _e28;
    let _e30 = fz;
    let _e31 = mx_fade(_e30);
    w = _e31;
    let _e33 = X_1;
    let _e34 = Y_1;
    let _e35 = Z;
    let _e36 = mx_hash_int_2(_e33, _e34, _e35);
    let _e37 = fx_1;
    let _e38 = fy_1;
    let _e39 = fz;
    let _e40 = mx_gradient_float_1(_e36, _e37, _e38, _e39);
    let _e41 = X_1;
    let _e44 = Y_1;
    let _e45 = Z;
    let _e46 = mx_hash_int_2((_e41 + 1i), _e44, _e45);
    let _e47 = fx_1;
    let _e50 = fy_1;
    let _e51 = fz;
    let _e52 = mx_gradient_float_1(_e46, (_e47 - 1f), _e50, _e51);
    let _e53 = X_1;
    let _e54 = Y_1;
    let _e57 = Z;
    let _e58 = mx_hash_int_2(_e53, (_e54 + 1i), _e57);
    let _e59 = fx_1;
    let _e60 = fy_1;
    let _e63 = fz;
    let _e64 = mx_gradient_float_1(_e58, _e59, (_e60 - 1f), _e63);
    let _e65 = X_1;
    let _e68 = Y_1;
    let _e71 = Z;
    let _e72 = mx_hash_int_2((_e65 + 1i), (_e68 + 1i), _e71);
    let _e73 = fx_1;
    let _e76 = fy_1;
    let _e79 = fz;
    let _e80 = mx_gradient_float_1(_e72, (_e73 - 1f), (_e76 - 1f), _e79);
    let _e81 = X_1;
    let _e82 = Y_1;
    let _e83 = Z;
    let _e86 = mx_hash_int_2(_e81, _e82, (_e83 + 1i));
    let _e87 = fx_1;
    let _e88 = fy_1;
    let _e89 = fz;
    let _e92 = mx_gradient_float_1(_e86, _e87, _e88, (_e89 - 1f));
    let _e93 = X_1;
    let _e96 = Y_1;
    let _e97 = Z;
    let _e100 = mx_hash_int_2((_e93 + 1i), _e96, (_e97 + 1i));
    let _e101 = fx_1;
    let _e104 = fy_1;
    let _e105 = fz;
    let _e108 = mx_gradient_float_1(_e100, (_e101 - 1f), _e104, (_e105 - 1f));
    let _e109 = X_1;
    let _e110 = Y_1;
    let _e113 = Z;
    let _e116 = mx_hash_int_2(_e109, (_e110 + 1i), (_e113 + 1i));
    let _e117 = fx_1;
    let _e118 = fy_1;
    let _e121 = fz;
    let _e124 = mx_gradient_float_1(_e116, _e117, (_e118 - 1f), (_e121 - 1f));
    let _e125 = X_1;
    let _e128 = Y_1;
    let _e131 = Z;
    let _e134 = mx_hash_int_2((_e125 + 1i), (_e128 + 1i), (_e131 + 1i));
    let _e135 = fx_1;
    let _e138 = fy_1;
    let _e141 = fz;
    let _e144 = mx_gradient_float_1(_e134, (_e135 - 1f), (_e138 - 1f), (_e141 - 1f));
    let _e145 = u_3;
    let _e146 = v_11;
    let _e147 = w;
    let _e148 = mx_trilerp(_e40, _e52, _e64, _e80, _e92, _e108, _e124, _e144, _e145, _e146, _e147);
    result_3 = _e148;
    let _e150 = result_3;
    let _e151 = mx_gradient_scale3d(_e150);
    return _e151;
}

fn mx_perlin_noise_vec3_(p_4: vec2<f32>) -> vec3<f32> {
    var p_5: vec2<f32>;
    var X_2: i32;
    var Y_2: i32;
    var fx_2: f32;
    var fy_2: f32;
    var u_4: f32;
    var v_12: f32;
    var result_4: vec3<f32>;

    p_5 = p_4;
    let _e5 = p_5;
    let _e9 = mx_floorfrac(_e5.x, (&X_2));
    fx_2 = _e9;
    let _e11 = p_5;
    let _e15 = mx_floorfrac(_e11.y, (&Y_2));
    fy_2 = _e15;
    let _e17 = fx_2;
    let _e18 = mx_fade(_e17);
    u_4 = _e18;
    let _e20 = fy_2;
    let _e21 = mx_fade(_e20);
    v_12 = _e21;
    let _e23 = X_2;
    let _e24 = Y_2;
    let _e25 = mx_hash_vec3_(_e23, _e24);
    let _e26 = fx_2;
    let _e27 = fy_2;
    let _e28 = mx_gradient_vec3_(_e25, _e26, _e27);
    let _e29 = X_2;
    let _e32 = Y_2;
    let _e33 = mx_hash_vec3_((_e29 + 1i), _e32);
    let _e34 = fx_2;
    let _e37 = fy_2;
    let _e38 = mx_gradient_vec3_(_e33, (_e34 - 1f), _e37);
    let _e39 = X_2;
    let _e40 = Y_2;
    let _e43 = mx_hash_vec3_(_e39, (_e40 + 1i));
    let _e44 = fx_2;
    let _e45 = fy_2;
    let _e48 = mx_gradient_vec3_(_e43, _e44, (_e45 - 1f));
    let _e49 = X_2;
    let _e52 = Y_2;
    let _e55 = mx_hash_vec3_((_e49 + 1i), (_e52 + 1i));
    let _e56 = fx_2;
    let _e59 = fy_2;
    let _e62 = mx_gradient_vec3_(_e55, (_e56 - 1f), (_e59 - 1f));
    let _e63 = u_4;
    let _e64 = v_12;
    let _e65 = mx_bilerp_1(_e28, _e38, _e48, _e62, _e63, _e64);
    result_4 = _e65;
    let _e67 = result_4;
    let _e68 = mx_gradient_scale2d_1(_e67);
    return _e68;
}

fn mx_perlin_noise_vec3_1(p_6: vec3<f32>) -> vec3<f32> {
    var p_7: vec3<f32>;
    var X_3: i32;
    var Y_3: i32;
    var Z_1: i32;
    var fx_3: f32;
    var fy_3: f32;
    var fz_1: f32;
    var u_5: f32;
    var v_13: f32;
    var w_1: f32;
    var result_5: vec3<f32>;

    p_7 = p_6;
    let _e6 = p_7;
    let _e10 = mx_floorfrac(_e6.x, (&X_3));
    fx_3 = _e10;
    let _e12 = p_7;
    let _e16 = mx_floorfrac(_e12.y, (&Y_3));
    fy_3 = _e16;
    let _e18 = p_7;
    let _e22 = mx_floorfrac(_e18.z, (&Z_1));
    fz_1 = _e22;
    let _e24 = fx_3;
    let _e25 = mx_fade(_e24);
    u_5 = _e25;
    let _e27 = fy_3;
    let _e28 = mx_fade(_e27);
    v_13 = _e28;
    let _e30 = fz_1;
    let _e31 = mx_fade(_e30);
    w_1 = _e31;
    let _e33 = X_3;
    let _e34 = Y_3;
    let _e35 = Z_1;
    let _e36 = mx_hash_vec3_1(_e33, _e34, _e35);
    let _e37 = fx_3;
    let _e38 = fy_3;
    let _e39 = fz_1;
    let _e40 = mx_gradient_vec3_1(_e36, _e37, _e38, _e39);
    let _e41 = X_3;
    let _e44 = Y_3;
    let _e45 = Z_1;
    let _e46 = mx_hash_vec3_1((_e41 + 1i), _e44, _e45);
    let _e47 = fx_3;
    let _e50 = fy_3;
    let _e51 = fz_1;
    let _e52 = mx_gradient_vec3_1(_e46, (_e47 - 1f), _e50, _e51);
    let _e53 = X_3;
    let _e54 = Y_3;
    let _e57 = Z_1;
    let _e58 = mx_hash_vec3_1(_e53, (_e54 + 1i), _e57);
    let _e59 = fx_3;
    let _e60 = fy_3;
    let _e63 = fz_1;
    let _e64 = mx_gradient_vec3_1(_e58, _e59, (_e60 - 1f), _e63);
    let _e65 = X_3;
    let _e68 = Y_3;
    let _e71 = Z_1;
    let _e72 = mx_hash_vec3_1((_e65 + 1i), (_e68 + 1i), _e71);
    let _e73 = fx_3;
    let _e76 = fy_3;
    let _e79 = fz_1;
    let _e80 = mx_gradient_vec3_1(_e72, (_e73 - 1f), (_e76 - 1f), _e79);
    let _e81 = X_3;
    let _e82 = Y_3;
    let _e83 = Z_1;
    let _e86 = mx_hash_vec3_1(_e81, _e82, (_e83 + 1i));
    let _e87 = fx_3;
    let _e88 = fy_3;
    let _e89 = fz_1;
    let _e92 = mx_gradient_vec3_1(_e86, _e87, _e88, (_e89 - 1f));
    let _e93 = X_3;
    let _e96 = Y_3;
    let _e97 = Z_1;
    let _e100 = mx_hash_vec3_1((_e93 + 1i), _e96, (_e97 + 1i));
    let _e101 = fx_3;
    let _e104 = fy_3;
    let _e105 = fz_1;
    let _e108 = mx_gradient_vec3_1(_e100, (_e101 - 1f), _e104, (_e105 - 1f));
    let _e109 = X_3;
    let _e110 = Y_3;
    let _e113 = Z_1;
    let _e116 = mx_hash_vec3_1(_e109, (_e110 + 1i), (_e113 + 1i));
    let _e117 = fx_3;
    let _e118 = fy_3;
    let _e121 = fz_1;
    let _e124 = mx_gradient_vec3_1(_e116, _e117, (_e118 - 1f), (_e121 - 1f));
    let _e125 = X_3;
    let _e128 = Y_3;
    let _e131 = Z_1;
    let _e134 = mx_hash_vec3_1((_e125 + 1i), (_e128 + 1i), (_e131 + 1i));
    let _e135 = fx_3;
    let _e138 = fy_3;
    let _e141 = fz_1;
    let _e144 = mx_gradient_vec3_1(_e134, (_e135 - 1f), (_e138 - 1f), (_e141 - 1f));
    let _e145 = u_5;
    let _e146 = v_13;
    let _e147 = w_1;
    let _e148 = mx_trilerp_1(_e40, _e52, _e64, _e80, _e92, _e108, _e124, _e144, _e145, _e146, _e147);
    result_5 = _e148;
    let _e150 = result_5;
    let _e151 = mx_gradient_scale3d_1(_e150);
    return _e151;
}

fn mx_cell_noise_float(p_8: f32) -> f32 {
    var p_9: f32;
    var ix: i32;

    p_9 = p_8;
    let _e3 = p_9;
    let _e4 = mx_floor(_e3);
    ix = _e4;
    let _e6 = ix;
    let _e7 = mx_hash_int(_e6);
    let _e8 = mx_bits_to_01_(_e7);
    return _e8;
}

fn mx_cell_noise_float_1(p_10: vec2<f32>) -> f32 {
    var p_11: vec2<f32>;
    var ix_1: i32;
    var iy: i32;

    p_11 = p_10;
    let _e3 = p_11;
    let _e5 = mx_floor(_e3.x);
    ix_1 = _e5;
    let _e7 = p_11;
    let _e9 = mx_floor(_e7.y);
    iy = _e9;
    let _e11 = ix_1;
    let _e12 = iy;
    let _e13 = mx_hash_int_1(_e11, _e12);
    let _e14 = mx_bits_to_01_(_e13);
    return _e14;
}

fn mx_cell_noise_float_2(p_12: vec3<f32>) -> f32 {
    var p_13: vec3<f32>;
    var ix_2: i32;
    var iy_1: i32;
    var iz: i32;

    p_13 = p_12;
    let _e3 = p_13;
    let _e5 = mx_floor(_e3.x);
    ix_2 = _e5;
    let _e7 = p_13;
    let _e9 = mx_floor(_e7.y);
    iy_1 = _e9;
    let _e11 = p_13;
    let _e13 = mx_floor(_e11.z);
    iz = _e13;
    let _e15 = ix_2;
    let _e16 = iy_1;
    let _e17 = iz;
    let _e18 = mx_hash_int_2(_e15, _e16, _e17);
    let _e19 = mx_bits_to_01_(_e18);
    return _e19;
}

fn mx_cell_noise_float_3(p_14: vec4<f32>) -> f32 {
    var p_15: vec4<f32>;
    var ix_3: i32;
    var iy_2: i32;
    var iz_1: i32;
    var iw: i32;

    p_15 = p_14;
    let _e3 = p_15;
    let _e5 = mx_floor(_e3.x);
    ix_3 = _e5;
    let _e7 = p_15;
    let _e9 = mx_floor(_e7.y);
    iy_2 = _e9;
    let _e11 = p_15;
    let _e13 = mx_floor(_e11.z);
    iz_1 = _e13;
    let _e15 = p_15;
    let _e17 = mx_floor(_e15.w);
    iw = _e17;
    let _e19 = ix_3;
    let _e20 = iy_2;
    let _e21 = iz_1;
    let _e22 = iw;
    let _e23 = mx_hash_int_3(_e19, _e20, _e21, _e22);
    let _e24 = mx_bits_to_01_(_e23);
    return _e24;
}

fn mx_cell_noise_vec3_(p_16: f32) -> vec3<f32> {
    var p_17: f32;
    var ix_4: i32;

    p_17 = p_16;
    let _e3 = p_17;
    let _e4 = mx_floor(_e3);
    ix_4 = _e4;
    let _e6 = ix_4;
    let _e8 = mx_hash_int_1(_e6, 0i);
    let _e9 = mx_bits_to_01_(_e8);
    let _e10 = ix_4;
    let _e12 = mx_hash_int_1(_e10, 1i);
    let _e13 = mx_bits_to_01_(_e12);
    let _e14 = ix_4;
    let _e16 = mx_hash_int_1(_e14, 2i);
    let _e17 = mx_bits_to_01_(_e16);
    return vec3<f32>(_e9, _e13, _e17);
}

fn mx_cell_noise_vec3_1(p_18: vec2<f32>) -> vec3<f32> {
    var p_19: vec2<f32>;
    var ix_5: i32;
    var iy_3: i32;

    p_19 = p_18;
    let _e3 = p_19;
    let _e5 = mx_floor(_e3.x);
    ix_5 = _e5;
    let _e7 = p_19;
    let _e9 = mx_floor(_e7.y);
    iy_3 = _e9;
    let _e11 = ix_5;
    let _e12 = iy_3;
    let _e14 = mx_hash_int_2(_e11, _e12, 0i);
    let _e15 = mx_bits_to_01_(_e14);
    let _e16 = ix_5;
    let _e17 = iy_3;
    let _e19 = mx_hash_int_2(_e16, _e17, 1i);
    let _e20 = mx_bits_to_01_(_e19);
    let _e21 = ix_5;
    let _e22 = iy_3;
    let _e24 = mx_hash_int_2(_e21, _e22, 2i);
    let _e25 = mx_bits_to_01_(_e24);
    return vec3<f32>(_e15, _e20, _e25);
}

fn mx_cell_noise_vec3_2(p_20: vec3<f32>) -> vec3<f32> {
    var p_21: vec3<f32>;
    var ix_6: i32;
    var iy_4: i32;
    var iz_2: i32;

    p_21 = p_20;
    let _e3 = p_21;
    let _e5 = mx_floor(_e3.x);
    ix_6 = _e5;
    let _e7 = p_21;
    let _e9 = mx_floor(_e7.y);
    iy_4 = _e9;
    let _e11 = p_21;
    let _e13 = mx_floor(_e11.z);
    iz_2 = _e13;
    let _e15 = ix_6;
    let _e16 = iy_4;
    let _e17 = iz_2;
    let _e19 = mx_hash_int_3(_e15, _e16, _e17, 0i);
    let _e20 = mx_bits_to_01_(_e19);
    let _e21 = ix_6;
    let _e22 = iy_4;
    let _e23 = iz_2;
    let _e25 = mx_hash_int_3(_e21, _e22, _e23, 1i);
    let _e26 = mx_bits_to_01_(_e25);
    let _e27 = ix_6;
    let _e28 = iy_4;
    let _e29 = iz_2;
    let _e31 = mx_hash_int_3(_e27, _e28, _e29, 2i);
    let _e32 = mx_bits_to_01_(_e31);
    return vec3<f32>(_e20, _e26, _e32);
}

fn mx_cell_noise_vec3_3(p_22: vec4<f32>) -> vec3<f32> {
    var p_23: vec4<f32>;
    var ix_7: i32;
    var iy_5: i32;
    var iz_3: i32;
    var iw_1: i32;

    p_23 = p_22;
    let _e3 = p_23;
    let _e5 = mx_floor(_e3.x);
    ix_7 = _e5;
    let _e7 = p_23;
    let _e9 = mx_floor(_e7.y);
    iy_5 = _e9;
    let _e11 = p_23;
    let _e13 = mx_floor(_e11.z);
    iz_3 = _e13;
    let _e15 = p_23;
    let _e17 = mx_floor(_e15.w);
    iw_1 = _e17;
    let _e19 = ix_7;
    let _e20 = iy_5;
    let _e21 = iz_3;
    let _e22 = iw_1;
    let _e24 = mx_hash_int_4(_e19, _e20, _e21, _e22, 0i);
    let _e25 = mx_bits_to_01_(_e24);
    let _e26 = ix_7;
    let _e27 = iy_5;
    let _e28 = iz_3;
    let _e29 = iw_1;
    let _e31 = mx_hash_int_4(_e26, _e27, _e28, _e29, 1i);
    let _e32 = mx_bits_to_01_(_e31);
    let _e33 = ix_7;
    let _e34 = iy_5;
    let _e35 = iz_3;
    let _e36 = iw_1;
    let _e38 = mx_hash_int_4(_e33, _e34, _e35, _e36, 2i);
    let _e39 = mx_bits_to_01_(_e38);
    return vec3<f32>(_e25, _e32, _e39);
}

fn mx_fractal2d_noise_float(p_24: vec2<f32>, octaves: i32, lacunarity: f32, diminish: f32) -> f32 {
    var p_25: vec2<f32>;
    var octaves_1: i32;
    var lacunarity_1: f32;
    var diminish_1: f32;
    var result_6: f32 = 0f;
    var amplitude: f32 = 1f;
    var i_1: i32 = 0i;

    p_25 = p_24;
    octaves_1 = octaves;
    lacunarity_1 = lacunarity;
    diminish_1 = diminish;
    loop {
        let _e15 = i_1;
        let _e16 = octaves_1;
        if !((_e15 < _e16)) {
            break;
        }
        {
            let _e22 = result_6;
            let _e23 = amplitude;
            let _e24 = p_25;
            let _e25 = mx_perlin_noise_float(_e24);
            result_6 = (_e22 + (_e23 * _e25));
            let _e28 = amplitude;
            let _e29 = diminish_1;
            amplitude = (_e28 * _e29);
            let _e31 = p_25;
            let _e32 = lacunarity_1;
            p_25 = (_e31 * _e32);
        }
        continuing {
            let _e19 = i_1;
            i_1 = (_e19 + 1i);
        }
    }
    let _e34 = result_6;
    return _e34;
}

fn mx_fractal2d_noise_vec3_(p_26: vec2<f32>, octaves_2: i32, lacunarity_2: f32, diminish_2: f32) -> vec3<f32> {
    var p_27: vec2<f32>;
    var octaves_3: i32;
    var lacunarity_3: f32;
    var diminish_3: f32;
    var result_7: vec3<f32> = vec3(0f);
    var amplitude_1: f32 = 1f;
    var i_2: i32 = 0i;

    p_27 = p_26;
    octaves_3 = octaves_2;
    lacunarity_3 = lacunarity_2;
    diminish_3 = diminish_2;
    loop {
        let _e16 = i_2;
        let _e17 = octaves_3;
        if !((_e16 < _e17)) {
            break;
        }
        {
            let _e23 = result_7;
            let _e24 = amplitude_1;
            let _e25 = p_27;
            let _e26 = mx_perlin_noise_vec3_(_e25);
            result_7 = (_e23 + (_e24 * _e26));
            let _e29 = amplitude_1;
            let _e30 = diminish_3;
            amplitude_1 = (_e29 * _e30);
            let _e32 = p_27;
            let _e33 = lacunarity_3;
            p_27 = (_e32 * _e33);
        }
        continuing {
            let _e20 = i_2;
            i_2 = (_e20 + 1i);
        }
    }
    let _e35 = result_7;
    return _e35;
}

fn mx_fractal2d_noise_vec2_(p_28: vec2<f32>, octaves_4: i32, lacunarity_4: f32, diminish_4: f32) -> vec2<f32> {
    var p_29: vec2<f32>;
    var octaves_5: i32;
    var lacunarity_5: f32;
    var diminish_5: f32;

    p_29 = p_28;
    octaves_5 = octaves_4;
    lacunarity_5 = lacunarity_4;
    diminish_5 = diminish_4;
    let _e9 = p_29;
    let _e10 = octaves_5;
    let _e11 = lacunarity_5;
    let _e12 = diminish_5;
    let _e13 = mx_fractal2d_noise_float(_e9, _e10, _e11, _e12);
    let _e14 = p_29;
    let _e21 = octaves_5;
    let _e22 = lacunarity_5;
    let _e23 = diminish_5;
    let _e24 = mx_fractal2d_noise_float((_e14 + vec2<f32>(19f, 193f)), _e21, _e22, _e23);
    return vec2<f32>(_e13, _e24);
}

fn mx_fractal2d_noise_vec4_(p_30: vec2<f32>, octaves_6: i32, lacunarity_6: f32, diminish_6: f32) -> vec4<f32> {
    var p_31: vec2<f32>;
    var octaves_7: i32;
    var lacunarity_7: f32;
    var diminish_7: f32;
    var c_7: vec3<f32>;
    var f_2: f32;

    p_31 = p_30;
    octaves_7 = octaves_6;
    lacunarity_7 = lacunarity_6;
    diminish_7 = diminish_6;
    let _e9 = p_31;
    let _e10 = octaves_7;
    let _e11 = lacunarity_7;
    let _e12 = diminish_7;
    let _e13 = mx_fractal2d_noise_vec3_(_e9, _e10, _e11, _e12);
    c_7 = _e13;
    let _e15 = p_31;
    let _e22 = octaves_7;
    let _e23 = lacunarity_7;
    let _e24 = diminish_7;
    let _e25 = mx_fractal2d_noise_float((_e15 + vec2<f32>(19f, 193f)), _e22, _e23, _e24);
    f_2 = _e25;
    let _e27 = c_7;
    let _e28 = f_2;
    return vec4<f32>(_e27.x, _e27.y, _e27.z, _e28);
}

fn mx_fractal3d_noise_float(p_32: vec3<f32>, octaves_8: i32, lacunarity_8: f32, diminish_8: f32) -> f32 {
    var p_33: vec3<f32>;
    var octaves_9: i32;
    var lacunarity_9: f32;
    var diminish_9: f32;
    var result_8: f32 = 0f;
    var amplitude_2: f32 = 1f;
    var i_3: i32 = 0i;

    p_33 = p_32;
    octaves_9 = octaves_8;
    lacunarity_9 = lacunarity_8;
    diminish_9 = diminish_8;
    loop {
        let _e15 = i_3;
        let _e16 = octaves_9;
        if !((_e15 < _e16)) {
            break;
        }
        {
            let _e22 = result_8;
            let _e23 = amplitude_2;
            let _e24 = p_33;
            let _e25 = mx_perlin_noise_float_1(_e24);
            result_8 = (_e22 + (_e23 * _e25));
            let _e28 = amplitude_2;
            let _e29 = diminish_9;
            amplitude_2 = (_e28 * _e29);
            let _e31 = p_33;
            let _e32 = lacunarity_9;
            p_33 = (_e31 * _e32);
        }
        continuing {
            let _e19 = i_3;
            i_3 = (_e19 + 1i);
        }
    }
    let _e34 = result_8;
    return _e34;
}

fn mx_fractal3d_noise_vec3_(p_34: vec3<f32>, octaves_10: i32, lacunarity_10: f32, diminish_10: f32) -> vec3<f32> {
    var p_35: vec3<f32>;
    var octaves_11: i32;
    var lacunarity_11: f32;
    var diminish_11: f32;
    var result_9: vec3<f32> = vec3(0f);
    var amplitude_3: f32 = 1f;
    var i_4: i32 = 0i;

    p_35 = p_34;
    octaves_11 = octaves_10;
    lacunarity_11 = lacunarity_10;
    diminish_11 = diminish_10;
    loop {
        let _e16 = i_4;
        let _e17 = octaves_11;
        if !((_e16 < _e17)) {
            break;
        }
        {
            let _e23 = result_9;
            let _e24 = amplitude_3;
            let _e25 = p_35;
            let _e26 = mx_perlin_noise_vec3_1(_e25);
            result_9 = (_e23 + (_e24 * _e26));
            let _e29 = amplitude_3;
            let _e30 = diminish_11;
            amplitude_3 = (_e29 * _e30);
            let _e32 = p_35;
            let _e33 = lacunarity_11;
            p_35 = (_e32 * _e33);
        }
        continuing {
            let _e20 = i_4;
            i_4 = (_e20 + 1i);
        }
    }
    let _e35 = result_9;
    return _e35;
}

fn mx_fractal3d_noise_vec2_(p_36: vec3<f32>, octaves_12: i32, lacunarity_12: f32, diminish_12: f32) -> vec2<f32> {
    var p_37: vec3<f32>;
    var octaves_13: i32;
    var lacunarity_13: f32;
    var diminish_13: f32;

    p_37 = p_36;
    octaves_13 = octaves_12;
    lacunarity_13 = lacunarity_12;
    diminish_13 = diminish_12;
    let _e9 = p_37;
    let _e10 = octaves_13;
    let _e11 = lacunarity_13;
    let _e12 = diminish_13;
    let _e13 = mx_fractal3d_noise_float(_e9, _e10, _e11, _e12);
    let _e14 = p_37;
    let _e23 = octaves_13;
    let _e24 = lacunarity_13;
    let _e25 = diminish_13;
    let _e26 = mx_fractal3d_noise_float((_e14 + vec3<f32>(19f, 193f, 17f)), _e23, _e24, _e25);
    return vec2<f32>(_e13, _e26);
}

fn mx_fractal3d_noise_vec4_(p_38: vec3<f32>, octaves_14: i32, lacunarity_14: f32, diminish_14: f32) -> vec4<f32> {
    var p_39: vec3<f32>;
    var octaves_15: i32;
    var lacunarity_15: f32;
    var diminish_15: f32;
    var c_8: vec3<f32>;
    var f_3: f32;

    p_39 = p_38;
    octaves_15 = octaves_14;
    lacunarity_15 = lacunarity_14;
    diminish_15 = diminish_14;
    let _e9 = p_39;
    let _e10 = octaves_15;
    let _e11 = lacunarity_15;
    let _e12 = diminish_15;
    let _e13 = mx_fractal3d_noise_vec3_(_e9, _e10, _e11, _e12);
    c_8 = _e13;
    let _e15 = p_39;
    let _e24 = octaves_15;
    let _e25 = lacunarity_15;
    let _e26 = diminish_15;
    let _e27 = mx_fractal3d_noise_float((_e15 + vec3<f32>(19f, 193f, 17f)), _e24, _e25, _e26);
    f_3 = _e27;
    let _e29 = c_8;
    let _e30 = f_3;
    return vec4<f32>(_e29.x, _e29.y, _e29.z, _e30);
}

fn mx_worley_cell_position(x_28: i32, y_20: i32, xoff: i32, yoff: i32, jitter: f32) -> vec2<f32> {
    var x_29: i32;
    var y_21: i32;
    var xoff_1: i32;
    var yoff_1: i32;
    var jitter_1: f32;
    var tmp: vec3<f32>;
    var off: vec2<f32>;

    x_29 = x_28;
    y_21 = y_20;
    xoff_1 = xoff;
    yoff_1 = yoff;
    jitter_1 = jitter;
    let _e11 = x_29;
    let _e12 = xoff_1;
    let _e14 = y_21;
    let _e15 = yoff_1;
    let _e20 = mx_cell_noise_vec3_1(vec2<f32>(f32((_e11 + _e12)), f32((_e14 + _e15))));
    tmp = _e20;
    let _e22 = tmp;
    let _e24 = tmp;
    off = vec2<f32>(_e22.x, _e24.y);
    let _e28 = off;
    off = (_e28 - vec2(0.5f));
    let _e32 = off;
    let _e33 = jitter_1;
    off = (_e32 * _e33);
    let _e35 = off;
    off = (_e35 + vec2(0.5f));
    let _e39 = x_29;
    let _e41 = y_21;
    let _e44 = off;
    return (vec2<f32>(f32(_e39), f32(_e41)) + _e44);
}

fn mx_worley_cell_position_1(x_30: i32, y_22: i32, z_12: i32, xoff_2: i32, yoff_2: i32, zoff: i32, jitter_2: f32) -> vec3<f32> {
    var x_31: i32;
    var y_23: i32;
    var z_13: i32;
    var xoff_3: i32;
    var yoff_3: i32;
    var zoff_1: i32;
    var jitter_3: f32;
    var off_1: vec3<f32>;

    x_31 = x_30;
    y_23 = y_22;
    z_13 = z_12;
    xoff_3 = xoff_2;
    yoff_3 = yoff_2;
    zoff_1 = zoff;
    jitter_3 = jitter_2;
    let _e15 = x_31;
    let _e16 = xoff_3;
    let _e18 = y_23;
    let _e19 = yoff_3;
    let _e21 = z_13;
    let _e22 = zoff_1;
    let _e28 = mx_cell_noise_vec3_2(vec3<f32>(f32((_e15 + _e16)), f32((_e18 + _e19)), f32((_e21 + _e22))));
    off_1 = _e28;
    let _e30 = off_1;
    off_1 = (_e30 - vec3(0.5f));
    let _e34 = off_1;
    let _e35 = jitter_3;
    off_1 = (_e34 * _e35);
    let _e37 = off_1;
    off_1 = (_e37 + vec3(0.5f));
    let _e41 = x_31;
    let _e43 = y_23;
    let _e45 = z_13;
    let _e48 = off_1;
    return (vec3<f32>(f32(_e41), f32(_e43), f32(_e45)) + _e48);
}

fn mx_worley_distance(p_40: vec2<f32>, x_32: i32, y_24: i32, xoff_4: i32, yoff_4: i32, jitter_4: f32, metric: i32) -> f32 {
    var p_41: vec2<f32>;
    var x_33: i32;
    var y_25: i32;
    var xoff_5: i32;
    var yoff_5: i32;
    var jitter_5: f32;
    var metric_1: i32;
    var cellpos: vec2<f32>;
    var diff: vec2<f32>;

    p_41 = p_40;
    x_33 = x_32;
    y_25 = y_24;
    xoff_5 = xoff_4;
    yoff_5 = yoff_4;
    jitter_5 = jitter_4;
    metric_1 = metric;
    let _e15 = x_33;
    let _e16 = y_25;
    let _e17 = xoff_5;
    let _e18 = yoff_5;
    let _e19 = jitter_5;
    let _e20 = mx_worley_cell_position(_e15, _e16, _e17, _e18, _e19);
    cellpos = _e20;
    let _e22 = cellpos;
    let _e23 = p_41;
    diff = (_e22 - _e23);
    let _e26 = metric_1;
    if (_e26 == 2i) {
        let _e29 = diff;
        let _e32 = diff;
        return (abs(_e29.x) + abs(_e32.y));
    }
    let _e36 = metric_1;
    if (_e36 == 3i) {
        let _e39 = diff;
        let _e42 = diff;
        return max(abs(_e39.x), abs(_e42.y));
    }
    let _e46 = diff;
    let _e47 = diff;
    return dot(_e46, _e47);
}

fn mx_worley_distance_1(p_42: vec3<f32>, x_34: i32, y_26: i32, z_14: i32, xoff_6: i32, yoff_6: i32, zoff_2: i32, jitter_6: f32, metric_2: i32) -> f32 {
    var p_43: vec3<f32>;
    var x_35: i32;
    var y_27: i32;
    var z_15: i32;
    var xoff_7: i32;
    var yoff_7: i32;
    var zoff_3: i32;
    var jitter_7: f32;
    var metric_3: i32;
    var cellpos_1: vec3<f32>;
    var diff_1: vec3<f32>;

    p_43 = p_42;
    x_35 = x_34;
    y_27 = y_26;
    z_15 = z_14;
    xoff_7 = xoff_6;
    yoff_7 = yoff_6;
    zoff_3 = zoff_2;
    jitter_7 = jitter_6;
    metric_3 = metric_2;
    let _e19 = x_35;
    let _e20 = y_27;
    let _e21 = z_15;
    let _e22 = xoff_7;
    let _e23 = yoff_7;
    let _e24 = zoff_3;
    let _e25 = jitter_7;
    let _e26 = mx_worley_cell_position_1(_e19, _e20, _e21, _e22, _e23, _e24, _e25);
    cellpos_1 = _e26;
    let _e28 = cellpos_1;
    let _e29 = p_43;
    diff_1 = (_e28 - _e29);
    let _e32 = metric_3;
    if (_e32 == 2i) {
        let _e35 = diff_1;
        let _e38 = diff_1;
        let _e42 = diff_1;
        return ((abs(_e35.x) + abs(_e38.y)) + abs(_e42.z));
    }
    let _e46 = metric_3;
    if (_e46 == 3i) {
        let _e49 = diff_1;
        let _e52 = diff_1;
        let _e56 = diff_1;
        return max(max(abs(_e49.x), abs(_e52.y)), abs(_e56.z));
    }
    let _e60 = diff_1;
    let _e61 = diff_1;
    return dot(_e60, _e61);
}

fn mx_worley_noise_float(p_44: vec2<f32>, jitter_8: f32, style: i32, metric_4: i32) -> f32 {
    var p_45: vec2<f32>;
    var jitter_9: f32;
    var style_1: i32;
    var metric_5: i32;
    var X_4: i32;
    var Y_4: i32;
    var dist: f32;
    var localpos: vec2<f32>;
    var sqdist: f32 = 1000000f;
    var minpos: vec2<f32> = vec2<f32>(0f, 0f);
    var x_36: i32 = -1i;
    var y_28: i32;
    var dist_1: f32;
    var cellpos_2: vec2<f32>;

    p_45 = p_44;
    jitter_9 = jitter_8;
    style_1 = style;
    metric_5 = metric_4;
    let _e12 = p_45;
    let _e16 = mx_floorfrac(_e12.x, (&X_4));
    let _e17 = p_45;
    let _e21 = mx_floorfrac(_e17.y, (&Y_4));
    localpos = vec2<f32>(_e16, _e21);
    loop {
        let _e35 = x_36;
        if !((_e35 <= 1i)) {
            break;
        }
        {
            y_28 = -1i;
            loop {
                let _e45 = y_28;
                if !((_e45 <= 1i)) {
                    break;
                }
                {
                    let _e52 = localpos;
                    let _e53 = x_36;
                    let _e54 = y_28;
                    let _e55 = X_4;
                    let _e56 = Y_4;
                    let _e57 = jitter_9;
                    let _e58 = metric_5;
                    let _e59 = mx_worley_distance(_e52, _e53, _e54, _e55, _e56, _e57, _e58);
                    dist_1 = _e59;
                    let _e61 = x_36;
                    let _e62 = y_28;
                    let _e63 = X_4;
                    let _e64 = Y_4;
                    let _e65 = jitter_9;
                    let _e66 = mx_worley_cell_position(_e61, _e62, _e63, _e64, _e65);
                    let _e67 = localpos;
                    cellpos_2 = (_e66 - _e67);
                    let _e70 = dist_1;
                    let _e71 = sqdist;
                    if (_e70 < _e71) {
                        {
                            let _e73 = dist_1;
                            sqdist = _e73;
                            let _e74 = cellpos_2;
                            minpos = _e74;
                        }
                    }
                }
                continuing {
                    let _e49 = y_28;
                    y_28 = (_e49 + 1i);
                }
            }
        }
        continuing {
            let _e39 = x_36;
            x_36 = (_e39 + 1i);
        }
    }
    let _e75 = style_1;
    if (_e75 == 1i) {
        let _e78 = minpos;
        let _e79 = p_45;
        let _e81 = mx_cell_noise_float_1((_e78 + _e79));
        return _e81;
    } else {
        {
            let _e82 = metric_5;
            if (_e82 == 0i) {
                let _e85 = sqdist;
                sqdist = sqrt(_e85);
            }
            let _e87 = sqdist;
            return _e87;
        }
    }
}

fn mx_worley_noise_vec2_(p_46: vec2<f32>, jitter_10: f32, style_2: i32, metric_6: i32) -> vec2<f32> {
    var p_47: vec2<f32>;
    var jitter_11: f32;
    var style_3: i32;
    var metric_7: i32;
    var X_5: i32;
    var Y_5: i32;
    var localpos_1: vec2<f32>;
    var sqdist_1: vec2<f32> = vec2<f32>(1000000f, 1000000f);
    var minpos_1: vec2<f32> = vec2<f32>(0f, 0f);
    var x_37: i32 = -1i;
    var y_29: i32;
    var dist_2: f32;
    var cellpos_3: vec2<f32>;
    var tmp_1: vec3<f32>;

    p_47 = p_46;
    jitter_11 = jitter_10;
    style_3 = style_2;
    metric_7 = metric_6;
    let _e11 = p_47;
    let _e15 = mx_floorfrac(_e11.x, (&X_5));
    let _e16 = p_47;
    let _e20 = mx_floorfrac(_e16.y, (&Y_5));
    localpos_1 = vec2<f32>(_e15, _e20);
    loop {
        let _e36 = x_37;
        if !((_e36 <= 1i)) {
            break;
        }
        {
            y_29 = -1i;
            loop {
                let _e46 = y_29;
                if !((_e46 <= 1i)) {
                    break;
                }
                {
                    let _e53 = localpos_1;
                    let _e54 = x_37;
                    let _e55 = y_29;
                    let _e56 = X_5;
                    let _e57 = Y_5;
                    let _e58 = jitter_11;
                    let _e59 = metric_7;
                    let _e60 = mx_worley_distance(_e53, _e54, _e55, _e56, _e57, _e58, _e59);
                    dist_2 = _e60;
                    let _e62 = x_37;
                    let _e63 = y_29;
                    let _e64 = X_5;
                    let _e65 = Y_5;
                    let _e66 = jitter_11;
                    let _e67 = mx_worley_cell_position(_e62, _e63, _e64, _e65, _e66);
                    let _e68 = localpos_1;
                    cellpos_3 = (_e67 - _e68);
                    let _e71 = dist_2;
                    let _e72 = sqdist_1;
                    if (_e71 < _e72.x) {
                        {
                            let _e76 = sqdist_1;
                            sqdist_1.y = _e76.x;
                            let _e79 = dist_2;
                            sqdist_1.x = _e79;
                            let _e80 = cellpos_3;
                            minpos_1 = _e80;
                        }
                    } else {
                        let _e81 = dist_2;
                        let _e82 = sqdist_1;
                        if (_e81 < _e82.y) {
                            {
                                let _e86 = dist_2;
                                sqdist_1.y = _e86;
                            }
                        }
                    }
                }
                continuing {
                    let _e50 = y_29;
                    y_29 = (_e50 + 1i);
                }
            }
        }
        continuing {
            let _e40 = x_37;
            x_37 = (_e40 + 1i);
        }
    }
    let _e87 = style_3;
    if (_e87 == 1i) {
        {
            let _e90 = minpos_1;
            let _e91 = p_47;
            let _e93 = mx_cell_noise_vec3_1((_e90 + _e91));
            tmp_1 = _e93;
            let _e95 = tmp_1;
            let _e97 = tmp_1;
            return vec2<f32>(_e95.x, _e97.y);
        }
    } else {
        {
            let _e100 = metric_7;
            if (_e100 == 0i) {
                let _e103 = sqdist_1;
                sqdist_1 = sqrt(_e103);
            }
            let _e105 = sqdist_1;
            return _e105;
        }
    }
}

fn mx_worley_noise_vec3_(p_48: vec2<f32>, jitter_12: f32, style_4: i32, metric_8: i32) -> vec3<f32> {
    var p_49: vec2<f32>;
    var jitter_13: f32;
    var style_5: i32;
    var metric_9: i32;
    var X_6: i32;
    var Y_6: i32;
    var localpos_2: vec2<f32>;
    var sqdist_2: vec3<f32> = vec3<f32>(1000000f, 1000000f, 1000000f);
    var minpos_2: vec2<f32> = vec2<f32>(0f, 0f);
    var x_38: i32 = -1i;
    var y_30: i32;
    var dist_3: f32;
    var cellpos_4: vec2<f32>;

    p_49 = p_48;
    jitter_13 = jitter_12;
    style_5 = style_4;
    metric_9 = metric_8;
    let _e11 = p_49;
    let _e15 = mx_floorfrac(_e11.x, (&X_6));
    let _e16 = p_49;
    let _e20 = mx_floorfrac(_e16.y, (&Y_6));
    localpos_2 = vec2<f32>(_e15, _e20);
    loop {
        let _e37 = x_38;
        if !((_e37 <= 1i)) {
            break;
        }
        {
            y_30 = -1i;
            loop {
                let _e47 = y_30;
                if !((_e47 <= 1i)) {
                    break;
                }
                {
                    let _e54 = localpos_2;
                    let _e55 = x_38;
                    let _e56 = y_30;
                    let _e57 = X_6;
                    let _e58 = Y_6;
                    let _e59 = jitter_13;
                    let _e60 = metric_9;
                    let _e61 = mx_worley_distance(_e54, _e55, _e56, _e57, _e58, _e59, _e60);
                    dist_3 = _e61;
                    let _e63 = x_38;
                    let _e64 = y_30;
                    let _e65 = X_6;
                    let _e66 = Y_6;
                    let _e67 = jitter_13;
                    let _e68 = mx_worley_cell_position(_e63, _e64, _e65, _e66, _e67);
                    let _e69 = localpos_2;
                    cellpos_4 = (_e68 - _e69);
                    let _e72 = dist_3;
                    let _e73 = sqdist_2;
                    if (_e72 < _e73.x) {
                        {
                            let _e77 = sqdist_2;
                            sqdist_2.z = _e77.y;
                            let _e80 = sqdist_2;
                            sqdist_2.y = _e80.x;
                            let _e83 = dist_3;
                            sqdist_2.x = _e83;
                            let _e84 = cellpos_4;
                            minpos_2 = _e84;
                        }
                    } else {
                        let _e85 = dist_3;
                        let _e86 = sqdist_2;
                        if (_e85 < _e86.y) {
                            {
                                let _e90 = sqdist_2;
                                sqdist_2.z = _e90.y;
                                let _e93 = dist_3;
                                sqdist_2.y = _e93;
                            }
                        } else {
                            let _e94 = dist_3;
                            let _e95 = sqdist_2;
                            if (_e94 < _e95.z) {
                                {
                                    let _e99 = dist_3;
                                    sqdist_2.z = _e99;
                                }
                            }
                        }
                    }
                }
                continuing {
                    let _e51 = y_30;
                    y_30 = (_e51 + 1i);
                }
            }
        }
        continuing {
            let _e41 = x_38;
            x_38 = (_e41 + 1i);
        }
    }
    let _e100 = style_5;
    if (_e100 == 1i) {
        let _e103 = minpos_2;
        let _e104 = p_49;
        let _e106 = mx_cell_noise_vec3_1((_e103 + _e104));
        return _e106;
    } else {
        {
            let _e107 = metric_9;
            if (_e107 == 0i) {
                let _e110 = sqdist_2;
                sqdist_2 = sqrt(_e110);
            }
            let _e112 = sqdist_2;
            return _e112;
        }
    }
}

fn mx_worley_noise_float_1(p_50: vec3<f32>, jitter_14: f32, style_6: i32, metric_10: i32) -> f32 {
    var p_51: vec3<f32>;
    var jitter_15: f32;
    var style_7: i32;
    var metric_11: i32;
    var X_7: i32;
    var Y_7: i32;
    var Z_2: i32;
    var localpos_3: vec3<f32>;
    var sqdist_3: f32 = 1000000f;
    var minpos_3: vec3<f32> = vec3<f32>(0f, 0f, 0f);
    var x_39: i32 = -1i;
    var y_31: i32;
    var z_16: i32;
    var dist_4: f32;
    var cellpos_5: vec3<f32>;

    p_51 = p_50;
    jitter_15 = jitter_14;
    style_7 = style_6;
    metric_11 = metric_10;
    let _e12 = p_51;
    let _e16 = mx_floorfrac(_e12.x, (&X_7));
    let _e17 = p_51;
    let _e21 = mx_floorfrac(_e17.y, (&Y_7));
    let _e22 = p_51;
    let _e26 = mx_floorfrac(_e22.z, (&Z_2));
    localpos_3 = vec3<f32>(_e16, _e21, _e26);
    loop {
        let _e42 = x_39;
        if !((_e42 <= 1i)) {
            break;
        }
        {
            y_31 = -1i;
            loop {
                let _e52 = y_31;
                if !((_e52 <= 1i)) {
                    break;
                }
                {
                    z_16 = -1i;
                    loop {
                        let _e62 = z_16;
                        if !((_e62 <= 1i)) {
                            break;
                        }
                        {
                            let _e69 = localpos_3;
                            let _e70 = x_39;
                            let _e71 = y_31;
                            let _e72 = z_16;
                            let _e73 = X_7;
                            let _e74 = Y_7;
                            let _e75 = Z_2;
                            let _e76 = jitter_15;
                            let _e77 = metric_11;
                            let _e78 = mx_worley_distance_1(_e69, _e70, _e71, _e72, _e73, _e74, _e75, _e76, _e77);
                            dist_4 = _e78;
                            let _e80 = x_39;
                            let _e81 = y_31;
                            let _e82 = z_16;
                            let _e83 = X_7;
                            let _e84 = Y_7;
                            let _e85 = Z_2;
                            let _e86 = jitter_15;
                            let _e87 = mx_worley_cell_position_1(_e80, _e81, _e82, _e83, _e84, _e85, _e86);
                            let _e88 = localpos_3;
                            cellpos_5 = (_e87 - _e88);
                            let _e91 = dist_4;
                            let _e92 = sqdist_3;
                            if (_e91 < _e92) {
                                {
                                    let _e94 = dist_4;
                                    sqdist_3 = _e94;
                                    let _e95 = cellpos_5;
                                    minpos_3 = _e95;
                                }
                            }
                        }
                        continuing {
                            let _e66 = z_16;
                            z_16 = (_e66 + 1i);
                        }
                    }
                }
                continuing {
                    let _e56 = y_31;
                    y_31 = (_e56 + 1i);
                }
            }
        }
        continuing {
            let _e46 = x_39;
            x_39 = (_e46 + 1i);
        }
    }
    let _e96 = style_7;
    if (_e96 == 1i) {
        let _e99 = minpos_3;
        let _e100 = p_51;
        let _e102 = mx_cell_noise_float_2((_e99 + _e100));
        return _e102;
    } else {
        {
            let _e103 = metric_11;
            if (_e103 == 0i) {
                let _e106 = sqdist_3;
                sqdist_3 = sqrt(_e106);
            }
            let _e108 = sqdist_3;
            return _e108;
        }
    }
}

fn mx_worley_noise_vec2_1(p_52: vec3<f32>, jitter_16: f32, style_8: i32, metric_12: i32) -> vec2<f32> {
    var p_53: vec3<f32>;
    var jitter_17: f32;
    var style_9: i32;
    var metric_13: i32;
    var X_8: i32;
    var Y_8: i32;
    var Z_3: i32;
    var localpos_4: vec3<f32>;
    var sqdist_4: vec2<f32> = vec2<f32>(1000000f, 1000000f);
    var minpos_4: vec3<f32> = vec3<f32>(0f, 0f, 0f);
    var x_40: i32 = -1i;
    var y_32: i32;
    var z_17: i32;
    var dist_5: f32;
    var cellpos_6: vec3<f32>;
    var tmp_2: vec3<f32>;

    p_53 = p_52;
    jitter_17 = jitter_16;
    style_9 = style_8;
    metric_13 = metric_12;
    let _e12 = p_53;
    let _e16 = mx_floorfrac(_e12.x, (&X_8));
    let _e17 = p_53;
    let _e21 = mx_floorfrac(_e17.y, (&Y_8));
    let _e22 = p_53;
    let _e26 = mx_floorfrac(_e22.z, (&Z_3));
    localpos_4 = vec3<f32>(_e16, _e21, _e26);
    loop {
        let _e44 = x_40;
        if !((_e44 <= 1i)) {
            break;
        }
        {
            y_32 = -1i;
            loop {
                let _e54 = y_32;
                if !((_e54 <= 1i)) {
                    break;
                }
                {
                    z_17 = -1i;
                    loop {
                        let _e64 = z_17;
                        if !((_e64 <= 1i)) {
                            break;
                        }
                        {
                            let _e71 = localpos_4;
                            let _e72 = x_40;
                            let _e73 = y_32;
                            let _e74 = z_17;
                            let _e75 = X_8;
                            let _e76 = Y_8;
                            let _e77 = Z_3;
                            let _e78 = jitter_17;
                            let _e79 = metric_13;
                            let _e80 = mx_worley_distance_1(_e71, _e72, _e73, _e74, _e75, _e76, _e77, _e78, _e79);
                            dist_5 = _e80;
                            let _e82 = x_40;
                            let _e83 = y_32;
                            let _e84 = z_17;
                            let _e85 = X_8;
                            let _e86 = Y_8;
                            let _e87 = Z_3;
                            let _e88 = jitter_17;
                            let _e89 = mx_worley_cell_position_1(_e82, _e83, _e84, _e85, _e86, _e87, _e88);
                            let _e90 = localpos_4;
                            cellpos_6 = (_e89 - _e90);
                            let _e93 = dist_5;
                            let _e94 = sqdist_4;
                            if (_e93 < _e94.x) {
                                {
                                    let _e98 = sqdist_4;
                                    sqdist_4.y = _e98.x;
                                    let _e101 = dist_5;
                                    sqdist_4.x = _e101;
                                    let _e102 = cellpos_6;
                                    minpos_4 = _e102;
                                }
                            } else {
                                let _e103 = dist_5;
                                let _e104 = sqdist_4;
                                if (_e103 < _e104.y) {
                                    {
                                        let _e108 = dist_5;
                                        sqdist_4.y = _e108;
                                    }
                                }
                            }
                        }
                        continuing {
                            let _e68 = z_17;
                            z_17 = (_e68 + 1i);
                        }
                    }
                }
                continuing {
                    let _e58 = y_32;
                    y_32 = (_e58 + 1i);
                }
            }
        }
        continuing {
            let _e48 = x_40;
            x_40 = (_e48 + 1i);
        }
    }
    let _e109 = style_9;
    if (_e109 == 1i) {
        {
            let _e112 = minpos_4;
            let _e113 = p_53;
            let _e115 = mx_cell_noise_vec3_2((_e112 + _e113));
            tmp_2 = _e115;
            let _e117 = tmp_2;
            let _e119 = tmp_2;
            return vec2<f32>(_e117.x, _e119.y);
        }
    } else {
        {
            let _e122 = metric_13;
            if (_e122 == 0i) {
                let _e125 = sqdist_4;
                sqdist_4 = sqrt(_e125);
            }
            let _e127 = sqdist_4;
            return _e127;
        }
    }
}

fn mx_worley_noise_vec3_1(p_54: vec3<f32>, jitter_18: f32, style_10: i32, metric_14: i32) -> vec3<f32> {
    var p_55: vec3<f32>;
    var jitter_19: f32;
    var style_11: i32;
    var metric_15: i32;
    var X_9: i32;
    var Y_9: i32;
    var Z_4: i32;
    var localpos_5: vec3<f32>;
    var sqdist_5: vec3<f32> = vec3<f32>(1000000f, 1000000f, 1000000f);
    var minpos_5: vec3<f32> = vec3<f32>(0f, 0f, 0f);
    var x_41: i32 = -1i;
    var y_33: i32;
    var z_18: i32;
    var dist_6: f32;
    var cellpos_7: vec3<f32>;

    p_55 = p_54;
    jitter_19 = jitter_18;
    style_11 = style_10;
    metric_15 = metric_14;
    let _e12 = p_55;
    let _e16 = mx_floorfrac(_e12.x, (&X_9));
    let _e17 = p_55;
    let _e21 = mx_floorfrac(_e17.y, (&Y_9));
    let _e22 = p_55;
    let _e26 = mx_floorfrac(_e22.z, (&Z_4));
    localpos_5 = vec3<f32>(_e16, _e21, _e26);
    loop {
        let _e45 = x_41;
        if !((_e45 <= 1i)) {
            break;
        }
        {
            y_33 = -1i;
            loop {
                let _e55 = y_33;
                if !((_e55 <= 1i)) {
                    break;
                }
                {
                    z_18 = -1i;
                    loop {
                        let _e65 = z_18;
                        if !((_e65 <= 1i)) {
                            break;
                        }
                        {
                            let _e72 = localpos_5;
                            let _e73 = x_41;
                            let _e74 = y_33;
                            let _e75 = z_18;
                            let _e76 = X_9;
                            let _e77 = Y_9;
                            let _e78 = Z_4;
                            let _e79 = jitter_19;
                            let _e80 = metric_15;
                            let _e81 = mx_worley_distance_1(_e72, _e73, _e74, _e75, _e76, _e77, _e78, _e79, _e80);
                            dist_6 = _e81;
                            let _e83 = x_41;
                            let _e84 = y_33;
                            let _e85 = z_18;
                            let _e86 = X_9;
                            let _e87 = Y_9;
                            let _e88 = Z_4;
                            let _e89 = jitter_19;
                            let _e90 = mx_worley_cell_position_1(_e83, _e84, _e85, _e86, _e87, _e88, _e89);
                            let _e91 = localpos_5;
                            cellpos_7 = (_e90 - _e91);
                            let _e94 = dist_6;
                            let _e95 = sqdist_5;
                            if (_e94 < _e95.x) {
                                {
                                    let _e99 = sqdist_5;
                                    sqdist_5.z = _e99.y;
                                    let _e102 = sqdist_5;
                                    sqdist_5.y = _e102.x;
                                    let _e105 = dist_6;
                                    sqdist_5.x = _e105;
                                    let _e106 = cellpos_7;
                                    minpos_5 = _e106;
                                }
                            } else {
                                let _e107 = dist_6;
                                let _e108 = sqdist_5;
                                if (_e107 < _e108.y) {
                                    {
                                        let _e112 = sqdist_5;
                                        sqdist_5.z = _e112.y;
                                        let _e115 = dist_6;
                                        sqdist_5.y = _e115;
                                    }
                                } else {
                                    let _e116 = dist_6;
                                    let _e117 = sqdist_5;
                                    if (_e116 < _e117.z) {
                                        {
                                            let _e121 = dist_6;
                                            sqdist_5.z = _e121;
                                        }
                                    }
                                }
                            }
                        }
                        continuing {
                            let _e69 = z_18;
                            z_18 = (_e69 + 1i);
                        }
                    }
                }
                continuing {
                    let _e59 = y_33;
                    y_33 = (_e59 + 1i);
                }
            }
        }
        continuing {
            let _e49 = x_41;
            x_41 = (_e49 + 1i);
        }
    }
    let _e122 = style_11;
    if (_e122 == 1i) {
        let _e125 = minpos_5;
        let _e126 = p_55;
        let _e128 = mx_cell_noise_vec3_2((_e125 + _e126));
        return _e128;
    } else {
        {
            let _e129 = metric_15;
            if (_e129 == 0i) {
                let _e132 = sqdist_5;
                sqdist_5 = sqrt(_e132);
            }
            let _e134 = sqdist_5;
            return _e134;
        }
    }
}

// Main function from mx_fractal2d_vector4.glsl
fn mx_fractal2d_vector4_(amplitude_4: vec4<f32>, octaves_16: i32, lacunarity_16: f32, diminish_16: f32, texcoord: vec2<f32>, result_10: ptr<function, vec4<f32>>) {
    var amplitude_5: vec4<f32>;
    var octaves_17: i32;
    var lacunarity_17: f32;
    var diminish_17: f32;
    var texcoord_1: vec2<f32>;
    var value: vec4<f32>;

    amplitude_5 = amplitude_4;
    octaves_17 = octaves_16;
    lacunarity_17 = lacunarity_16;
    diminish_17 = diminish_16;
    texcoord_1 = texcoord;
    let _e12 = texcoord_1;
    let _e13 = octaves_17;
    let _e14 = lacunarity_17;
    let _e15 = diminish_17;
    let _e16 = mx_fractal2d_noise_vec4_(_e12, _e13, _e14, _e15);
    value = _e16;
    let _e18 = value;
    let _e19 = amplitude_5;
    (*result_10) = (_e18 * _e19);
    return;
}