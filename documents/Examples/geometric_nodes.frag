#version 400

// --------------------------------- Common Constants ---------------------------------------

#define M_PI 3.1415926535897932384626433832795
#define M_PI_INV 1.0/3.1415926535897932384626433832795

struct BSDF { vec3 fr; vec3 ft; };
#define EDF vec3
struct VDF { vec3 absorption; vec3 scattering; };
struct surfaceshader { vec3 color; vec3 transparency; };
struct volumeshader { VDF vdf; vec3 edf; };
struct displacementshader { vec3 offset; float scale; };

uniform mat4 u_normalMatrix = mat4(1.0);
uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 u_viewProjectionMatrix = mat4(1.0);

in VertexData
{
    vec3 normalWorld;
} vd;

// Data output by the pixel shader
out vec4 geometric_nodes_output1;

float square(float x)
{
    return x*x;
}

vec2 square(vec2 x)
{
    return x*x;
}

float maxv(vec2 v)
{
    return max(v.x, v.y);
}

float maxv(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

float maxv(vec4 v)
{
    return max(max(max(v.x, v.y), v.z), v.w);
}

void vdirection(vec2 texcoord, out vec2 result)
{
   result = texcoord;
}

void main()
{
    vec3 normal1_out = vd.normalWorld;
    geometric_nodes_output1 = vec4(normal1_out, 1.0);
}

