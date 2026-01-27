#include "../../stdlib/genglsl/lib/mx_noise.glsl"
// hash noise is similar to cell noise without floor operation on input.  
void mx_hashnoise2d_vector3(vec2 texcoord, out vec3 result)
{
    int ix = mx_float_bits_to_int(texcoord.x);
    int iy = mx_float_bits_to_int(texcoord.y);
    result = vec3(
            mx_bits_to_01(mx_hash_int(ix, iy, 0)),
            mx_bits_to_01(mx_hash_int(ix, iy, 1)),
            mx_bits_to_01(mx_hash_int(ix, iy, 2))
    );
}
