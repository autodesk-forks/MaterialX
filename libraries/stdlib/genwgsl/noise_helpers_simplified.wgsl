fn mx_select(b: bool, t: f32, f: f32) -> f32 {
    var local: f32;
    if b {
        local = t;
    } else {
        local = f;
    }
    return local;
}

fn mx_negate_if(val: f32, b_2: bool) -> f32 {
    var local_1: f32;
    if b_2 {
        local_1 = -val;
    } else {
        local_1 = val;
    }
    return local_1;
}

fn mx_floor(x: f32) -> i32 {
    return i32(floor(x));
}

fn mx_floorfrac(x_2: f32, i: ptr<function, i32>) -> f32 {
    (*i) = mx_floor(x_2);
    return x_2 - f32(*i);
}

fn mx_bilerp(v0_: f32, v1_: f32, v2_: f32, v3_: f32, s: f32, t_2: f32) -> f32 {
    let s1_: f32 = 1f - s;
    return ((1f - t_2) * ((v0_ * s1_) + (v1_ * s))) + (t_2 * ((v2_ * s1_) + (v3_ * s)));
}

fn mx_bilerp_1(v0_2: vec3<f32>, v1_2: vec3<f32>, v2_2: vec3<f32>, v3_2: vec3<f32>, s_2: f32, t_4: f32) -> vec3<f32> {
    let s1_1: f32 = 1f - s_2;
    return ((1f - t_4) * ((v0_2 * s1_1) + (v1_2 * s_2))) + (t_4 * ((v2_2 * s1_1) + (v3_2 * s_2)));
}

fn mx_trilerp(v0_4: f32, v1_4: f32, v2_4: f32, v3_4: f32, v4_: f32, v5_: f32, v6_: f32, v7_: f32, s_4: f32, t_6: f32, r: f32) -> f32 {
    let s1_2: f32 = 1f - s_4;
    let t1_: f32 = 1f - t_6;
    let r1_: f32 = 1f - r;
    return (r1_ * ((t1_ * ((v0_4 * s1_2) + (v1_4 * s_4))) + (t_6 * ((v2_4 * s1_2) + (v3_4 * s_4))))) + (r * ((t1_ * ((v4_ * s1_2) + (v5_ * s_4))) + (t_6 * ((v6_ * s1_2) + (v7_ * s_4)))));
}

fn mx_trilerp_1(v0_6: vec3<f32>, v1_6: vec3<f32>, v2_6: vec3<f32>, v3_6: vec3<f32>, v4_2: vec3<f32>, v5_2: vec3<f32>, v6_2: vec3<f32>, v7_2: vec3<f32>, s_6: f32, t_8: f32, r_2: f32) -> vec3<f32> {
    let s1_3: f32 = 1f - s_6;
    let t1_1: f32 = 1f - t_8;
    let r1_1: f32 = 1f - r_2;
    return (r1_1 * ((t1_1 * ((v0_6 * s1_3) + (v1_6 * s_6))) + (t_8 * ((v2_6 * s1_3) + (v3_6 * s_6))))) + (r_2 * ((t1_1 * ((v4_2 * s1_3) + (v5_2 * s_6))) + (t_8 * ((v6_2 * s1_3) + (v7_2 * s_6)))));
}

fn mx_gradient_float(hash: u32, x_4: f32, y: f32) -> f32 {
    let h: u32 = hash & 7u;
    let u: f32 = mx_select(h < 4u, x_4, y);
    let v: f32 = 2f * mx_select(h < 4u, y, x_4);
    return mx_negate_if(u, bool(h & 1u)) + mx_negate_if(v, bool(h & 2u));
}

fn mx_gradient_float_1(hash_2: u32, x_6: f32, y_2: f32, z: f32) -> f32 {
    let h_1: u32 = hash_2 & 15u;
    let u_1: f32 = mx_select(h_1 < 8u, x_6, y_2);
    let v_1: f32 = mx_select(h_1 < 4u, y_2, mx_select((h_1 == 12u) || (h_1 == 14u), x_6, z));
    return mx_negate_if(u_1, bool(h_1 & 1u)) + mx_negate_if(v_1, bool(h_1 & 2u));
}

fn mx_gradient_vec3_(hash_4: vec3<u32>, x_8: f32, y_4: f32) -> vec3<f32> {
    return vec3<f32>(mx_gradient_float(hash_4.x, x_8, y_4), mx_gradient_float(hash_4.y, x_8, y_4), mx_gradient_float(hash_4.z, x_8, y_4));
}

fn mx_gradient_vec3_1(hash_6: vec3<u32>, x_10: f32, y_6: f32, z_2: f32) -> vec3<f32> {
    return vec3<f32>(mx_gradient_float_1(hash_6.x, x_10, y_6, z_2), mx_gradient_float_1(hash_6.y, x_10, y_6, z_2), mx_gradient_float_1(hash_6.z, x_10, y_6, z_2));
}

fn mx_gradient_scale2d(v_2: f32) -> f32 {
    return 0.6616f * v_2;
}

fn mx_gradient_scale3d(v_4: f32) -> f32 {
    return 0.982f * v_4;
}

fn mx_gradient_scale2d_1(v_6: vec3<f32>) -> vec3<f32> {
    return 0.6616f * v_6;
}

fn mx_gradient_scale3d_1(v_8: vec3<f32>) -> vec3<f32> {
    return 0.982f * v_8;
}

fn mx_rotl32_(x_12: u32, k: i32) -> u32 {
    return (x_12 << u32(k)) | (x_12 >> u32(32i - k));
}

fn mx_bjmix(a: ptr<function, u32>, b_4: ptr<function, u32>, c: ptr<function, u32>) {
    (*a) = (*a) - (*c);
    (*a) = (*a) ^ mx_rotl32_(*c, 4i);
    (*c) = (*c) + (*b_4);
    (*b_4) = (*b_4) - (*a);
    (*b_4) = (*b_4) ^ mx_rotl32_(*a, 6i);
    (*a) = (*a) + (*c);
    (*c) = (*c) - (*b_4);
    (*c) = (*c) ^ mx_rotl32_(*b_4, 8i);
    (*b_4) = (*b_4) + (*a);
    (*a) = (*a) - (*c);
    (*a) = (*a) ^ mx_rotl32_(*c, 16i);
    (*c) = (*c) + (*b_4);
    (*b_4) = (*b_4) - (*a);
    (*b_4) = (*b_4) ^ mx_rotl32_(*a, 19i);
    (*a) = (*a) + (*c);
    (*c) = (*c) - (*b_4);
    (*c) = (*c) ^ mx_rotl32_(*b_4, 4i);
    (*b_4) = (*b_4) + (*a);
    return;
}

fn mx_bjfinal(a_1: u32, b_5: u32, c_1: u32) -> u32 {
    var c_2: u32 = c_1;
    c_2 = c_2 ^ b_5;
    c_2 = c_2 - mx_rotl32_(b_5, 14i);
    var a_2: u32 = a_1 ^ c_2;
    a_2 = a_2 - mx_rotl32_(c_2, 11i);
    var b_6: u32 = b_5 ^ a_2;
    b_6 = b_6 - mx_rotl32_(a_2, 25i);
    c_2 = c_2 ^ b_6;
    c_2 = c_2 - mx_rotl32_(b_6, 16i);
    a_2 = a_2 ^ c_2;
    a_2 = a_2 - mx_rotl32_(c_2, 4i);
    b_6 = b_6 ^ a_2;
    b_6 = b_6 - mx_rotl32_(a_2, 14i);
    c_2 = c_2 ^ b_6;
    c_2 = c_2 - mx_rotl32_(b_6, 24i);
    return c_2;
}

fn mx_bits_to_01_(bits: u32) -> f32 {
    return f32(bits) / 4294967300f;
}

fn mx_fade(t_10: f32) -> f32 {
    return (t_10 * t_10) * t_10 * ((t_10 * ((t_10 * 6f) - 15f)) + 10f);
}

fn mx_hash_int(x_14: i32) -> u32 {
    let len: u32 = 1u;
    let seed: u32 = (3735928559u + (len << 2u)) + 13u;
    return mx_bjfinal(seed + u32(x_14), seed, seed);
}

fn mx_hash_int_1(x_16: i32, y_8: i32) -> u32 {
    let len_1: u32 = 2u;
    let init: u32 = (3735928559u + (len_1 << 2u)) + 13u;
    var a_3: u32 = init + u32(x_16);
    var b_7: u32 = init + u32(y_8);
    var c_3: u32 = init;
    return mx_bjfinal(a_3, b_7, c_3);
}

fn mx_hash_int_2(x_18: i32, y_10: i32, z_4: i32) -> u32 {
    let len_2: u32 = 3u;
    let init: u32 = (3735928559u + (len_2 << 2u)) + 13u;
    var a_4: u32 = init + u32(x_18);
    var b_8: u32 = init + u32(y_10);
    var c_4: u32 = init + u32(z_4);
    return mx_bjfinal(a_4, b_8, c_4);
}

fn mx_hash_int_3(x_20: i32, y_12: i32, z_6: i32, xx: i32) -> u32 {
    let len_3: u32 = 4u;
    let init: u32 = (3735928559u + (len_3 << 2u)) + 13u;
    var a_5: u32 = init + u32(x_20);
    var b_9: u32 = init + u32(y_12);
    var c_5: u32 = init + u32(z_6);
    mx_bjmix((&a_5), (&b_9), (&c_5));
    a_5 = a_5 + u32(xx);
    return mx_bjfinal(a_5, b_9, c_5);
}

fn mx_hash_int_4(x_22: i32, y_14: i32, z_8: i32, xx_2: i32, yy: i32) -> u32 {
    let len_4: u32 = 5u;
    let init: u32 = (3735928559u + (len_4 << 2u)) + 13u;
    var a_6: u32 = init + u32(x_22);
    var b_10: u32 = init + u32(y_14);
    var c_6: u32 = init + u32(z_8);
    mx_bjmix((&a_6), (&b_10), (&c_6));
    a_6 = a_6 + u32(xx_2);
    b_10 = b_10 + u32(yy);
    return mx_bjfinal(a_6, b_10, c_6);
}

fn mx_hash_vec3_(x_24: i32, y_16: i32) -> vec3<u32> {
    let h_2: u32 = mx_hash_int_1(x_24, y_16);
    var result: vec3<u32>;
    result.x = h_2 & 255u;
    result.y = (h_2 >> 8u) & 255u;
    result.z = (h_2 >> 16u) & 255u;
    return result;
}

fn mx_hash_vec3_1(x_26: i32, y_18: i32, z_10: i32) -> vec3<u32> {
    let h_3: u32 = mx_hash_int_2(x_26, y_18, z_10);
    var result_1: vec3<u32>;
    result_1.x = h_3 & 255u;
    result_1.y = (h_3 >> 8u) & 255u;
    result_1.z = (h_3 >> 16u) & 255u;
    return result_1;
}

fn mx_perlin_noise_float(p: vec2<f32>) -> f32 {
    var X: i32;
    var Y: i32;
    let fx: f32 = mx_floorfrac(p.x, (&X));
    let fy: f32 = mx_floorfrac(p.y, (&Y));
    let u_2: f32 = mx_fade(fx);
    let v_10: f32 = mx_fade(fy);
    let result_2: f32 = mx_bilerp(
        mx_gradient_float(mx_hash_int_1(X, Y), fx, fy),
        mx_gradient_float(mx_hash_int_1(X + 1i, Y), fx - 1f, fy),
        mx_gradient_float(mx_hash_int_1(X, Y + 1i), fx, fy - 1f),
        mx_gradient_float(mx_hash_int_1(X + 1i, Y + 1i), fx - 1f, fy - 1f),
        u_2, v_10
    );
    return mx_gradient_scale2d(result_2);
}

fn mx_perlin_noise_float_1(p_2: vec3<f32>) -> f32 {
    var X_1: i32;
    var Y_1: i32;
    var Z: i32;
    let fx_1: f32 = mx_floorfrac(p_2.x, (&X_1));
    let fy_1: f32 = mx_floorfrac(p_2.y, (&Y_1));
    let fz: f32 = mx_floorfrac(p_2.z, (&Z));
    let u_3: f32 = mx_fade(fx_1);
    let v_11: f32 = mx_fade(fy_1);
    let w: f32 = mx_fade(fz);
    let result_3: f32 = mx_trilerp(
        mx_gradient_float_1(mx_hash_int_2(X_1, Y_1, Z), fx_1, fy_1, fz),
        mx_gradient_float_1(mx_hash_int_2(X_1 + 1i, Y_1, Z), fx_1 - 1f, fy_1, fz),
        mx_gradient_float_1(mx_hash_int_2(X_1, Y_1 + 1i, Z), fx_1, fy_1 - 1f, fz),
        mx_gradient_float_1(mx_hash_int_2(X_1 + 1i, Y_1 + 1i, Z), fx_1 - 1f, fy_1 - 1f, fz),
        mx_gradient_float_1(mx_hash_int_2(X_1, Y_1, Z + 1i), fx_1, fy_1, fz - 1f),
        mx_gradient_float_1(mx_hash_int_2(X_1 + 1i, Y_1, Z + 1i), fx_1 - 1f, fy_1, fz - 1f),
        mx_gradient_float_1(mx_hash_int_2(X_1, Y_1 + 1i, Z + 1i), fx_1, fy_1 - 1f, fz - 1f),
        mx_gradient_float_1(mx_hash_int_2(X_1 + 1i, Y_1 + 1i, Z + 1i), fx_1 - 1f, fy_1 - 1f, fz - 1f),
        u_3, v_11, w
    );
    return mx_gradient_scale3d(result_3);
}

fn mx_perlin_noise_vec3_(p_4: vec2<f32>) -> vec3<f32> {
    var X_2: i32;
    var Y_2: i32;
    let fx_2: f32 = mx_floorfrac(p_4.x, (&X_2));
    let fy_2: f32 = mx_floorfrac(p_4.y, (&Y_2));
    let u_4: f32 = mx_fade(fx_2);
    let v_12: f32 = mx_fade(fy_2);
    let result_4: vec3<f32> = mx_bilerp_1(
        mx_gradient_vec3_(mx_hash_vec3_(X_2, Y_2), fx_2, fy_2),
        mx_gradient_vec3_(mx_hash_vec3_(X_2 + 1i, Y_2), fx_2 - 1f, fy_2),
        mx_gradient_vec3_(mx_hash_vec3_(X_2, Y_2 + 1i), fx_2, fy_2 - 1f),
        mx_gradient_vec3_(mx_hash_vec3_(X_2 + 1i, Y_2 + 1i), fx_2 - 1f, fy_2 - 1f),
        u_4, v_12
    );
    return mx_gradient_scale2d_1(result_4);
}

fn mx_perlin_noise_vec3_1(p_6: vec3<f32>) -> vec3<f32> {
    var X_3: i32;
    var Y_3: i32;
    var Z_1: i32;
    let fx_3: f32 = mx_floorfrac(p_6.x, (&X_3));
    let fy_3: f32 = mx_floorfrac(p_6.y, (&Y_3));
    let fz_1: f32 = mx_floorfrac(p_6.z, (&Z_1));
    let u_5: f32 = mx_fade(fx_3);
    let v_13: f32 = mx_fade(fy_3);
    let w_1: f32 = mx_fade(fz_1);
    let result_5: vec3<f32> = mx_trilerp_1(
        mx_gradient_vec3_1(mx_hash_vec3_1(X_3, Y_3, Z_1), fx_3, fy_3, fz_1),
        mx_gradient_vec3_1(mx_hash_vec3_1(X_3 + 1i, Y_3, Z_1), fx_3 - 1f, fy_3, fz_1),
        mx_gradient_vec3_1(mx_hash_vec3_1(X_3, Y_3 + 1i, Z_1), fx_3, fy_3 - 1f, fz_1),
        mx_gradient_vec3_1(mx_hash_vec3_1(X_3 + 1i, Y_3 + 1i, Z_1), fx_3 - 1f, fy_3 - 1f, fz_1),
        mx_gradient_vec3_1(mx_hash_vec3_1(X_3, Y_3, Z_1 + 1i), fx_3, fy_3, fz_1 - 1f),
        mx_gradient_vec3_1(mx_hash_vec3_1(X_3 + 1i, Y_3, Z_1 + 1i), fx_3 - 1f, fy_3, fz_1 - 1f),
        mx_gradient_vec3_1(mx_hash_vec3_1(X_3, Y_3 + 1i, Z_1 + 1i), fx_3, fy_3 - 1f, fz_1 - 1f),
        mx_gradient_vec3_1(mx_hash_vec3_1(X_3 + 1i, Y_3 + 1i, Z_1 + 1i), fx_3 - 1f, fy_3 - 1f, fz_1 - 1f),
        u_5, v_13, w_1
    );
    return mx_gradient_scale3d_1(result_5);
}


