
#define LENGTH_UNITS 10
// nanometer, millimeter, micron, centimeter, foot, yard, kilometer, inch, mile, meter
uniform float u_length_unit_scales[LENGTH_UNITS] = float[](
    1000000000.0,
    1000.0,
    1000000.0,
    100.0,
    3.28099990,
    1.09361303,
    0.00100000005,
    39.3699989,
    0.000621000014,
    1.0
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