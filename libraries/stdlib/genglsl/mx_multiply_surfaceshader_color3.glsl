surfaceshader mx_multiply_surfaceshader_color3(surfaceshader shader1, vec3 value)
{
    surfaceshader returnshader;
    returnshader.color = shader1.color * value;
    returnshader.alpha = shader1.alpha;
}
