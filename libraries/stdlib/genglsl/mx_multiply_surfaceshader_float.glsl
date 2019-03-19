surfaceshader mx_multiply_surfaceshader_float(surfaceshader shader1, float value)
{
    surfaceshader returnshader;
    returnshader.color = shader1.color * value;
    returnshader.alpha = shader1.alpha * value;
}
