const XYZ_to_RGB: mat3x3<f32> = mat3x3<f32>(vec3<f32>(3.2406f, -0.9689f, 0.0557f), vec3<f32>(-1.5372f, 1.8758f, -0.204f), vec3<f32>(-0.4986f, 0.0415f, 1.057f));

fn mx_blackbody(temperatureKelvin: f32, colorValue: ptr<function, vec3<f32>>) {
    let temp: f32 = clamp(temperatureKelvin, 1667f, 25000f);
    let t: f32 = 1000f / temp;
    let t2: f32 = t * t;
    let t3: f32 = t2 * t;
    
    var xc: f32;
    if (temp < 4000f) {
        xc = (-0.2661239f * t3) - (0.234358f * t2) + (0.8776956f * t) + 0.17991f;
    } else {
        xc = (-3.025847f * t3) + (2.1070378f * t2) + (0.2226347f * t) + 0.24039f;
    }
    
    let xc2: f32 = xc * xc;
    let xc3: f32 = xc2 * xc;
    
    var yc: f32;
    if (temp < 2222f) {
        yc = (-1.1063814f * xc3) - (1.3481102f * xc2) + (2.1855583f * xc) - 0.20219684f;
    } else if (temp < 4000f) {
        yc = (-0.9549476f * xc3) - (1.3741859f * xc2) + (2.09137f * xc) - 0.16748866f;
    } else {
        yc = (3.081758f * xc3) - (5.873387f * xc2) + (3.7511299f * xc) - 0.37001482f;
    }
    
    if (yc <= 0f) {
        (*colorValue) = vec3(1f);
        return;
    }
    
    let XYZ: vec3<f32> = vec3<f32>(xc / yc, 1f, ((1f - xc) - yc) / yc);
    (*colorValue) = max(mx_matrix_mul_4(XYZ_to_RGB, XYZ), vec3(0f));
    return;
}
