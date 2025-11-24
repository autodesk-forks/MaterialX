fn mx_select(b: bool, t: f32, f: f32) -> f32 {
    var b_1: bool;
    var t_1: f32;
    var f_1: f32;
    var local: f32;

    b_1 = b;
    t_1 = t;
    f_1 = f;
    let _e6 = b_1;
    if _e6 {
        let _e7 = t_1;
        local = _e7;
    } else {
        let _e8 = f_1;
        local = _e8;
    }
    let _e10 = local;
    return _e10;
}

fn mx_negate_if(val: f32, b_2: bool) -> f32 {
    var val_1: f32;
    var b_3: bool;
    var local_1: f32;

    val_1 = val;
    b_3 = b_2;
    let _e4 = b_3;
    if _e4 {
        let _e5 = val_1;
        local_1 = -(_e5);
    } else {
        let _e7 = val_1;
        local_1 = _e7;
    }
    let _e9 = local_1;
    return _e9;
}

fn mx_floor(x: f32) -> i32 {
    var x_1: f32;

    x_1 = x;
    let _e2 = x_1;
    return i32(floor(_e2));
}

fn mx_floorfrac(x_2: f32, i: ptr<function, i32>) -> f32 {
    var x_3: f32;

    x_3 = x_2;
    let _e3 = x_3;
    let _e4 = mx_floor(_e3);
    (*i) = _e4;
    let _e5 = x_3;
    let _e6 = (*i);
    return (_e5 - f32(_e6));
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
    let _e13 = s_1;
    s1_ = (1f - _e13);
    let _e17 = t_3;
    let _e19 = v0_1;
    let _e20 = s1_;
    let _e22 = v1_1;
    let _e23 = s_1;
    let _e27 = t_3;
    let _e28 = v2_1;
    let _e29 = s1_;
    let _e31 = v3_1;
    let _e32 = s_1;
    return (((1f - _e17) * ((_e19 * _e20) + (_e22 * _e23))) + (_e27 * ((_e28 * _e29) + (_e31 * _e32))));
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
    let _e13 = s_3;
    s1_1 = (1f - _e13);
    let _e17 = t_5;
    let _e19 = v0_3;
    let _e20 = s1_1;
    let _e22 = v1_3;
    let _e23 = s_3;
    let _e27 = t_5;
    let _e28 = v2_3;
    let _e29 = s1_1;
    let _e31 = v3_3;
    let _e32 = s_3;
    return (((1f - _e17) * ((_e19 * _e20) + (_e22 * _e23))) + (_e27 * ((_e28 * _e29) + (_e31 * _e32))));
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
    let _e23 = s_5;
    s1_2 = (1f - _e23);
    let _e27 = t_7;
    t1_ = (1f - _e27);
    let _e31 = r_1;
    r1_ = (1f - _e31);
    let _e34 = r1_;
    let _e35 = t1_;
    let _e36 = v0_5;
    let _e37 = s1_2;
    let _e39 = v1_5;
    let _e40 = s_5;
    let _e44 = t_7;
    let _e45 = v2_5;
    let _e46 = s1_2;
    let _e48 = v3_5;
    let _e49 = s_5;
    let _e55 = r_1;
    let _e56 = t1_;
    let _e57 = v4_1;
    let _e58 = s1_2;
    let _e60 = v5_1;
    let _e61 = s_5;
    let _e65 = t_7;
    let _e66 = v6_1;
    let _e67 = s1_2;
    let _e69 = v7_1;
    let _e70 = s_5;
    return ((_e34 * ((_e35 * ((_e36 * _e37) + (_e39 * _e40))) + (_e44 * ((_e45 * _e46) + (_e48 * _e49))))) + (_e55 * ((_e56 * ((_e57 * _e58) + (_e60 * _e61))) + (_e65 * ((_e66 * _e67) + (_e69 * _e70))))));
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
    let _e23 = s_7;
    s1_3 = (1f - _e23);
    let _e27 = t_9;
    t1_1 = (1f - _e27);
    let _e31 = r_3;
    r1_1 = (1f - _e31);
    let _e34 = r1_1;
    let _e35 = t1_1;
    let _e36 = v0_7;
    let _e37 = s1_3;
    let _e39 = v1_7;
    let _e40 = s_7;
    let _e44 = t_9;
    let _e45 = v2_7;
    let _e46 = s1_3;
    let _e48 = v3_7;
    let _e49 = s_7;
    let _e55 = r_3;
    let _e56 = t1_1;
    let _e57 = v4_3;
    let _e58 = s1_3;
    let _e60 = v5_3;
    let _e61 = s_7;
    let _e65 = t_9;
    let _e66 = v6_3;
    let _e67 = s1_3;
    let _e69 = v7_3;
    let _e70 = s_7;
    return ((_e34 * ((_e35 * ((_e36 * _e37) + (_e39 * _e40))) + (_e44 * ((_e45 * _e46) + (_e48 * _e49))))) + (_e55 * ((_e56 * ((_e57 * _e58) + (_e60 * _e61))) + (_e65 * ((_e66 * _e67) + (_e69 * _e70))))));
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
    let _e6 = hash_1;
    h = (_e6 & 7u);
    let _e10 = h;
    let _e13 = x_5;
    let _e14 = y_1;
    let _e15 = mx_select((_e10 < 4u), _e13, _e14);
    u = _e15;
    let _e18 = h;
    let _e21 = y_1;
    let _e22 = x_5;
    let _e23 = mx_select((_e18 < 4u), _e21, _e22);
    v = (2f * _e23);
    let _e26 = u;
    let _e27 = h;
    let _e31 = mx_negate_if(_e26, bool((_e27 & 1u)));
    let _e32 = v;
    let _e33 = h;
    let _e37 = mx_negate_if(_e32, bool((_e33 & 2u)));
    return (_e31 + _e37);
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
    let _e8 = hash_3;
    h_1 = (_e8 & 15u);
    let _e12 = h_1;
    let _e15 = x_7;
    let _e16 = y_3;
    let _e17 = mx_select((_e12 < 8u), _e15, _e16);
    u_1 = _e17;
    let _e19 = h_1;
    let _e22 = y_3;
    let _e23 = h_1;
    let _e26 = h_1;
    let _e30 = x_7;
    let _e31 = z_1;
    let _e32 = mx_select(((_e23 == 12u) || (_e26 == 14u)), _e30, _e31);
    let _e33 = mx_select((_e19 < 4u), _e22, _e32);
    v_1 = _e33;
    let _e35 = u_1;
    let _e36 = h_1;
    let _e40 = mx_negate_if(_e35, bool((_e36 & 1u)));
    let _e41 = v_1;
    let _e42 = h_1;
    let _e46 = mx_negate_if(_e41, bool((_e42 & 2u)));
    return (_e40 + _e46);
}

fn mx_gradient_vec3_(hash_4: vec3<u32>, x_8: f32, y_4: f32) -> vec3<f32> {
    var hash_5: vec3<u32>;
    var x_9: f32;
    var y_5: f32;

    hash_5 = hash_4;
    x_9 = x_8;
    y_5 = y_4;
    let _e6 = hash_5;
    let _e8 = x_9;
    let _e9 = y_5;
    let _e10 = mx_gradient_float(_e6.x, _e8, _e9);
    let _e11 = hash_5;
    let _e13 = x_9;
    let _e14 = y_5;
    let _e15 = mx_gradient_float(_e11.y, _e13, _e14);
    let _e16 = hash_5;
    let _e18 = x_9;
    let _e19 = y_5;
    let _e20 = mx_gradient_float(_e16.z, _e18, _e19);
    return vec3<f32>(_e10, _e15, _e20);
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
    let _e8 = hash_7;
    let _e10 = x_11;
    let _e11 = y_7;
    let _e12 = z_3;
    let _e13 = mx_gradient_float_1(_e8.x, _e10, _e11, _e12);
    let _e14 = hash_7;
    let _e16 = x_11;
    let _e17 = y_7;
    let _e18 = z_3;
    let _e19 = mx_gradient_float_1(_e14.y, _e16, _e17, _e18);
    let _e20 = hash_7;
    let _e22 = x_11;
    let _e23 = y_7;
    let _e24 = z_3;
    let _e25 = mx_gradient_float_1(_e20.z, _e22, _e23, _e24);
    return vec3<f32>(_e13, _e19, _e25);
}

fn mx_gradient_scale2d(v_2: f32) -> f32 {
    var v_3: f32;

    v_3 = v_2;
    let _e3 = v_3;
    return (0.6616f * _e3);
}

fn mx_gradient_scale3d(v_4: f32) -> f32 {
    var v_5: f32;

    v_5 = v_4;
    let _e3 = v_5;
    return (0.982f * _e3);
}

fn mx_gradient_scale2d_1(v_6: vec3<f32>) -> vec3<f32> {
    var v_7: vec3<f32>;

    v_7 = v_6;
    let _e3 = v_7;
    return (0.6616f * _e3);
}

fn mx_gradient_scale3d_1(v_8: vec3<f32>) -> vec3<f32> {
    var v_9: vec3<f32>;

    v_9 = v_8;
    let _e3 = v_9;
    return (0.982f * _e3);
}

fn mx_rotl32_(x_12: u32, k: i32) -> u32 {
    var x_13: u32;
    var k_1: i32;

    x_13 = x_12;
    k_1 = k;
    let _e4 = x_13;
    let _e5 = k_1;
    let _e8 = x_13;
    let _e10 = k_1;
    return ((_e4 << u32(_e5)) | (_e8 >> u32((32i - _e10))));
}

fn mx_bjmix(a: ptr<function, u32>, b_4: ptr<function, u32>, c: ptr<function, u32>) {
    let _e3 = (*a);
    let _e4 = (*c);
    (*a) = (_e3 - _e4);
    let _e6 = (*a);
    let _e7 = (*c);
    let _e9 = mx_rotl32_(_e7, 4i);
    (*a) = (_e6 ^ _e9);
    let _e11 = (*c);
    let _e12 = (*b_4);
    (*c) = (_e11 + _e12);
    let _e14 = (*b_4);
    let _e15 = (*a);
    (*b_4) = (_e14 - _e15);
    let _e17 = (*b_4);
    let _e18 = (*a);
    let _e20 = mx_rotl32_(_e18, 6i);
    (*b_4) = (_e17 ^ _e20);
    let _e22 = (*a);
    let _e23 = (*c);
    (*a) = (_e22 + _e23);
    let _e25 = (*c);
    let _e26 = (*b_4);
    (*c) = (_e25 - _e26);
    let _e28 = (*c);
    let _e29 = (*b_4);
    let _e31 = mx_rotl32_(_e29, 8i);
    (*c) = (_e28 ^ _e31);
    let _e33 = (*b_4);
    let _e34 = (*a);
    (*b_4) = (_e33 + _e34);
    let _e36 = (*a);
    let _e37 = (*c);
    (*a) = (_e36 - _e37);
    let _e39 = (*a);
    let _e40 = (*c);
    let _e42 = mx_rotl32_(_e40, 16i);
    (*a) = (_e39 ^ _e42);
    let _e44 = (*c);
    let _e45 = (*b_4);
    (*c) = (_e44 + _e45);
    let _e47 = (*b_4);
    let _e48 = (*a);
    (*b_4) = (_e47 - _e48);
    let _e50 = (*b_4);
    let _e51 = (*a);
    let _e53 = mx_rotl32_(_e51, 19i);
    (*b_4) = (_e50 ^ _e53);
    let _e55 = (*a);
    let _e56 = (*c);
    (*a) = (_e55 + _e56);
    let _e58 = (*c);
    let _e59 = (*b_4);
    (*c) = (_e58 - _e59);
    let _e61 = (*c);
    let _e62 = (*b_4);
    let _e64 = mx_rotl32_(_e62, 4i);
    (*c) = (_e61 ^ _e64);
    let _e66 = (*b_4);
    let _e67 = (*a);
    (*b_4) = (_e66 + _e67);
    return;
}

fn mx_bjfinal(a_1: u32, b_5: u32, c_1: u32) -> u32 {
    var a_2: u32;
    var b_6: u32;
    var c_2: u32;

    a_2 = a_1;
    b_6 = b_5;
    c_2 = c_1;
    let _e6 = c_2;
    let _e7 = b_6;
    c_2 = (_e6 ^ _e7);
    let _e9 = c_2;
    let _e10 = b_6;
    let _e12 = mx_rotl32_(_e10, 14i);
    c_2 = (_e9 - _e12);
    let _e14 = a_2;
    let _e15 = c_2;
    a_2 = (_e14 ^ _e15);
    let _e17 = a_2;
    let _e18 = c_2;
    let _e20 = mx_rotl32_(_e18, 11i);
    a_2 = (_e17 - _e20);
    let _e22 = b_6;
    let _e23 = a_2;
    b_6 = (_e22 ^ _e23);
    let _e25 = b_6;
    let _e26 = a_2;
    let _e28 = mx_rotl32_(_e26, 25i);
    b_6 = (_e25 - _e28);
    let _e30 = c_2;
    let _e31 = b_6;
    c_2 = (_e30 ^ _e31);
    let _e33 = c_2;
    let _e34 = b_6;
    let _e36 = mx_rotl32_(_e34, 16i);
    c_2 = (_e33 - _e36);
    let _e38 = a_2;
    let _e39 = c_2;
    a_2 = (_e38 ^ _e39);
    let _e41 = a_2;
    let _e42 = c_2;
    let _e44 = mx_rotl32_(_e42, 4i);
    a_2 = (_e41 - _e44);
    let _e46 = b_6;
    let _e47 = a_2;
    b_6 = (_e46 ^ _e47);
    let _e49 = b_6;
    let _e50 = a_2;
    let _e52 = mx_rotl32_(_e50, 14i);
    b_6 = (_e49 - _e52);
    let _e54 = c_2;
    let _e55 = b_6;
    c_2 = (_e54 ^ _e55);
    let _e57 = c_2;
    let _e58 = b_6;
    let _e60 = mx_rotl32_(_e58, 24i);
    c_2 = (_e57 - _e60);
    let _e62 = c_2;
    return _e62;
}

fn mx_bits_to_01_(bits: u32) -> f32 {
    var bits_1: u32;

    bits_1 = bits;
    let _e2 = bits_1;
    return (f32(_e2) / 4294967300f);
}

fn mx_fade(t_10: f32) -> f32 {
    var t_11: f32;

    t_11 = t_10;
    let _e2 = t_11;
    let _e3 = t_11;
    let _e5 = t_11;
    let _e7 = t_11;
    let _e8 = t_11;
    return (((_e2 * _e3) * _e5) * ((_e7 * ((_e8 * 6f) - 15f)) + 10f));
}

fn mx_hash_int(x_14: i32) -> u32 {
    var x_15: i32;
    var len: u32 = 1u;
    var seed: u32;

    x_15 = x_14;
    let _e6 = len;
    seed = ((3735928559u + (_e6 << 2u)) + 13u);
    let _e13 = seed;
    let _e14 = x_15;
    let _e17 = seed;
    let _e18 = seed;
    let _e19 = mx_bjfinal((_e13 + u32(_e14)), _e17, _e18);
    return _e19;
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
    let _e11 = len_1;
    let _e16 = ((3735928559u + (_e11 << 2u)) + 13u);
    c_3 = _e16;
    b_7 = _e16;
    a_3 = _e16;
    let _e17 = a_3;
    let _e18 = x_17;
    a_3 = (_e17 + u32(_e18));
    let _e21 = b_7;
    let _e22 = y_9;
    b_7 = (_e21 + u32(_e22));
    let _e25 = a_3;
    let _e26 = b_7;
    let _e27 = c_3;
    let _e28 = mx_bjfinal(_e25, _e26, _e27);
    return _e28;
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
    let _e13 = len_2;
    let _e18 = ((3735928559u + (_e13 << 2u)) + 13u);
    c_4 = _e18;
    b_8 = _e18;
    a_4 = _e18;
    let _e19 = a_4;
    let _e20 = x_19;
    a_4 = (_e19 + u32(_e20));
    let _e23 = b_8;
    let _e24 = y_11;
    b_8 = (_e23 + u32(_e24));
    let _e27 = c_4;
    let _e28 = z_5;
    c_4 = (_e27 + u32(_e28));
    let _e31 = a_4;
    let _e32 = b_8;
    let _e33 = c_4;
    let _e34 = mx_bjfinal(_e31, _e32, _e33);
    return _e34;
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
    let _e15 = len_3;
    let _e20 = ((3735928559u + (_e15 << 2u)) + 13u);
    c_5 = _e20;
    b_9 = _e20;
    a_5 = _e20;
    let _e21 = a_5;
    let _e22 = x_21;
    a_5 = (_e21 + u32(_e22));
    let _e25 = b_9;
    let _e26 = y_13;
    b_9 = (_e25 + u32(_e26));
    let _e29 = c_5;
    let _e30 = z_7;
    c_5 = (_e29 + u32(_e30));
    mx_bjmix((&a_5), (&b_9), (&c_5));
    let _e39 = a_5;
    let _e40 = xx_1;
    a_5 = (_e39 + u32(_e40));
    let _e43 = a_5;
    let _e44 = b_9;
    let _e45 = c_5;
    let _e46 = mx_bjfinal(_e43, _e44, _e45);
    return _e46;
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
    let _e17 = len_4;
    let _e22 = ((3735928559u + (_e17 << 2u)) + 13u);
    c_6 = _e22;
    b_10 = _e22;
    a_6 = _e22;
    let _e23 = a_6;
    let _e24 = x_23;
    a_6 = (_e23 + u32(_e24));
    let _e27 = b_10;
    let _e28 = y_15;
    b_10 = (_e27 + u32(_e28));
    let _e31 = c_6;
    let _e32 = z_9;
    c_6 = (_e31 + u32(_e32));
    mx_bjmix((&a_6), (&b_10), (&c_6));
    let _e41 = a_6;
    let _e42 = xx_3;
    a_6 = (_e41 + u32(_e42));
    let _e45 = b_10;
    let _e46 = yy_1;
    b_10 = (_e45 + u32(_e46));
    let _e49 = a_6;
    let _e50 = b_10;
    let _e51 = c_6;
    let _e52 = mx_bjfinal(_e49, _e50, _e51);
    return _e52;
}

fn mx_hash_vec3_(x_24: i32, y_16: i32) -> vec3<u32> {
    var x_25: i32;
    var y_17: i32;
    var h_2: u32;
    var result: vec3<u32>;

    x_25 = x_24;
    y_17 = y_16;
    let _e4 = x_25;
    let _e5 = y_17;
    let _e6 = mx_hash_int_1(_e4, _e5);
    h_2 = _e6;
    let _e10 = h_2;
    result.x = (_e10 & 255u);
    let _e14 = h_2;
    result.y = ((_e14 >> 8u) & 255u);
    let _e21 = h_2;
    result.z = ((_e21 >> 16u) & 255u);
    let _e27 = result;
    return _e27;
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
    let _e6 = x_27;
    let _e7 = y_19;
    let _e8 = z_11;
    let _e9 = mx_hash_int_2(_e6, _e7, _e8);
    h_3 = _e9;
    let _e13 = h_3;
    result_1.x = (_e13 & 255u);
    let _e17 = h_3;
    result_1.y = ((_e17 >> 8u) & 255u);
    let _e24 = h_3;
    result_1.z = ((_e24 >> 16u) & 255u);
    let _e30 = result_1;
    return _e30;
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
    let _e4 = p_1;
    let _e8 = mx_floorfrac(_e4.x, (&X));
    fx = _e8;
    let _e10 = p_1;
    let _e14 = mx_floorfrac(_e10.y, (&Y));
    fy = _e14;
    let _e16 = fx;
    let _e17 = mx_fade(_e16);
    u_2 = _e17;
    let _e19 = fy;
    let _e20 = mx_fade(_e19);
    v_10 = _e20;
    let _e22 = X;
    let _e23 = Y;
    let _e24 = mx_hash_int_1(_e22, _e23);
    let _e25 = fx;
    let _e26 = fy;
    let _e27 = mx_gradient_float(_e24, _e25, _e26);
    let _e28 = X;
    let _e31 = Y;
    let _e32 = mx_hash_int_1((_e28 + 1i), _e31);
    let _e33 = fx;
    let _e36 = fy;
    let _e37 = mx_gradient_float(_e32, (_e33 - 1f), _e36);
    let _e38 = X;
    let _e39 = Y;
    let _e42 = mx_hash_int_1(_e38, (_e39 + 1i));
    let _e43 = fx;
    let _e44 = fy;
    let _e47 = mx_gradient_float(_e42, _e43, (_e44 - 1f));
    let _e48 = X;
    let _e51 = Y;
    let _e54 = mx_hash_int_1((_e48 + 1i), (_e51 + 1i));
    let _e55 = fx;
    let _e58 = fy;
    let _e61 = mx_gradient_float(_e54, (_e55 - 1f), (_e58 - 1f));
    let _e62 = u_2;
    let _e63 = v_10;
    let _e64 = mx_bilerp(_e27, _e37, _e47, _e61, _e62, _e63);
    result_2 = _e64;
    let _e66 = result_2;
    let _e67 = mx_gradient_scale2d(_e66);
    return _e67;
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
    let _e5 = p_3;
    let _e9 = mx_floorfrac(_e5.x, (&X_1));
    fx_1 = _e9;
    let _e11 = p_3;
    let _e15 = mx_floorfrac(_e11.y, (&Y_1));
    fy_1 = _e15;
    let _e17 = p_3;
    let _e21 = mx_floorfrac(_e17.z, (&Z));
    fz = _e21;
    let _e23 = fx_1;
    let _e24 = mx_fade(_e23);
    u_3 = _e24;
    let _e26 = fy_1;
    let _e27 = mx_fade(_e26);
    v_11 = _e27;
    let _e29 = fz;
    let _e30 = mx_fade(_e29);
    w = _e30;
    let _e32 = X_1;
    let _e33 = Y_1;
    let _e34 = Z;
    let _e35 = mx_hash_int_2(_e32, _e33, _e34);
    let _e36 = fx_1;
    let _e37 = fy_1;
    let _e38 = fz;
    let _e39 = mx_gradient_float_1(_e35, _e36, _e37, _e38);
    let _e40 = X_1;
    let _e43 = Y_1;
    let _e44 = Z;
    let _e45 = mx_hash_int_2((_e40 + 1i), _e43, _e44);
    let _e46 = fx_1;
    let _e49 = fy_1;
    let _e50 = fz;
    let _e51 = mx_gradient_float_1(_e45, (_e46 - 1f), _e49, _e50);
    let _e52 = X_1;
    let _e53 = Y_1;
    let _e56 = Z;
    let _e57 = mx_hash_int_2(_e52, (_e53 + 1i), _e56);
    let _e58 = fx_1;
    let _e59 = fy_1;
    let _e62 = fz;
    let _e63 = mx_gradient_float_1(_e57, _e58, (_e59 - 1f), _e62);
    let _e64 = X_1;
    let _e67 = Y_1;
    let _e70 = Z;
    let _e71 = mx_hash_int_2((_e64 + 1i), (_e67 + 1i), _e70);
    let _e72 = fx_1;
    let _e75 = fy_1;
    let _e78 = fz;
    let _e79 = mx_gradient_float_1(_e71, (_e72 - 1f), (_e75 - 1f), _e78);
    let _e80 = X_1;
    let _e81 = Y_1;
    let _e82 = Z;
    let _e85 = mx_hash_int_2(_e80, _e81, (_e82 + 1i));
    let _e86 = fx_1;
    let _e87 = fy_1;
    let _e88 = fz;
    let _e91 = mx_gradient_float_1(_e85, _e86, _e87, (_e88 - 1f));
    let _e92 = X_1;
    let _e95 = Y_1;
    let _e96 = Z;
    let _e99 = mx_hash_int_2((_e92 + 1i), _e95, (_e96 + 1i));
    let _e100 = fx_1;
    let _e103 = fy_1;
    let _e104 = fz;
    let _e107 = mx_gradient_float_1(_e99, (_e100 - 1f), _e103, (_e104 - 1f));
    let _e108 = X_1;
    let _e109 = Y_1;
    let _e112 = Z;
    let _e115 = mx_hash_int_2(_e108, (_e109 + 1i), (_e112 + 1i));
    let _e116 = fx_1;
    let _e117 = fy_1;
    let _e120 = fz;
    let _e123 = mx_gradient_float_1(_e115, _e116, (_e117 - 1f), (_e120 - 1f));
    let _e124 = X_1;
    let _e127 = Y_1;
    let _e130 = Z;
    let _e133 = mx_hash_int_2((_e124 + 1i), (_e127 + 1i), (_e130 + 1i));
    let _e134 = fx_1;
    let _e137 = fy_1;
    let _e140 = fz;
    let _e143 = mx_gradient_float_1(_e133, (_e134 - 1f), (_e137 - 1f), (_e140 - 1f));
    let _e144 = u_3;
    let _e145 = v_11;
    let _e146 = w;
    let _e147 = mx_trilerp(_e39, _e51, _e63, _e79, _e91, _e107, _e123, _e143, _e144, _e145, _e146);
    result_3 = _e147;
    let _e149 = result_3;
    let _e150 = mx_gradient_scale3d(_e149);
    return _e150;
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
    let _e4 = p_5;
    let _e8 = mx_floorfrac(_e4.x, (&X_2));
    fx_2 = _e8;
    let _e10 = p_5;
    let _e14 = mx_floorfrac(_e10.y, (&Y_2));
    fy_2 = _e14;
    let _e16 = fx_2;
    let _e17 = mx_fade(_e16);
    u_4 = _e17;
    let _e19 = fy_2;
    let _e20 = mx_fade(_e19);
    v_12 = _e20;
    let _e22 = X_2;
    let _e23 = Y_2;
    let _e24 = mx_hash_vec3_(_e22, _e23);
    let _e25 = fx_2;
    let _e26 = fy_2;
    let _e27 = mx_gradient_vec3_(_e24, _e25, _e26);
    let _e28 = X_2;
    let _e31 = Y_2;
    let _e32 = mx_hash_vec3_((_e28 + 1i), _e31);
    let _e33 = fx_2;
    let _e36 = fy_2;
    let _e37 = mx_gradient_vec3_(_e32, (_e33 - 1f), _e36);
    let _e38 = X_2;
    let _e39 = Y_2;
    let _e42 = mx_hash_vec3_(_e38, (_e39 + 1i));
    let _e43 = fx_2;
    let _e44 = fy_2;
    let _e47 = mx_gradient_vec3_(_e42, _e43, (_e44 - 1f));
    let _e48 = X_2;
    let _e51 = Y_2;
    let _e54 = mx_hash_vec3_((_e48 + 1i), (_e51 + 1i));
    let _e55 = fx_2;
    let _e58 = fy_2;
    let _e61 = mx_gradient_vec3_(_e54, (_e55 - 1f), (_e58 - 1f));
    let _e62 = u_4;
    let _e63 = v_12;
    let _e64 = mx_bilerp_1(_e27, _e37, _e47, _e61, _e62, _e63);
    result_4 = _e64;
    let _e66 = result_4;
    let _e67 = mx_gradient_scale2d_1(_e66);
    return _e67;
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
    let _e5 = p_7;
    let _e9 = mx_floorfrac(_e5.x, (&X_3));
    fx_3 = _e9;
    let _e11 = p_7;
    let _e15 = mx_floorfrac(_e11.y, (&Y_3));
    fy_3 = _e15;
    let _e17 = p_7;
    let _e21 = mx_floorfrac(_e17.z, (&Z_1));
    fz_1 = _e21;
    let _e23 = fx_3;
    let _e24 = mx_fade(_e23);
    u_5 = _e24;
    let _e26 = fy_3;
    let _e27 = mx_fade(_e26);
    v_13 = _e27;
    let _e29 = fz_1;
    let _e30 = mx_fade(_e29);
    w_1 = _e30;
    let _e32 = X_3;
    let _e33 = Y_3;
    let _e34 = Z_1;
    let _e35 = mx_hash_vec3_1(_e32, _e33, _e34);
    let _e36 = fx_3;
    let _e37 = fy_3;
    let _e38 = fz_1;
    let _e39 = mx_gradient_vec3_1(_e35, _e36, _e37, _e38);
    let _e40 = X_3;
    let _e43 = Y_3;
    let _e44 = Z_1;
    let _e45 = mx_hash_vec3_1((_e40 + 1i), _e43, _e44);
    let _e46 = fx_3;
    let _e49 = fy_3;
    let _e50 = fz_1;
    let _e51 = mx_gradient_vec3_1(_e45, (_e46 - 1f), _e49, _e50);
    let _e52 = X_3;
    let _e53 = Y_3;
    let _e56 = Z_1;
    let _e57 = mx_hash_vec3_1(_e52, (_e53 + 1i), _e56);
    let _e58 = fx_3;
    let _e59 = fy_3;
    let _e62 = fz_1;
    let _e63 = mx_gradient_vec3_1(_e57, _e58, (_e59 - 1f), _e62);
    let _e64 = X_3;
    let _e67 = Y_3;
    let _e70 = Z_1;
    let _e71 = mx_hash_vec3_1((_e64 + 1i), (_e67 + 1i), _e70);
    let _e72 = fx_3;
    let _e75 = fy_3;
    let _e78 = fz_1;
    let _e79 = mx_gradient_vec3_1(_e71, (_e72 - 1f), (_e75 - 1f), _e78);
    let _e80 = X_3;
    let _e81 = Y_3;
    let _e82 = Z_1;
    let _e85 = mx_hash_vec3_1(_e80, _e81, (_e82 + 1i));
    let _e86 = fx_3;
    let _e87 = fy_3;
    let _e88 = fz_1;
    let _e91 = mx_gradient_vec3_1(_e85, _e86, _e87, (_e88 - 1f));
    let _e92 = X_3;
    let _e95 = Y_3;
    let _e96 = Z_1;
    let _e99 = mx_hash_vec3_1((_e92 + 1i), _e95, (_e96 + 1i));
    let _e100 = fx_3;
    let _e103 = fy_3;
    let _e104 = fz_1;
    let _e107 = mx_gradient_vec3_1(_e99, (_e100 - 1f), _e103, (_e104 - 1f));
    let _e108 = X_3;
    let _e109 = Y_3;
    let _e112 = Z_1;
    let _e115 = mx_hash_vec3_1(_e108, (_e109 + 1i), (_e112 + 1i));
    let _e116 = fx_3;
    let _e117 = fy_3;
    let _e120 = fz_1;
    let _e123 = mx_gradient_vec3_1(_e115, _e116, (_e117 - 1f), (_e120 - 1f));
    let _e124 = X_3;
    let _e127 = Y_3;
    let _e130 = Z_1;
    let _e133 = mx_hash_vec3_1((_e124 + 1i), (_e127 + 1i), (_e130 + 1i));
    let _e134 = fx_3;
    let _e137 = fy_3;
    let _e140 = fz_1;
    let _e143 = mx_gradient_vec3_1(_e133, (_e134 - 1f), (_e137 - 1f), (_e140 - 1f));
    let _e144 = u_5;
    let _e145 = v_13;
    let _e146 = w_1;
    let _e147 = mx_trilerp_1(_e39, _e51, _e63, _e79, _e91, _e107, _e123, _e143, _e144, _e145, _e146);
    result_5 = _e147;
    let _e149 = result_5;
    let _e150 = mx_gradient_scale3d_1(_e149);
    return _e150;
}

fn mx_cell_noise_float(p_8: f32) -> f32 {
    var p_9: f32;
    var ix: i32;

    p_9 = p_8;
    let _e2 = p_9;
    let _e3 = mx_floor(_e2);
    ix = _e3;
    let _e5 = ix;
    let _e6 = mx_hash_int(_e5);
    let _e7 = mx_bits_to_01_(_e6);
    return _e7;
}

fn mx_cell_noise_float_1(p_10: vec2<f32>) -> f32 {
    var p_11: vec2<f32>;
    var ix_1: i32;
    var iy: i32;

    p_11 = p_10;
    let _e2 = p_11;
    let _e4 = mx_floor(_e2.x);
    ix_1 = _e4;
    let _e6 = p_11;
    let _e8 = mx_floor(_e6.y);
    iy = _e8;
    let _e10 = ix_1;
    let _e11 = iy;
    let _e12 = mx_hash_int_1(_e10, _e11);
    let _e13 = mx_bits_to_01_(_e12);
    return _e13;
}

fn mx_cell_noise_float_2(p_12: vec3<f32>) -> f32 {
    var p_13: vec3<f32>;
    var ix_2: i32;
    var iy_1: i32;
    var iz: i32;

    p_13 = p_12;
    let _e2 = p_13;
    let _e4 = mx_floor(_e2.x);
    ix_2 = _e4;
    let _e6 = p_13;
    let _e8 = mx_floor(_e6.y);
    iy_1 = _e8;
    let _e10 = p_13;
    let _e12 = mx_floor(_e10.z);
    iz = _e12;
    let _e14 = ix_2;
    let _e15 = iy_1;
    let _e16 = iz;
    let _e17 = mx_hash_int_2(_e14, _e15, _e16);
    let _e18 = mx_bits_to_01_(_e17);
    return _e18;
}

fn mx_cell_noise_float_3(p_14: vec4<f32>) -> f32 {
    var p_15: vec4<f32>;
    var ix_3: i32;
    var iy_2: i32;
    var iz_1: i32;
    var iw: i32;

    p_15 = p_14;
    let _e2 = p_15;
    let _e4 = mx_floor(_e2.x);
    ix_3 = _e4;
    let _e6 = p_15;
    let _e8 = mx_floor(_e6.y);
    iy_2 = _e8;
    let _e10 = p_15;
    let _e12 = mx_floor(_e10.z);
    iz_1 = _e12;
    let _e14 = p_15;
    let _e16 = mx_floor(_e14.w);
    iw = _e16;
    let _e18 = ix_3;
    let _e19 = iy_2;
    let _e20 = iz_1;
    let _e21 = iw;
    let _e22 = mx_hash_int_3(_e18, _e19, _e20, _e21);
    let _e23 = mx_bits_to_01_(_e22);
    return _e23;
}

fn mx_cell_noise_vec3_(p_16: f32) -> vec3<f32> {
    var p_17: f32;
    var ix_4: i32;

    p_17 = p_16;
    let _e2 = p_17;
    let _e3 = mx_floor(_e2);
    ix_4 = _e3;
    let _e5 = ix_4;
    let _e7 = mx_hash_int_1(_e5, 0i);
    let _e8 = mx_bits_to_01_(_e7);
    let _e9 = ix_4;
    let _e11 = mx_hash_int_1(_e9, 1i);
    let _e12 = mx_bits_to_01_(_e11);
    let _e13 = ix_4;
    let _e15 = mx_hash_int_1(_e13, 2i);
    let _e16 = mx_bits_to_01_(_e15);
    return vec3<f32>(_e8, _e12, _e16);
}

fn mx_cell_noise_vec3_1(p_18: vec2<f32>) -> vec3<f32> {
    var p_19: vec2<f32>;
    var ix_5: i32;
    var iy_3: i32;

    p_19 = p_18;
    let _e2 = p_19;
    let _e4 = mx_floor(_e2.x);
    ix_5 = _e4;
    let _e6 = p_19;
    let _e8 = mx_floor(_e6.y);
    iy_3 = _e8;
    let _e10 = ix_5;
    let _e11 = iy_3;
    let _e13 = mx_hash_int_2(_e10, _e11, 0i);
    let _e14 = mx_bits_to_01_(_e13);
    let _e15 = ix_5;
    let _e16 = iy_3;
    let _e18 = mx_hash_int_2(_e15, _e16, 1i);
    let _e19 = mx_bits_to_01_(_e18);
    let _e20 = ix_5;
    let _e21 = iy_3;
    let _e23 = mx_hash_int_2(_e20, _e21, 2i);
    let _e24 = mx_bits_to_01_(_e23);
    return vec3<f32>(_e14, _e19, _e24);
}

fn mx_cell_noise_vec3_2(p_20: vec3<f32>) -> vec3<f32> {
    var p_21: vec3<f32>;
    var ix_6: i32;
    var iy_4: i32;
    var iz_2: i32;

    p_21 = p_20;
    let _e2 = p_21;
    let _e4 = mx_floor(_e2.x);
    ix_6 = _e4;
    let _e6 = p_21;
    let _e8 = mx_floor(_e6.y);
    iy_4 = _e8;
    let _e10 = p_21;
    let _e12 = mx_floor(_e10.z);
    iz_2 = _e12;
    let _e14 = ix_6;
    let _e15 = iy_4;
    let _e16 = iz_2;
    let _e18 = mx_hash_int_3(_e14, _e15, _e16, 0i);
    let _e19 = mx_bits_to_01_(_e18);
    let _e20 = ix_6;
    let _e21 = iy_4;
    let _e22 = iz_2;
    let _e24 = mx_hash_int_3(_e20, _e21, _e22, 1i);
    let _e25 = mx_bits_to_01_(_e24);
    let _e26 = ix_6;
    let _e27 = iy_4;
    let _e28 = iz_2;
    let _e30 = mx_hash_int_3(_e26, _e27, _e28, 2i);
    let _e31 = mx_bits_to_01_(_e30);
    return vec3<f32>(_e19, _e25, _e31);
}

fn mx_cell_noise_vec3_3(p_22: vec4<f32>) -> vec3<f32> {
    var p_23: vec4<f32>;
    var ix_7: i32;
    var iy_5: i32;
    var iz_3: i32;
    var iw_1: i32;

    p_23 = p_22;
    let _e2 = p_23;
    let _e4 = mx_floor(_e2.x);
    ix_7 = _e4;
    let _e6 = p_23;
    let _e8 = mx_floor(_e6.y);
    iy_5 = _e8;
    let _e10 = p_23;
    let _e12 = mx_floor(_e10.z);
    iz_3 = _e12;
    let _e14 = p_23;
    let _e16 = mx_floor(_e14.w);
    iw_1 = _e16;
    let _e18 = ix_7;
    let _e19 = iy_5;
    let _e20 = iz_3;
    let _e21 = iw_1;
    let _e23 = mx_hash_int_4(_e18, _e19, _e20, _e21, 0i);
    let _e24 = mx_bits_to_01_(_e23);
    let _e25 = ix_7;
    let _e26 = iy_5;
    let _e27 = iz_3;
    let _e28 = iw_1;
    let _e30 = mx_hash_int_4(_e25, _e26, _e27, _e28, 1i);
    let _e31 = mx_bits_to_01_(_e30);
    let _e32 = ix_7;
    let _e33 = iy_5;
    let _e34 = iz_3;
    let _e35 = iw_1;
    let _e37 = mx_hash_int_4(_e32, _e33, _e34, _e35, 2i);
    let _e38 = mx_bits_to_01_(_e37);
    return vec3<f32>(_e24, _e31, _e38);
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
        let _e14 = i_1;
        let _e15 = octaves_1;
        if !((_e14 < _e15)) {
            break;
        }
        {
            let _e21 = result_6;
            let _e22 = amplitude;
            let _e23 = p_25;
            let _e24 = mx_perlin_noise_float(_e23);
            result_6 = (_e21 + (_e22 * _e24));
            let _e27 = amplitude;
            let _e28 = diminish_1;
            amplitude = (_e27 * _e28);
            let _e30 = p_25;
            let _e31 = lacunarity_1;
            p_25 = (_e30 * _e31);
        }
        continuing {
            let _e18 = i_1;
            i_1 = (_e18 + 1i);
        }
    }
    let _e33 = result_6;
    return _e33;
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
        let _e15 = i_2;
        let _e16 = octaves_3;
        if !((_e15 < _e16)) {
            break;
        }
        {
            let _e22 = result_7;
            let _e23 = amplitude_1;
            let _e24 = p_27;
            let _e25 = mx_perlin_noise_vec3_(_e24);
            result_7 = (_e22 + (_e23 * _e25));
            let _e28 = amplitude_1;
            let _e29 = diminish_3;
            amplitude_1 = (_e28 * _e29);
            let _e31 = p_27;
            let _e32 = lacunarity_3;
            p_27 = (_e31 * _e32);
        }
        continuing {
            let _e19 = i_2;
            i_2 = (_e19 + 1i);
        }
    }
    let _e34 = result_7;
    return _e34;
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
    let _e8 = p_29;
    let _e9 = octaves_5;
    let _e10 = lacunarity_5;
    let _e11 = diminish_5;
    let _e12 = mx_fractal2d_noise_float(_e8, _e9, _e10, _e11);
    let _e13 = p_29;
    let _e20 = octaves_5;
    let _e21 = lacunarity_5;
    let _e22 = diminish_5;
    let _e23 = mx_fractal2d_noise_float((_e13 + vec2<f32>(19f, 193f)), _e20, _e21, _e22);
    return vec2<f32>(_e12, _e23);
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
    let _e8 = p_31;
    let _e9 = octaves_7;
    let _e10 = lacunarity_7;
    let _e11 = diminish_7;
    let _e12 = mx_fractal2d_noise_vec3_(_e8, _e9, _e10, _e11);
    c_7 = _e12;
    let _e14 = p_31;
    let _e21 = octaves_7;
    let _e22 = lacunarity_7;
    let _e23 = diminish_7;
    let _e24 = mx_fractal2d_noise_float((_e14 + vec2<f32>(19f, 193f)), _e21, _e22, _e23);
    f_2 = _e24;
    let _e26 = c_7;
    let _e27 = f_2;
    return vec4<f32>(_e26.x, _e26.y, _e26.z, _e27);
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
        let _e14 = i_3;
        let _e15 = octaves_9;
        if !((_e14 < _e15)) {
            break;
        }
        {
            let _e21 = result_8;
            let _e22 = amplitude_2;
            let _e23 = p_33;
            let _e24 = mx_perlin_noise_float_1(_e23);
            result_8 = (_e21 + (_e22 * _e24));
            let _e27 = amplitude_2;
            let _e28 = diminish_9;
            amplitude_2 = (_e27 * _e28);
            let _e30 = p_33;
            let _e31 = lacunarity_9;
            p_33 = (_e30 * _e31);
        }
        continuing {
            let _e18 = i_3;
            i_3 = (_e18 + 1i);
        }
    }
    let _e33 = result_8;
    return _e33;
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
        let _e15 = i_4;
        let _e16 = octaves_11;
        if !((_e15 < _e16)) {
            break;
        }
        {
            let _e22 = result_9;
            let _e23 = amplitude_3;
            let _e24 = p_35;
            let _e25 = mx_perlin_noise_vec3_1(_e24);
            result_9 = (_e22 + (_e23 * _e25));
            let _e28 = amplitude_3;
            let _e29 = diminish_11;
            amplitude_3 = (_e28 * _e29);
            let _e31 = p_35;
            let _e32 = lacunarity_11;
            p_35 = (_e31 * _e32);
        }
        continuing {
            let _e19 = i_4;
            i_4 = (_e19 + 1i);
        }
    }
    let _e34 = result_9;
    return _e34;
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
    let _e8 = p_37;
    let _e9 = octaves_13;
    let _e10 = lacunarity_13;
    let _e11 = diminish_13;
    let _e12 = mx_fractal3d_noise_float(_e8, _e9, _e10, _e11);
    let _e13 = p_37;
    let _e22 = octaves_13;
    let _e23 = lacunarity_13;
    let _e24 = diminish_13;
    let _e25 = mx_fractal3d_noise_float((_e13 + vec3<f32>(19f, 193f, 17f)), _e22, _e23, _e24);
    return vec2<f32>(_e12, _e25);
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
    let _e8 = p_39;
    let _e9 = octaves_15;
    let _e10 = lacunarity_15;
    let _e11 = diminish_15;
    let _e12 = mx_fractal3d_noise_vec3_(_e8, _e9, _e10, _e11);
    c_8 = _e12;
    let _e14 = p_39;
    let _e23 = octaves_15;
    let _e24 = lacunarity_15;
    let _e25 = diminish_15;
    let _e26 = mx_fractal3d_noise_float((_e14 + vec3<f32>(19f, 193f, 17f)), _e23, _e24, _e25);
    f_3 = _e26;
    let _e28 = c_8;
    let _e29 = f_3;
    return vec4<f32>(_e28.x, _e28.y, _e28.z, _e29);
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
    let _e10 = x_29;
    let _e11 = xoff_1;
    let _e13 = y_21;
    let _e14 = yoff_1;
    let _e19 = mx_cell_noise_vec3_1(vec2<f32>(f32((_e10 + _e11)), f32((_e13 + _e14))));
    tmp = _e19;
    let _e21 = tmp;
    let _e23 = tmp;
    off = vec2<f32>(_e21.x, _e23.y);
    let _e27 = off;
    off = (_e27 - vec2(0.5f));
    let _e31 = off;
    let _e32 = jitter_1;
    off = (_e31 * _e32);
    let _e34 = off;
    off = (_e34 + vec2(0.5f));
    let _e38 = x_29;
    let _e40 = y_21;
    let _e43 = off;
    return (vec2<f32>(f32(_e38), f32(_e40)) + _e43);
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
    let _e14 = x_31;
    let _e15 = xoff_3;
    let _e17 = y_23;
    let _e18 = yoff_3;
    let _e20 = z_13;
    let _e21 = zoff_1;
    let _e27 = mx_cell_noise_vec3_2(vec3<f32>(f32((_e14 + _e15)), f32((_e17 + _e18)), f32((_e20 + _e21))));
    off_1 = _e27;
    let _e29 = off_1;
    off_1 = (_e29 - vec3(0.5f));
    let _e33 = off_1;
    let _e34 = jitter_3;
    off_1 = (_e33 * _e34);
    let _e36 = off_1;
    off_1 = (_e36 + vec3(0.5f));
    let _e40 = x_31;
    let _e42 = y_23;
    let _e44 = z_13;
    let _e47 = off_1;
    return (vec3<f32>(f32(_e40), f32(_e42), f32(_e44)) + _e47);
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
    let _e14 = x_33;
    let _e15 = y_25;
    let _e16 = xoff_5;
    let _e17 = yoff_5;
    let _e18 = jitter_5;
    let _e19 = mx_worley_cell_position(_e14, _e15, _e16, _e17, _e18);
    cellpos = _e19;
    let _e21 = cellpos;
    let _e22 = p_41;
    diff = (_e21 - _e22);
    let _e25 = metric_1;
    if (_e25 == 2i) {
        let _e28 = diff;
        let _e31 = diff;
        return (abs(_e28.x) + abs(_e31.y));
    }
    let _e35 = metric_1;
    if (_e35 == 3i) {
        let _e38 = diff;
        let _e41 = diff;
        return max(abs(_e38.x), abs(_e41.y));
    }
    let _e45 = diff;
    let _e46 = diff;
    return dot(_e45, _e46);
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
    let _e18 = x_35;
    let _e19 = y_27;
    let _e20 = z_15;
    let _e21 = xoff_7;
    let _e22 = yoff_7;
    let _e23 = zoff_3;
    let _e24 = jitter_7;
    let _e25 = mx_worley_cell_position_1(_e18, _e19, _e20, _e21, _e22, _e23, _e24);
    cellpos_1 = _e25;
    let _e27 = cellpos_1;
    let _e28 = p_43;
    diff_1 = (_e27 - _e28);
    let _e31 = metric_3;
    if (_e31 == 2i) {
        let _e34 = diff_1;
        let _e37 = diff_1;
        let _e41 = diff_1;
        return ((abs(_e34.x) + abs(_e37.y)) + abs(_e41.z));
    }
    let _e45 = metric_3;
    if (_e45 == 3i) {
        let _e48 = diff_1;
        let _e51 = diff_1;
        let _e55 = diff_1;
        return max(max(abs(_e48.x), abs(_e51.y)), abs(_e55.z));
    }
    let _e59 = diff_1;
    let _e60 = diff_1;
    return dot(_e59, _e60);
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
    let _e11 = p_45;
    let _e15 = mx_floorfrac(_e11.x, (&X_4));
    let _e16 = p_45;
    let _e20 = mx_floorfrac(_e16.y, (&Y_4));
    localpos = vec2<f32>(_e15, _e20);
    loop {
        let _e34 = x_36;
        if !((_e34 <= 1i)) {
            break;
        }
        {
            y_28 = -1i;
            loop {
                let _e44 = y_28;
                if !((_e44 <= 1i)) {
                    break;
                }
                {
                    let _e51 = localpos;
                    let _e52 = x_36;
                    let _e53 = y_28;
                    let _e54 = X_4;
                    let _e55 = Y_4;
                    let _e56 = jitter_9;
                    let _e57 = metric_5;
                    let _e58 = mx_worley_distance(_e51, _e52, _e53, _e54, _e55, _e56, _e57);
                    dist_1 = _e58;
                    let _e60 = x_36;
                    let _e61 = y_28;
                    let _e62 = X_4;
                    let _e63 = Y_4;
                    let _e64 = jitter_9;
                    let _e65 = mx_worley_cell_position(_e60, _e61, _e62, _e63, _e64);
                    let _e66 = localpos;
                    cellpos_2 = (_e65 - _e66);
                    let _e69 = dist_1;
                    let _e70 = sqdist;
                    if (_e69 < _e70) {
                        {
                            let _e72 = dist_1;
                            sqdist = _e72;
                            let _e73 = cellpos_2;
                            minpos = _e73;
                        }
                    }
                }
                continuing {
                    let _e48 = y_28;
                    y_28 = (_e48 + 1i);
                }
            }
        }
        continuing {
            let _e38 = x_36;
            x_36 = (_e38 + 1i);
        }
    }
    let _e74 = style_1;
    if (_e74 == 1i) {
        let _e77 = minpos;
        let _e78 = p_45;
        let _e80 = mx_cell_noise_float_1((_e77 + _e78));
        return _e80;
    } else {
        {
            let _e81 = metric_5;
            if (_e81 == 0i) {
                let _e84 = sqdist;
                sqdist = sqrt(_e84);
            }
            let _e86 = sqdist;
            return _e86;
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
    let _e10 = p_47;
    let _e14 = mx_floorfrac(_e10.x, (&X_5));
    let _e15 = p_47;
    let _e19 = mx_floorfrac(_e15.y, (&Y_5));
    localpos_1 = vec2<f32>(_e14, _e19);
    loop {
        let _e35 = x_37;
        if !((_e35 <= 1i)) {
            break;
        }
        {
            y_29 = -1i;
            loop {
                let _e45 = y_29;
                if !((_e45 <= 1i)) {
                    break;
                }
                {
                    let _e52 = localpos_1;
                    let _e53 = x_37;
                    let _e54 = y_29;
                    let _e55 = X_5;
                    let _e56 = Y_5;
                    let _e57 = jitter_11;
                    let _e58 = metric_7;
                    let _e59 = mx_worley_distance(_e52, _e53, _e54, _e55, _e56, _e57, _e58);
                    dist_2 = _e59;
                    let _e61 = x_37;
                    let _e62 = y_29;
                    let _e63 = X_5;
                    let _e64 = Y_5;
                    let _e65 = jitter_11;
                    let _e66 = mx_worley_cell_position(_e61, _e62, _e63, _e64, _e65);
                    let _e67 = localpos_1;
                    cellpos_3 = (_e66 - _e67);
                    let _e70 = dist_2;
                    let _e71 = sqdist_1;
                    if (_e70 < _e71.x) {
                        {
                            let _e75 = sqdist_1;
                            sqdist_1.y = _e75.x;
                            let _e78 = dist_2;
                            sqdist_1.x = _e78;
                            let _e79 = cellpos_3;
                            minpos_1 = _e79;
                        }
                    } else {
                        let _e80 = dist_2;
                        let _e81 = sqdist_1;
                        if (_e80 < _e81.y) {
                            {
                                let _e85 = dist_2;
                                sqdist_1.y = _e85;
                            }
                        }
                    }
                }
                continuing {
                    let _e49 = y_29;
                    y_29 = (_e49 + 1i);
                }
            }
        }
        continuing {
            let _e39 = x_37;
            x_37 = (_e39 + 1i);
        }
    }
    let _e86 = style_3;
    if (_e86 == 1i) {
        {
            let _e89 = minpos_1;
            let _e90 = p_47;
            let _e92 = mx_cell_noise_vec3_1((_e89 + _e90));
            tmp_1 = _e92;
            let _e94 = tmp_1;
            let _e96 = tmp_1;
            return vec2<f32>(_e94.x, _e96.y);
        }
    } else {
        {
            let _e99 = metric_7;
            if (_e99 == 0i) {
                let _e102 = sqdist_1;
                sqdist_1 = sqrt(_e102);
            }
            let _e104 = sqdist_1;
            return _e104;
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
    let _e10 = p_49;
    let _e14 = mx_floorfrac(_e10.x, (&X_6));
    let _e15 = p_49;
    let _e19 = mx_floorfrac(_e15.y, (&Y_6));
    localpos_2 = vec2<f32>(_e14, _e19);
    loop {
        let _e36 = x_38;
        if !((_e36 <= 1i)) {
            break;
        }
        {
            y_30 = -1i;
            loop {
                let _e46 = y_30;
                if !((_e46 <= 1i)) {
                    break;
                }
                {
                    let _e53 = localpos_2;
                    let _e54 = x_38;
                    let _e55 = y_30;
                    let _e56 = X_6;
                    let _e57 = Y_6;
                    let _e58 = jitter_13;
                    let _e59 = metric_9;
                    let _e60 = mx_worley_distance(_e53, _e54, _e55, _e56, _e57, _e58, _e59);
                    dist_3 = _e60;
                    let _e62 = x_38;
                    let _e63 = y_30;
                    let _e64 = X_6;
                    let _e65 = Y_6;
                    let _e66 = jitter_13;
                    let _e67 = mx_worley_cell_position(_e62, _e63, _e64, _e65, _e66);
                    let _e68 = localpos_2;
                    cellpos_4 = (_e67 - _e68);
                    let _e71 = dist_3;
                    let _e72 = sqdist_2;
                    if (_e71 < _e72.x) {
                        {
                            let _e76 = sqdist_2;
                            sqdist_2.z = _e76.y;
                            let _e79 = sqdist_2;
                            sqdist_2.y = _e79.x;
                            let _e82 = dist_3;
                            sqdist_2.x = _e82;
                            let _e83 = cellpos_4;
                            minpos_2 = _e83;
                        }
                    } else {
                        let _e84 = dist_3;
                        let _e85 = sqdist_2;
                        if (_e84 < _e85.y) {
                            {
                                let _e89 = sqdist_2;
                                sqdist_2.z = _e89.y;
                                let _e92 = dist_3;
                                sqdist_2.y = _e92;
                            }
                        } else {
                            let _e93 = dist_3;
                            let _e94 = sqdist_2;
                            if (_e93 < _e94.z) {
                                {
                                    let _e98 = dist_3;
                                    sqdist_2.z = _e98;
                                }
                            }
                        }
                    }
                }
                continuing {
                    let _e50 = y_30;
                    y_30 = (_e50 + 1i);
                }
            }
        }
        continuing {
            let _e40 = x_38;
            x_38 = (_e40 + 1i);
        }
    }
    let _e99 = style_5;
    if (_e99 == 1i) {
        let _e102 = minpos_2;
        let _e103 = p_49;
        let _e105 = mx_cell_noise_vec3_1((_e102 + _e103));
        return _e105;
    } else {
        {
            let _e106 = metric_9;
            if (_e106 == 0i) {
                let _e109 = sqdist_2;
                sqdist_2 = sqrt(_e109);
            }
            let _e111 = sqdist_2;
            return _e111;
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
    let _e11 = p_51;
    let _e15 = mx_floorfrac(_e11.x, (&X_7));
    let _e16 = p_51;
    let _e20 = mx_floorfrac(_e16.y, (&Y_7));
    let _e21 = p_51;
    let _e25 = mx_floorfrac(_e21.z, (&Z_2));
    localpos_3 = vec3<f32>(_e15, _e20, _e25);
    loop {
        let _e41 = x_39;
        if !((_e41 <= 1i)) {
            break;
        }
        {
            y_31 = -1i;
            loop {
                let _e51 = y_31;
                if !((_e51 <= 1i)) {
                    break;
                }
                {
                    z_16 = -1i;
                    loop {
                        let _e61 = z_16;
                        if !((_e61 <= 1i)) {
                            break;
                        }
                        {
                            let _e68 = localpos_3;
                            let _e69 = x_39;
                            let _e70 = y_31;
                            let _e71 = z_16;
                            let _e72 = X_7;
                            let _e73 = Y_7;
                            let _e74 = Z_2;
                            let _e75 = jitter_15;
                            let _e76 = metric_11;
                            let _e77 = mx_worley_distance_1(_e68, _e69, _e70, _e71, _e72, _e73, _e74, _e75, _e76);
                            dist_4 = _e77;
                            let _e79 = x_39;
                            let _e80 = y_31;
                            let _e81 = z_16;
                            let _e82 = X_7;
                            let _e83 = Y_7;
                            let _e84 = Z_2;
                            let _e85 = jitter_15;
                            let _e86 = mx_worley_cell_position_1(_e79, _e80, _e81, _e82, _e83, _e84, _e85);
                            let _e87 = localpos_3;
                            cellpos_5 = (_e86 - _e87);
                            let _e90 = dist_4;
                            let _e91 = sqdist_3;
                            if (_e90 < _e91) {
                                {
                                    let _e93 = dist_4;
                                    sqdist_3 = _e93;
                                    let _e94 = cellpos_5;
                                    minpos_3 = _e94;
                                }
                            }
                        }
                        continuing {
                            let _e65 = z_16;
                            z_16 = (_e65 + 1i);
                        }
                    }
                }
                continuing {
                    let _e55 = y_31;
                    y_31 = (_e55 + 1i);
                }
            }
        }
        continuing {
            let _e45 = x_39;
            x_39 = (_e45 + 1i);
        }
    }
    let _e95 = style_7;
    if (_e95 == 1i) {
        let _e98 = minpos_3;
        let _e99 = p_51;
        let _e101 = mx_cell_noise_float_2((_e98 + _e99));
        return _e101;
    } else {
        {
            let _e102 = metric_11;
            if (_e102 == 0i) {
                let _e105 = sqdist_3;
                sqdist_3 = sqrt(_e105);
            }
            let _e107 = sqdist_3;
            return _e107;
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
    let _e11 = p_53;
    let _e15 = mx_floorfrac(_e11.x, (&X_8));
    let _e16 = p_53;
    let _e20 = mx_floorfrac(_e16.y, (&Y_8));
    let _e21 = p_53;
    let _e25 = mx_floorfrac(_e21.z, (&Z_3));
    localpos_4 = vec3<f32>(_e15, _e20, _e25);
    loop {
        let _e43 = x_40;
        if !((_e43 <= 1i)) {
            break;
        }
        {
            y_32 = -1i;
            loop {
                let _e53 = y_32;
                if !((_e53 <= 1i)) {
                    break;
                }
                {
                    z_17 = -1i;
                    loop {
                        let _e63 = z_17;
                        if !((_e63 <= 1i)) {
                            break;
                        }
                        {
                            let _e70 = localpos_4;
                            let _e71 = x_40;
                            let _e72 = y_32;
                            let _e73 = z_17;
                            let _e74 = X_8;
                            let _e75 = Y_8;
                            let _e76 = Z_3;
                            let _e77 = jitter_17;
                            let _e78 = metric_13;
                            let _e79 = mx_worley_distance_1(_e70, _e71, _e72, _e73, _e74, _e75, _e76, _e77, _e78);
                            dist_5 = _e79;
                            let _e81 = x_40;
                            let _e82 = y_32;
                            let _e83 = z_17;
                            let _e84 = X_8;
                            let _e85 = Y_8;
                            let _e86 = Z_3;
                            let _e87 = jitter_17;
                            let _e88 = mx_worley_cell_position_1(_e81, _e82, _e83, _e84, _e85, _e86, _e87);
                            let _e89 = localpos_4;
                            cellpos_6 = (_e88 - _e89);
                            let _e92 = dist_5;
                            let _e93 = sqdist_4;
                            if (_e92 < _e93.x) {
                                {
                                    let _e97 = sqdist_4;
                                    sqdist_4.y = _e97.x;
                                    let _e100 = dist_5;
                                    sqdist_4.x = _e100;
                                    let _e101 = cellpos_6;
                                    minpos_4 = _e101;
                                }
                            } else {
                                let _e102 = dist_5;
                                let _e103 = sqdist_4;
                                if (_e102 < _e103.y) {
                                    {
                                        let _e107 = dist_5;
                                        sqdist_4.y = _e107;
                                    }
                                }
                            }
                        }
                        continuing {
                            let _e67 = z_17;
                            z_17 = (_e67 + 1i);
                        }
                    }
                }
                continuing {
                    let _e57 = y_32;
                    y_32 = (_e57 + 1i);
                }
            }
        }
        continuing {
            let _e47 = x_40;
            x_40 = (_e47 + 1i);
        }
    }
    let _e108 = style_9;
    if (_e108 == 1i) {
        {
            let _e111 = minpos_4;
            let _e112 = p_53;
            let _e114 = mx_cell_noise_vec3_2((_e111 + _e112));
            tmp_2 = _e114;
            let _e116 = tmp_2;
            let _e118 = tmp_2;
            return vec2<f32>(_e116.x, _e118.y);
        }
    } else {
        {
            let _e121 = metric_13;
            if (_e121 == 0i) {
                let _e124 = sqdist_4;
                sqdist_4 = sqrt(_e124);
            }
            let _e126 = sqdist_4;
            return _e126;
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
    let _e11 = p_55;
    let _e15 = mx_floorfrac(_e11.x, (&X_9));
    let _e16 = p_55;
    let _e20 = mx_floorfrac(_e16.y, (&Y_9));
    let _e21 = p_55;
    let _e25 = mx_floorfrac(_e21.z, (&Z_4));
    localpos_5 = vec3<f32>(_e15, _e20, _e25);
    loop {
        let _e44 = x_41;
        if !((_e44 <= 1i)) {
            break;
        }
        {
            y_33 = -1i;
            loop {
                let _e54 = y_33;
                if !((_e54 <= 1i)) {
                    break;
                }
                {
                    z_18 = -1i;
                    loop {
                        let _e64 = z_18;
                        if !((_e64 <= 1i)) {
                            break;
                        }
                        {
                            let _e71 = localpos_5;
                            let _e72 = x_41;
                            let _e73 = y_33;
                            let _e74 = z_18;
                            let _e75 = X_9;
                            let _e76 = Y_9;
                            let _e77 = Z_4;
                            let _e78 = jitter_19;
                            let _e79 = metric_15;
                            let _e80 = mx_worley_distance_1(_e71, _e72, _e73, _e74, _e75, _e76, _e77, _e78, _e79);
                            dist_6 = _e80;
                            let _e82 = x_41;
                            let _e83 = y_33;
                            let _e84 = z_18;
                            let _e85 = X_9;
                            let _e86 = Y_9;
                            let _e87 = Z_4;
                            let _e88 = jitter_19;
                            let _e89 = mx_worley_cell_position_1(_e82, _e83, _e84, _e85, _e86, _e87, _e88);
                            let _e90 = localpos_5;
                            cellpos_7 = (_e89 - _e90);
                            let _e93 = dist_6;
                            let _e94 = sqdist_5;
                            if (_e93 < _e94.x) {
                                {
                                    let _e98 = sqdist_5;
                                    sqdist_5.z = _e98.y;
                                    let _e101 = sqdist_5;
                                    sqdist_5.y = _e101.x;
                                    let _e104 = dist_6;
                                    sqdist_5.x = _e104;
                                    let _e105 = cellpos_7;
                                    minpos_5 = _e105;
                                }
                            } else {
                                let _e106 = dist_6;
                                let _e107 = sqdist_5;
                                if (_e106 < _e107.y) {
                                    {
                                        let _e111 = sqdist_5;
                                        sqdist_5.z = _e111.y;
                                        let _e114 = dist_6;
                                        sqdist_5.y = _e114;
                                    }
                                } else {
                                    let _e115 = dist_6;
                                    let _e116 = sqdist_5;
                                    if (_e115 < _e116.z) {
                                        {
                                            let _e120 = dist_6;
                                            sqdist_5.z = _e120;
                                        }
                                    }
                                }
                            }
                        }
                        continuing {
                            let _e68 = z_18;
                            z_18 = (_e68 + 1i);
                        }
                    }
                }
                continuing {
                    let _e58 = y_33;
                    y_33 = (_e58 + 1i);
                }
            }
        }
        continuing {
            let _e48 = x_41;
            x_41 = (_e48 + 1i);
        }
    }
    let _e121 = style_11;
    if (_e121 == 1i) {
        let _e124 = minpos_5;
        let _e125 = p_55;
        let _e127 = mx_cell_noise_vec3_2((_e124 + _e125));
        return _e127;
    } else {
        {
            let _e128 = metric_15;
            if (_e128 == 0i) {
                let _e131 = sqdist_5;
                sqdist_5 = sqrt(_e131);
            }
            let _e133 = sqdist_5;
            return _e133;
        }
    }
}
