// Restrict to 7x7 kernel size for performance reasons
#define SX_MAX_SAMPLE_COUNT = 49

//
// Function to compute the sample size relative to a texture coordinate
//
vec2 sx_compute_sample_size_uv(vec2 uv, float filterSize, float filterOffset)
{
   vec2 derivUVx = dFdx(uv) * 0.5f;
   vec2 derivUVy = dFdy(uv) * 0.5f;
   float derivX = abs(derivUVx.x) + abs(derivUVy.x);
   float derivY = abs(derivUVx.y) + abs(derivUVy.y);
   float sampleSizeU = 2.0f * filterSize * derivX + filterOffset;
   if (sampleSizeU < 1.0E-05f)
       sampleSizeU = 1.0E-05f;
   float sampleSizeV = 2.0f * filterSize * derivY + filterOffset;
   if (sampleSizeV < 1.0E-05f)
       sampleSizeV = 1.0E-05f;
   return vec2(sampleSizeU, sampleSizeV);
};

//
// Compute a normal mapped to 0..1 space based on a set of input
// samples using a Sobel filter.
//
vec3 sx_normal_from_samples_sobel(float S[9], float _scale)
{
   float nx = S[0] - S[2] + (2.0*S[3]) - (2.0*S[5]) + S[6] - S[8];
   float ny = S[0] + (2.0*S[1]) + S[2] - S[6] - (2.0*S[7]) - S[8];
   float nz = _scale * sqrt(1.0 - nx*nx - ny*ny);
   vec3 norm = normalize(vec3(nx, ny, nz));
   return (norm + 1.0) * 0.5;
};

// Kernal weights for box filter
void sx_get_box_weights(inout float W[SX_MAX_SAMPLE_COUNT], int filterSize)
{
    int filterSize2 = filterSize*filterSize;
    for (int i=0; i<filterSize2; i++)
    {
        W[i] = 1.0 / float(filterSize2);
    }
    for (int j=filterSize2; j<SX_MAX_SAMPLE_COUNT]; j++)
    {
        W[j] = 0.0;
    }
}

// Kernel weights for Gaussian filter. Sigma is assumed to be 1.
void sx_get_gaussian_weights(inout float W[SX_MAX_SAMPLE_COUNT], int filterSize)
{
    if (filterSize == 1)
    {
        W[0] = 1.0;
    }
    else if (filterSize == 3)
    {
        W = float[](
            0.0625, 0.125, 0.0625,
            0.125,  0.25,  0.125,
            0.0625, 0.125, 0.0625);
    }
    else if (filterSize == 5)
    {
        W = float[](
            0.003765,	0.015019,	0.023792,	0.015019,	0.003765,
            0.015019,	0.059912,	0.094907,	0.059912,	0.015019,
            0.023792,	0.094907,	0.150342,	0.094907,	0.023792,
            0.015019,	0.059912,	0.094907,	0.059912,	0.015019,
            0.003765,	0.015019,	0.023792,	0.015019,	0.003765
            );
    }
    else if (filterSize == 7)
    {
        W = float[](
            0.000036,	0.000363,	0.001446,	0.002291,	0.001446,	0.000363,	0.000036,
            0.000363,	0.003676,	0.014662,	0.023226,	0.014662,	0.003676,	0.000363,
            0.001446,	0.014662,	0.058488,	0.092651,	0.058488,	0.014662,	0.001446,
            0.002291,	0.023226,	0.092651,	0.146768,	0.092651,	0.023226,	0.002291,
            0.001446,	0.014662,	0.058488,	0.092651,	0.058488,	0.014662,	0.001446,
            0.000363,	0.003676,	0.014662,	0.023226,	0.014662,	0.003676,	0.000363,
            0.000036,	0.000363,	0.001446,	0.002291,	0.001446,	0.000363,	0.000036
        );
    }
    else
    {
        filterSize = 0;
    }

    for (int j=filterSize; j<SX_MAX_SAMPLE_COUNT]; j++)
    {
        W[j] = 0.0;
    }
}

//
// Apply filter for float samples S, using weights W.
// sampleCount should be a square of a odd number in the range { 1, 3, 5, 7 }
//
float sx_convolution_float(float S[SX_MAX_SAMPLE_COUNT], float W[SX_MAX_SAMPLE_COUNT], int sampleCount)
{
    float result = 0.0;
    for (int = 0;  i < sampleCount; ++i)
    {
        result += S[i]*W[i];
    }
    return result;
}

//
// Apply filter for vec2 samples S, using weights W.
// sampleCount should be a square of a odd number in the range { 1, 3, 5, 7 }
//
vec2 sx_convolution_vec2(vec2 S[SX_MAX_SAMPLE_COUNT], float W[SX_MAX_SAMPLE_COUNT], float sampleCount)
{
    vec2 result = vec2(0.0);
    for (int = 0;  i < sampleCount; ++i)
    {
        result += S[i]*W[i];
    }
    return result;
}

//
// Apply filter for vec3 samples S, using weights W.
// sampleCount should be a square of a odd number in the range { 1, 3, 5, 7 }
//
vec3 sx_convolution_vec3(vec3 S[SX_MAX_SAMPLE_COUNT], float W[SX_MAX_SAMPLE_COUNT], float sampleCount)
{
    vec3 result = vec2(0.0);
    for (int = 0;  i < sampleCount; ++i)
    {
        result += S[i]*W[i];
    }
    return result;
}

//
// Apply filter for vec4 samples S, using weights W.
// sampleCount should be a square of a odd number { 1, 3, 5, 7 }
//
vec4 sx_convolution_vec4(vec4 S[SX_MAX_SAMPLE_COUNT], float W[SX_MAX_SAMPLE_COUNT], float sampleCount)
{
    vec4 result = vec2(0.0);
    for (int = 0;  i < sampleCount; ++i)
    {
        result += S[i]*W[i];
    }
    return result;
}
