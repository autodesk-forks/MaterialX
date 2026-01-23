struct displacementshader {
    offset: vec3<f32>,
    scale: f32,
}

fn mx_displacement_float(disp: f32, scale: f32, result: ptr<function, displacementshader>) {
    var disp_1: f32;
    var scale_1: f32;

    disp_1 = disp;
    scale_1 = scale;
    let _e7 = disp_1;
    (*result).offset = vec3(_e7);
    let _e10 = scale_1;
    (*result).scale = _e10;
    return;
}