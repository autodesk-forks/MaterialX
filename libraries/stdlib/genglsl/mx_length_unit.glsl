
#define LENGTH_UNITS 10
uniform float u_length_unit_scales[LENGTH_UNITS] = float[](
    1000000000.0, 
    1000000.0,
    1000.0,
    100.0,
    1.0,
    0.001,
    3.281,
    39.37,
    1.093613,
    0.000621
);

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