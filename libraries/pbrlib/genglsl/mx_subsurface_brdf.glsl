﻿#include "pbrlib/genglsl/lib/mx_microfacet_diffuse.glsl"

void mx_subsurface_brdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 color, vec3 radius, float anisotropy, vec3 normal, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    vec3 sss = mx_subsurface_scattering_approx(normal, L, P, color, radius);
    float NdotL = clamp(dot(normal, L), M_FLOAT_EPS, 1.0);
    float visibleOcclusion = 1.0 - NdotL * (1.0 - occlusion);
    result = sss * visibleOcclusion * weight;
}

void mx_subsurface_brdf_indirect(vec3 V, float weight, vec3 color, vec3 radius, float anisotropy, vec3 normal, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    // For now, we render indirect subsurface as simple indirect diffuse.
    vec3 Li = mx_environment_irradiance(normal);
    result = Li * color * weight;
}