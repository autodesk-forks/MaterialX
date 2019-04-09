#define SHEEN_ALBEDO_TABLE_SIZE 16

// LUT for sheen directional albedo. 
// A 2D table parameterized with 'cosTheta' (cosine of angle to normal) on x-axis and 'roughness' on y-axis.
uniform float u_sheenAlbedo[SHEEN_ALBEDO_TABLE_SIZE*SHEEN_ALBEDO_TABLE_SIZE] = float[](
    2.76647, 0.171509, 0.00618558, 9.84063e-05, 6.19921e-07, 1.39083e-09, 8.60341e-13, 1.17699e-16, 5.52024e-21, 1.09804e-26, 2.47167e-33, 4.20165e-41, 0, 0, 0, 0,
    2.34342, 0.953678, 0.398052, 0.157998, 0.0585023, 0.0197559, 0.00593582, 0.00154916, 0.000345975, 6.1129e-05, 8.45661e-06, 8.16544e-07, 5.10183e-08, 1.46869e-09, 1.20029e-11, 4.83963e-15,
    1.75273, 0.999625, 0.59552, 0.353428, 0.206245, 0.116969, 0.0637988, 0.0331292, 0.0162756, 0.00738193, 0.00304303, 0.00110141, 0.000335789, 7.8093e-05, 1.14029e-05, 5.48291e-07,
    1.40076, 0.92493, 0.637354, 0.441978, 0.305191, 0.208143, 0.139327, 0.0909457, 0.0576628, 0.0350688, 0.0202479, 0.0109112, 0.00535637, 0.00228022, 0.000759862, 0.000144217,
    1.17609, 0.843426, 0.6295, 0.474382, 0.357669, 0.268079, 0.198835, 0.145234, 0.104175, 0.07279, 0.0491926, 0.0318195, 0.0193899, 0.010801, 0.00517464, 0.00179383,
    1.02304, 0.773515, 0.607168, 0.481838, 0.383447, 0.304344, 0.240044, 0.187408, 0.144514, 0.109423, 0.0809322, 0.0580511, 0.0399357, 0.0258247, 0.0151074, 0.00726248,
    0.913656, 0.71651, 0.582186, 0.478524, 0.394923, 0.325699, 0.267583, 0.218279, 0.176487, 0.140792, 0.110376, 0.0845898, 0.0628646, 0.0446772, 0.029627, 0.017359,
    0.832616, 0.670512, 0.558584, 0.470861, 0.398849, 0.338036, 0.285863, 0.240521, 0.201053, 0.166352, 0.135812, 0.108974, 0.0854257, 0.0647832, 0.0467738, 0.0311395,
    0.770894, 0.633325, 0.537588, 0.461804, 0.398847, 0.344966, 0.298041, 0.256572, 0.219803, 0.186817, 0.157128, 0.130387, 0.106271, 0.0844736, 0.0647964, 0.047032,
    0.722833, 0.603082, 0.519383, 0.452708, 0.396877, 0.348656, 0.306218, 0.268268, 0.234179, 0.203159, 0.174793, 0.148801, 0.124908, 0.102856, 0.0824895, 0.0636263,
    0.684725, 0.578298, 0.503766, 0.444169, 0.394004, 0.350406, 0.311757, 0.276907, 0.245311, 0.216269, 0.189408, 0.164493, 0.141283, 0.119545, 0.0991505, 0.0799304,
    0.654041, 0.557827, 0.490423, 0.436414, 0.390805, 0.351004, 0.315546, 0.283384, 0.254036, 0.226867, 0.201534, 0.177832, 0.155541, 0.134447, 0.114438, 0.0953504,
    0.629007, 0.540783, 0.479022, 0.429493, 0.387588, 0.350928, 0.318159, 0.288317, 0.260964, 0.235515, 0.211648, 0.189181, 0.167907, 0.147628, 0.12824, 0.109586,
    0.608342, 0.526482, 0.46926, 0.423371, 0.384512, 0.350468, 0.319976, 0.292133, 0.266534, 0.242639, 0.220136, 0.198862, 0.178622, 0.159227, 0.140582, 0.122531,
    0.591105, 0.514398, 0.460873, 0.417978, 0.38165, 0.349802, 0.321246, 0.295129, 0.27107, 0.248562, 0.227306, 0.207153, 0.187916, 0.169412, 0.151556, 0.13419,
    0.576597, 0.504112, 0.453642, 0.413236, 0.379028, 0.34904, 0.322139, 0.297513, 0.274803, 0.253531, 0.233402, 0.214284, 0.195992, 0.178354, 0.161286, 0.144635
);

float mx_orennayar(vec3 L, vec3 V, vec3 N, float NdotL, float roughness)
{
    float LdotV = dot(L, V);
    float NdotV = dot(N, V);

    float t = LdotV - NdotL * NdotV;
    t = t > 0.0 ? t / max(NdotL, NdotV) : 0.0;

    float sigma2 = mx_square(roughness * M_PI);
    float A = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    float B = 0.45f * sigma2 / (sigma2 + 0.09);

    return A + B * t;
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Appendix B.2 Equation 13
float mx_microfacet_ggx_NDF(vec3 X, vec3 Y, vec3 H, float NdotH, float alphaX, float alphaY)
{
    float XdotH = dot(X, H);
    float YdotH = dot(Y, H);
    float denom = mx_square(XdotH / alphaX) + mx_square(YdotH / alphaY) + mx_square(NdotH);
    return 1.0 / (M_PI * alphaX * alphaY * mx_square(denom));
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Appendix B.1 Equation 3
float mx_microfacet_ggx_PDF(vec3 X, vec3 Y, vec3 H, float NdotH, float LdotH, float alphaX, float alphaY)
{
    return mx_microfacet_ggx_NDF(X, Y, H, NdotH, alphaX, alphaY) * NdotH / (4.0 * LdotH);
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Appendix B.2 Equation 15
vec3 mx_microfacet_ggx_IS(vec2 Xi, vec3 X, vec3 Y, vec3 N, float alphaX, float alphaY)
{
    float phi = 2.0 * M_PI * Xi.x;
    float tanTheta = sqrt(Xi.y / (1.0 - Xi.y));
    vec3 H = vec3(X * (tanTheta * alphaX * cos(phi)) +
                  Y * (tanTheta * alphaY * sin(phi)) +
                  N);
    return normalize(H);
}

// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf (Equation 34)
float mx_microfacet_ggx_G1(float cosTheta, float alpha)
{
    float cosTheta2 = cosTheta * cosTheta;
    float tanTheta2 = (1.0 - cosTheta2) / cosTheta2;
    return 2.0 / (1.0 + sqrt(1.0 + alpha * alpha * tanTheta2));
}

// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf (Equation 23)
float mx_microfacet_ggx_smith_G(float NdotL, float NdotV, float alpha)
{
    return mx_microfacet_ggx_G1(NdotL, alpha) * mx_microfacet_ggx_G1(NdotV, alpha);
}

// http://blog.selfshadow.com/publications/s2017-shading-course/imageworks/s2017_pbs_imageworks_sheen.pdf (Equation 2)
float mx_microfacet_sheen_NDF(float NdotH, float alpha)
{
    float invAlpha = 1.0 / alpha;
    float cos2h = NdotH * NdotH;
    float sin2h = 1.0 - cos2h;
    return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * M_PI);
}

float mx_microfacet_sheen_albedo(float cosTheta, float alpha)
{
   float sr = alpha    * (SHEEN_ALBEDO_TABLE_SIZE - 1);
   float sc = cosTheta * (SHEEN_ALBEDO_TABLE_SIZE - 1);
   int ir0 = int(sr);
   int ic0 = int(sc);
   float fr = sr - ir0;
   float fc = sc - ic0;
   int ir1 = clamp(ir0 + 1, 0, SHEEN_ALBEDO_TABLE_SIZE - 1);
   int ic1 = clamp(ic0 + 1, 0, SHEEN_ALBEDO_TABLE_SIZE - 1);
   return mx_mix(u_sheenAlbedo[ir0 * SHEEN_ALBEDO_TABLE_SIZE + ic0],
                 u_sheenAlbedo[ir1 * SHEEN_ALBEDO_TABLE_SIZE + ic0],
                 u_sheenAlbedo[ir0 * SHEEN_ALBEDO_TABLE_SIZE + ic1],
                 u_sheenAlbedo[ir1 * SHEEN_ALBEDO_TABLE_SIZE + ic1],
                 fr, fc);
}

vec3 mx_fresnel_schlick(float cosTheta, vec3 F0, vec3 F90, float exponent)
{
    float x = clamp(1.0 - cosTheta, 0.0, 1.0);
    return mix(F0, F90, pow(x, exponent));
}

vec3 mx_fresnel_schlick(float cosTheta, vec3 F0)
{
    if (cosTheta < 0.0)
        return vec3(1.0);
    float x = 1.0 - cosTheta;
    float x2 = x*x;
    float x5 = x2*x2*x;
    return F0 + (1.0 - F0) * x5;
}

float mx_fresnel_schlick(float cosTheta, float ior)
{
    if (cosTheta < 0.0)
        return 1.0;
    float F0 = (ior - 1.0) / (ior + 1.0);
    F0 *= F0;
    float x = 1.0 - cosTheta;
    float x2 = x*x;
    float x5 = x2*x2*x;
    return F0 + (1.0 - F0) * x5;
}

float mx_fresnel_schlick_roughness(float cosTheta, float ior, float roughness)
{
    cosTheta = abs(cosTheta);
    float F0 = (ior - 1.0) / (ior + 1.0);
    F0 *= F0;
    float x = 1.0 - cosTheta;
    float x2 = x*x;
    float x5 = x2*x2*x;
    return F0 + (max(1.0 - roughness, F0) - F0) * x5;
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
float mx_fresnel_dielectric(float cosTheta, float ior)
{
    if (cosTheta < 0.0)
        return 1.0;

    float g =  ior*ior + cosTheta*cosTheta - 1.0;
    // Check for total internal reflection
    if (g < 0.0)
        return 1.0;

    g = sqrt(g);
    float gmc = g - cosTheta;
    float gpc = g + cosTheta;
    float x = gmc / gpc;
    float y = (gpc * cosTheta - 1.0) / (gmc * cosTheta + 1.0);
    return 0.5 * x * x * (1.0 + y * y);
}

vec3 mx_fresnel_conductor(float cosTheta, vec3 n, vec3 k)
{
   float c2 = cosTheta*cosTheta;
   vec3 n2_k2 = n*n + k*k;
   vec3 nc2 = 2.0 * n * cosTheta;

   vec3 rs_a = n2_k2 + c2;
   vec3 rp_a = n2_k2 * c2 + 1.0;
   vec3 rs = (rs_a - nc2) / (rs_a + nc2);
   vec3 rp = (rp_a - nc2) / (rp_a + nc2);

   return 0.5 * (rs + rp);
}
