// Main function: mx_hsvtorgb_color3
// Included from lib/mx_hsv.glsl (WGSL)
fn mx_hsvtorgb(hsv: vec3<f32>) -> vec3<f32> {
    var hsv_1: vec3<f32>;
    var h: f32;
    var s: f32;
    var v: f32;
    var hi: i32;
    var f: f32;
    var p: f32;
    var q: f32;
    var t: f32;

    hsv_1 = hsv;
    let _e3 = hsv_1;
    h = _e3.x;
    let _e6 = hsv_1;
    s = _e6.y;
    let _e9 = hsv_1;
    v = _e9.z;
    let _e12 = s;
    if (_e12 < 0.0001f) {
        {
            let _e15 = v;
            let _e16 = v;
            let _e17 = v;
            return vec3<f32>(_e15, _e16, _e17);
        }
    } else {
        {
            let _e20 = h;
            let _e21 = h;
            h = (6f * (_e20 - floor(_e21)));
            let _e25 = h;
            hi = i32(trunc(_e25));
            let _e29 = h;
            let _e30 = hi;
            f = (_e29 - f32(_e30));
            let _e34 = v;
            let _e36 = s;
            p = (_e34 * (1f - _e36));
            let _e40 = v;
            let _e42 = s;
            let _e43 = f;
            q = (_e40 * (1f - (_e42 * _e43)));
            let _e48 = v;
            let _e50 = s;
            let _e52 = f;
            t = (_e48 * (1f - (_e50 * (1f - _e52))));
            let _e58 = hi;
            if (_e58 == 0i) {
                let _e61 = v;
                let _e62 = t;
                let _e63 = p;
                return vec3<f32>(_e61, _e62, _e63);
            } else {
                let _e65 = hi;
                if (_e65 == 1i) {
                    let _e68 = q;
                    let _e69 = v;
                    let _e70 = p;
                    return vec3<f32>(_e68, _e69, _e70);
                } else {
                    let _e72 = hi;
                    if (_e72 == 2i) {
                        let _e75 = p;
                        let _e76 = v;
                        let _e77 = t;
                        return vec3<f32>(_e75, _e76, _e77);
                    } else {
                        let _e79 = hi;
                        if (_e79 == 3i) {
                            let _e82 = p;
                            let _e83 = q;
                            let _e84 = v;
                            return vec3<f32>(_e82, _e83, _e84);
                        } else {
                            let _e86 = hi;
                            if (_e86 == 4i) {
                                let _e89 = t;
                                let _e90 = p;
                                let _e91 = v;
                                return vec3<f32>(_e89, _e90, _e91);
                            }
                        }
                    }
                }
            }
            let _e93 = v;
            let _e94 = p;
            let _e95 = q;
            return vec3<f32>(_e93, _e94, _e95);
        }
    }
}

fn mx_rgbtohsv(c: vec3<f32>) -> vec3<f32> {
    var c_1: vec3<f32>;
    var r: f32;
    var g: f32;
    var b: f32;
    var mincomp: f32;
    var maxcomp: f32;
    var delta: f32;
    var h_1: f32;
    var s_1: f32;
    var v_1: f32;

    c_1 = c;
    let _e3 = c_1;
    r = _e3.x;
    let _e6 = c_1;
    g = _e6.y;
    let _e9 = c_1;
    b = _e9.z;
    let _e12 = r;
    let _e13 = g;
    let _e14 = b;
    mincomp = min(_e12, min(_e13, _e14));
    let _e18 = r;
    let _e19 = g;
    let _e20 = b;
    maxcomp = max(_e18, max(_e19, _e20));
    let _e24 = maxcomp;
    let _e25 = mincomp;
    delta = (_e24 - _e25);
    let _e31 = maxcomp;
    v_1 = _e31;
    let _e32 = maxcomp;
    if (_e32 > 0f) {
        let _e35 = delta;
        let _e36 = maxcomp;
        s_1 = (_e35 / _e36);
    } else {
        s_1 = 0f;
    }
    let _e39 = s_1;
    if (_e39 <= 0f) {
        h_1 = 0f;
    } else {
        {
            let _e43 = r;
            let _e44 = maxcomp;
            if (_e43 >= _e44) {
                let _e46 = g;
                let _e47 = b;
                let _e49 = delta;
                h_1 = ((_e46 - _e47) / _e49);
            } else {
                let _e51 = g;
                let _e52 = maxcomp;
                if (_e51 >= _e52) {
                    let _e55 = b;
                    let _e56 = r;
                    let _e58 = delta;
                    h_1 = (2f + ((_e55 - _e56) / _e58));
                } else {
                    let _e62 = r;
                    let _e63 = g;
                    let _e65 = delta;
                    h_1 = (4f + ((_e62 - _e63) / _e65));
                }
            }
            let _e68 = h_1;
            h_1 = (_e68 * 0.16666667f);
            let _e73 = h_1;
            if (_e73 < 0f) {
                let _e76 = h_1;
                h_1 = (_e76 + 1f);
            }
        }
    }
    let _e79 = h_1;
    let _e80 = s_1;
    let _e81 = v_1;
    return vec3<f32>(_e79, _e80, _e81);
}

// Main function from mx_hsvtorgb_color3.glsl
fn mx_hsvtorgb_color3_(_in: vec3<f32>, result: ptr<function, vec3<f32>>) {
    var _in_1: vec3<f32>;

    _in_1 = _in;
    let _e4 = _in_1;
    let _e5 = mx_hsvtorgb(_e4);
    (*result) = _e5;
    return;
}