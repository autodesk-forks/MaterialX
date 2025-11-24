fn mx_heighttonormal_vector3_(height: f32, scale: f32, texcoord: vec2<f32>, result: ptr<function, vec3<f32>>) {
    let SOBEL_SCALE_FACTOR: f32 = 0.0625f;
    let dHdS: vec2<f32> = (vec2<f32>(dpdx(height), dpdy(height)) * scale) * SOBEL_SCALE_FACTOR;
    let dUdS: vec2<f32> = vec2<f32>(dpdx(texcoord.x), dpdy(texcoord.x));
    let dVdS: vec2<f32> = vec2<f32>(dpdx(texcoord.y), dpdy(texcoord.y));
    let tangent: vec3<f32> = vec3<f32>(dUdS.x, dVdS.x, dHdS.x);
    let bitangent: vec3<f32> = vec3<f32>(dUdS.y, dVdS.y, dHdS.y);
    var n: vec3<f32> = cross(tangent, bitangent);
    if (dot(n, n) < 0.0000000000000001f) {
        n = vec3<f32>(0f, 0f, 1f);
    } else {
        if (n.z < 0f) {
            n = n * -1f;
        }
    }
    (*result) = (normalize(n) * 0.5f) + vec3(0.5f);
    return;
}
