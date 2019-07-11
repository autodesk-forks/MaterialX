void mx_image_color2(sampler2D tex_sampler, int layer, vec2 defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, out vec2 result)
{
    // TODO: Fix handling of addressmode
    if(textureSize(tex_sampler, 0).x > 1)
    {
        vec2 uv = mx_transform_uv(texcoord, vec2(1.0), vec2(0.0));
        result = texture(tex_sampler, uv).rg;
    }
    else
    {
        result = defaultval;
    }
}
