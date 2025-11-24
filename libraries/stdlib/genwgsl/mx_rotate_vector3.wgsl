fn mx_rotationMatrix(axis: vec3<f32>, angle: f32) -> mat4x4<f32> {
    let normalized_axis: vec3<f32> = normalize(axis);
    let s: f32 = sin(angle);
    let c: f32 = cos(angle);
    let oc: f32 = 1f - c;
    return mat4x4<f32>(
        vec4<f32>((oc * normalized_axis.x * normalized_axis.x) + c, (oc * normalized_axis.x * normalized_axis.y) - (normalized_axis.z * s), (oc * normalized_axis.z * normalized_axis.x) + (normalized_axis.y * s), 0f),
        vec4<f32>((oc * normalized_axis.x * normalized_axis.y) + (normalized_axis.z * s), (oc * normalized_axis.y * normalized_axis.y) + c, (oc * normalized_axis.y * normalized_axis.z) - (normalized_axis.x * s), 0f),
        vec4<f32>((oc * normalized_axis.z * normalized_axis.x) - (normalized_axis.y * s), (oc * normalized_axis.y * normalized_axis.z) + (normalized_axis.x * s), (oc * normalized_axis.z * normalized_axis.z) + c, 0f),
        vec4<f32>(0f, 0f, 0f, 1f)
    );
}

fn mx_rotate_vector3_(_in: vec3<f32>, amount: f32, axis_2: vec3<f32>, result: ptr<function, vec3<f32>>) {
    let rotationRadians: f32 = radians(amount);
    let m_12: mat4x4<f32> = mx_rotationMatrix(axis_2, rotationRadians);
    (*result) = mx_matrix_mul_5(m_12, vec4<f32>(_in.x, _in.y, _in.z, 1f)).xyz;
    return;
}
