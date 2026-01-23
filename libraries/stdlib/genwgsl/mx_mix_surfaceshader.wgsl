struct surfaceshader {
    color: vec3<f32>,
    transparency: vec3<f32>,
}

fn mx_mix_surfaceshader(fg: surfaceshader, bg: surfaceshader, w: f32, returnshader: ptr<function, surfaceshader>) {
    var fg_1: surfaceshader;
    var bg_1: surfaceshader;
    var w_1: f32;

    fg_1 = fg;
    bg_1 = bg;
    w_1 = w;
    let _e9 = bg_1;
    let _e11 = fg_1;
    let _e13 = w_1;
    (*returnshader).color = mix(_e9.color, _e11.color, vec3(_e13));
    let _e17 = bg_1;
    let _e19 = fg_1;
    let _e21 = w_1;
    (*returnshader).transparency = mix(_e17.transparency, _e19.transparency, vec3(_e21));
    return;
}