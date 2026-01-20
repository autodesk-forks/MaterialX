#include "lib/mx_closure_type.glsl"

void mx_layer_bsdf(ClosureData closureData, BSDF top, BSDF base, out BSDF result)
{
    // If top is effectively empty (zero response and full throughput), just pass through base
    // This handles the case where optional nodes like sheen_bsdf are skipped
    if (length(top.response) < 0.0001 && length(top.throughput - vec3(1.0)) < 0.0001)
    {
        result = base;
    }
    else
    {
        result.response = top.response + base.response * top.throughput;
        result.throughput = top.throughput * base.throughput;
    }
}
