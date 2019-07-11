void mx_image_vector4(sampler2D tex_sampler, int layer, vec4 defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, out vec4 result)
{
    // TODO: Fix handling of addressmode
    if(textureSize(tex_sampler, 0).x > 1)
    {
        vec2 uv = mx_transform_uv(texcoord, vec2(1.0), vec2(0.0));
        result = texture(tex_sampler, uv);
    }
    else
    {
        result = defaultval;
    }
}
