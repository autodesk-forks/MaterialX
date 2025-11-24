fn mx_rotate_vector2_(_in: vec2<f32>, amount: f32, result: ptr<function, vec2<f32>>) {
    let rotationRadians: f32 = radians(amount);
    let sa: f32 = sin(rotationRadians);
    let ca: f32 = cos(rotationRadians);
    (*result) = vec2<f32>((ca * _in.x) + (sa * _in.y), (-sa * _in.x) + (ca * _in.y));
    return;
}
