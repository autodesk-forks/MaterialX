struct surfaceshader {
    color: vec3<f32>,
    transparency: vec3<f32>,
}

fn mx_surface_unlit(emission: f32, emission_color: vec3<f32>, transmission: f32, transmission_color: vec3<f32>, opacity: f32, result: ptr<function, surfaceshader>) {
    var emission_1: f32;
    var emission_color_1: vec3<f32>;
    var transmission_1: f32;
    var transmission_color_1: vec3<f32>;
    var opacity_1: f32;

    emission_1 = emission;
    emission_color_1 = emission_color;
    transmission_1 = transmission;
    transmission_color_1 = transmission_color;
    opacity_1 = opacity;
    let _e13 = emission_1;
    let _e14 = emission_color_1;
    let _e16 = opacity_1;
    (*result).color = ((_e13 * _e14) * _e16);
    let _e21 = transmission_1;
    let _e22 = transmission_color_1;
    let _e24 = opacity_1;
    (*result).transparency = mix(vec3(1f), (_e21 * _e22), vec3(_e24));
    return;
}