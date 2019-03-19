surfaceshader mx_add_surfaceshader(surfaceshader shader1, surfaceshader shader2)
{
    surfaceshader returnshader;
    returnshader.color = shader1.color + shader2.color;
    returnshader.alpha = shader1.alpha + shader2.alpha;    
}
