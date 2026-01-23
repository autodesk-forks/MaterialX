fn mx_heighttonormal_vector3_(height: f32, scale: f32, texcoord: vec2<f32>, result: ptr<function, vec3<f32>>) {
    var height_1: f32;
    var scale_1: f32;
    var texcoord_1: vec2<f32>;
    var SOBEL_SCALE_FACTOR: f32 = 0.0625f;
    var dHdS: vec2<f32>;
    var dUdS: vec2<f32>;
    var dVdS: vec2<f32>;
    var tangent: vec3<f32>;
    var bitangent: vec3<f32>;
    var n: vec3<f32>;

    height_1 = height;
    scale_1 = scale;
    texcoord_1 = texcoord;
    let _e12 = height_1;
    let _e13 = dpdx(_e12);
    let _e14 = height_1;
    let _e15 = dpdy(_e14);
    let _e17 = scale_1;
    let _e19 = SOBEL_SCALE_FACTOR;
    dHdS = ((vec2<f32>(_e13, _e15) * _e17) * _e19);
    let _e22 = texcoord_1;
    let _e24 = dpdx(_e22.x);
    let _e25 = texcoord_1;
    let _e27 = dpdy(_e25.x);
    dUdS = vec2<f32>(_e24, _e27);
    let _e30 = texcoord_1;
    let _e32 = dpdx(_e30.y);
    let _e33 = texcoord_1;
    let _e35 = dpdy(_e33.y);
    dVdS = vec2<f32>(_e32, _e35);
    let _e38 = dUdS;
    let _e40 = dVdS;
    let _e42 = dHdS;
    tangent = vec3<f32>(_e38.x, _e40.x, _e42.x);
    let _e46 = dUdS;
    let _e48 = dVdS;
    let _e50 = dHdS;
    bitangent = vec3<f32>(_e46.y, _e48.y, _e50.y);
    let _e54 = tangent;
    let _e55 = bitangent;
    n = cross(_e54, _e55);
    let _e58 = n;
    let _e59 = n;
    if (dot(_e58, _e59) < 0.0000000000000001f) {
        {
            n = vec3<f32>(0f, 0f, 1f);
        }
    } else {
        let _e72 = n;
        if (_e72.z < 0f) {
            {
                let _e76 = n;
                n = (_e76 * -1f);
            }
        }
    }
    let _e80 = n;
    (*result) = ((normalize(_e80) * 0.5f) + vec3(0.5f));
    return;
}