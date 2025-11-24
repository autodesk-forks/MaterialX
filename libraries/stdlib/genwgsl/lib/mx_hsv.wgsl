fn mx_hsvtorgb(hsv: vec3<f32>) -> vec3<f32> {
    var h: f32 = hsv.x;
    let s: f32 = hsv.y;
    let v: f32 = hsv.z;
    if (s < 0.0001f) {
        return vec3<f32>(v, v, v);
    } else {
        h = 6f * (h - floor(h));
        let hi: i32 = i32(trunc(h));
        let f: f32 = h - f32(hi);
        let p: f32 = v * (1f - s);
        let q: f32 = v * (1f - (s * f));
        let t: f32 = v * (1f - (s * (1f - f)));
        if (hi == 0i) {
            return vec3<f32>(v, t, p);
        } else if (hi == 1i) {
            return vec3<f32>(q, v, p);
        } else if (hi == 2i) {
            return vec3<f32>(p, v, t);
        } else if (hi == 3i) {
            return vec3<f32>(p, q, v);
        } else if (hi == 4i) {
            return vec3<f32>(t, p, v);
        }
        return vec3<f32>(v, p, q);
    }
}

fn mx_rgbtohsv(c: vec3<f32>) -> vec3<f32> {
    let r: f32 = c.x;
    let g: f32 = c.y;
    let b: f32 = c.z;
    let mincomp: f32 = min(r, min(g, b));
    let maxcomp: f32 = max(r, max(g, b));
    let delta: f32 = maxcomp - mincomp;
    let v: f32 = maxcomp;
    var s: f32;
    if (maxcomp > 0f) {
        s = delta / maxcomp;
    } else {
        s = 0f;
    }
    var h: f32;
    if (s <= 0f) {
        h = 0f;
    } else {
        if (r >= maxcomp) {
            h = (g - b) / delta;
        } else if (g >= maxcomp) {
            h = 2f + ((b - r) / delta);
        } else {
            h = 4f + ((r - g) / delta);
        }
        h = h * 0.16666667f;
        if (h < 0f) {
            h = h + 1f;
        }
    }
    return vec3<f32>(h, s, v);
}
