fn mx_artistic_ior(reflectivity: vec3<f32>, edge_color: vec3<f32>, ior: ptr<function, vec3<f32>>, extinction: ptr<function, vec3<f32>>) {
    let r: vec3<f32> = clamp(reflectivity, vec3(0f), vec3(0.99f));
    let r_sqrt: vec3<f32> = sqrt(r);
    let n_min: vec3<f32> = (vec3(1f) - r) / (vec3(1f) + r);
    let n_max: vec3<f32> = (vec3(1f) + r_sqrt) / (vec3(1f) - r_sqrt);
    (*ior) = mix(n_max, n_min, edge_color);
    let np1: vec3<f32> = (*ior) + vec3(1f);
    let nm1: vec3<f32> = (*ior) - vec3(1f);
    let k2: vec3<f32> = max(((np1 * np1) * r - (nm1 * nm1)) / (vec3(1f) - r), vec3(0f));
    (*extinction) = sqrt(k2);
    return;
}
