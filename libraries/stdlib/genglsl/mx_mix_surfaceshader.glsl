surfaceshader mx_mix_surfaceshader(surfaceshader shader1, surfaceshader shader2, float value)
{
    surfaceshader returnshader;
    returnshader.color = mix(shader1.color, shader2.color, value);
    returnshader.alpha = mix(shader1.alpha, shader2.alpha, value);
}
