float unit_ratio(int unit_from, int unit_to) {
    return (u_length_unit_scales[unit_from] / u_length_unit_scales[unit_to]);
}

void mx_length_unit_float(float _in , int _unit_from, int _unit_to, out float result) {
    result = _in * unit_ratio(_unit_from, _unit_to);
}

void mx_length_unit_vector2(vec2 _in , int _unit_from, int _unit_to, out vec2 result) {
    result = _in * unit_ratio(_unit_from, _unit_to);
}

void mx_length_unit_vector3(vec3 _in , int _unit_from, int _unit_to, out vec3 result) {
    result = _in * unit_ratio(_unit_from, _unit_to);
}

void mx_length_unit_vector4(vec4 _in , int _unit_from, int _unit_to, out vec4 result) {
    result = _in * unit_ratio(_unit_from, _unit_to);
}