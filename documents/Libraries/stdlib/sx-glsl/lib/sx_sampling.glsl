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
// The sampling grid is 3x3 in size and is referenced base
// on the array index order shown.
// ----+-----+----
//  0  |  1  | 2
// ----+-----+----
//  3  |  4  | 5
// ----+-----+----
//  6  |  7  | 8
// ----+-----+----
//
vec3 sx_normal_from_samples_sobel(float S[9], float _scale)
{
   float nx = S[0] - S[2] + (2.0*S[3]) - (2.0*S[5]) + S[6] - S[8];
   float ny = S[0] + (2.0*S[1]) + S[2] - S[6] - (2.0*S[7]) - S[8];
   float nz = _scale * sqrt(1.0 - nx*nx - ny*ny);
   vec3 norm = normalize(vec3(nx, ny, nz));
   return (norm + 1.0) * 0.5;
};
