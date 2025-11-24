fn mx_cellnoise3d_float(position: vec3<f32>, result_10: ptr<function, f32>) {
    (*result_10) = (mx_cell_noise_float_2(position_1));
    return;
}
