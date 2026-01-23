fn mx_disjointover_color4_(fg: vec4<f32>, bg: vec4<f32>, mixval: f32, result: ptr<function, vec4<f32>>) {
    var fg_1: vec4<f32>;
    var bg_1: vec4<f32>;
    var mixval_1: f32;
    var summedAlpha: f32;
    var x: f32;

    fg_1 = fg;
    bg_1 = bg;
    mixval_1 = mixval;
    let _e8 = fg_1;
    let _e10 = bg_1;
    summedAlpha = (_e8.w + _e10.w);
    let _e14 = summedAlpha;
    if (_e14 <= 1f) {
        {
            let _e17 = (*result);
            let _e19 = fg_1;
            let _e21 = bg_1;
            let _e23 = (_e19.xyz + _e21.xyz);
            (*result).x = _e23.x;
            (*result).y = _e23.y;
            (*result).z = _e23.z;
        }
    } else {
        {
            let _e30 = bg_1;
            if (abs(_e30.w) < 0.00000001f) {
                {
                    let _e35 = (*result);
                    (*result).x = 0f;
                    (*result).y = 0f;
                    (*result).z = 0f;
                }
            } else {
                {
                    let _e43 = fg_1;
                    let _e46 = bg_1;
                    x = ((1f - _e43.w) / _e46.w);
                    let _e50 = (*result);
                    let _e52 = fg_1;
                    let _e54 = bg_1;
                    let _e56 = x;
                    let _e58 = (_e52.xyz + (_e54.xyz * _e56));
                    (*result).x = _e58.x;
                    (*result).y = _e58.y;
                    (*result).z = _e58.z;
                }
            }
        }
    }
    let _e66 = summedAlpha;
    (*result).w = min(_e66, 1f);
    let _e69 = (*result);
    let _e71 = (*result);
    let _e73 = mixval_1;
    let _e76 = mixval_1;
    let _e78 = bg_1;
    let _e81 = ((_e71.xyz * _e73) + ((1f - _e76) * _e78.xyz));
    (*result).x = _e81.x;
    (*result).y = _e81.y;
    (*result).z = _e81.z;
    let _e89 = (*result);
    let _e91 = mixval_1;
    let _e94 = mixval_1;
    let _e96 = bg_1;
    (*result).w = ((_e89.w * _e91) + ((1f - _e94) * _e96.w));
    return;
}