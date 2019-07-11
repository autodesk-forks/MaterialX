//
// Function to transform uv-coordinates before texture sampling
//
vec2 mx_get_target_uv(vec2 uv)
{
    float scaleX = $scaleX;
    float scaleY = $scaleY;
    float offsetX = $offsetX;
    float offsetY = $offsetY;
    uv = vec2(uv.x * scaleX + offsetX, uv.y * scaleY + offsetY);
    return uv;
}
