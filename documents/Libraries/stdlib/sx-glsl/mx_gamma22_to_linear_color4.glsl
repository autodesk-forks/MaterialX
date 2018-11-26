void mx_gamma22_to_linear_color4(vec4 _in, out vec4 result)
{
    vec4 gamma = vec4(2.200000047683716, 2.200000047683716, 2.200000047683716, 1.);
    result = pow( max( vec4(0., 0., 0., 0.), _in ), gamma );
}
