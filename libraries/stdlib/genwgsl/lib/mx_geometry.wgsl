fn mx_normals_to_gradient(N: vec3<f32>, Np: vec3<f32>) -> vec3<f32> {
    let d: f32 = dot(N, Np);
    return ((d * N) - Np) / vec3(max(0.00000001f, abs(d)));
}

fn mx_gradient_blend_3_normals(N: vec3<f32>, N1: vec3<f32>, N1_weight: f32, N2: vec3<f32>, N2_weight: f32, N3: vec3<f32>, N3_weight: f32) -> vec3<f32> {
    let w1: f32 = clamp(N1_weight, 0f, 1f);
    let w2: f32 = clamp(N2_weight, 0f, 1f);
    let w3: f32 = clamp(N3_weight, 0f, 1f);
    let g1: vec3<f32> = mx_normals_to_gradient(N, N1);
    let g2: vec3<f32> = mx_normals_to_gradient(N, N2);
    let g3: vec3<f32> = mx_normals_to_gradient(N, N3);
    let gg: vec3<f32> = (w1 * g1) + (w2 * g2) + (w3 * g3);
    return normalize(N - gg);
}

fn mx_axis_rotation_matrix(a: vec3<f32>, r: f32) -> mat3x3<f32> {
    let s: f32 = sin(r);
    let c: f32 = cos(r);
    let omc: f32 = 1f - c;
    return mat3x3<f32>(
        vec3<f32>((a.x * a.x * omc) + c, (a.x * a.y * omc) - (a.z * s), (a.x * a.z * omc) + (a.y * s)),
        vec3<f32>((a.y * a.x * omc) + (a.z * s), (a.y * a.y * omc) + c, (a.y * a.z * omc) - (a.x * s)),
        vec3<f32>((a.z * a.x * omc) - (a.y * s), (a.z * a.y * omc) + (a.x * s), (a.z * a.z * omc) + c)
    );
}
