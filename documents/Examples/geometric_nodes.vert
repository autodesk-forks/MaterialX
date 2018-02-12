#version 400

uniform mat4 u_normalMatrix = mat4(1.0);
uniform mat4 u_modelMatrix = mat4(1.0);
uniform mat4 u_viewProjectionMatrix = mat4(1.0);

in vec3 i_normal;
in vec3 i_position;

out VertexData
{
    vec3 normalWorld;
} vd;

void main()
{
    vec4 hPositionWorld = u_modelMatrix * vec4(i_position, 1.0);
    gl_Position = u_viewProjectionMatrix * hPositionWorld;
    vd.normalWorld = normalize((u_normalMatrix * vec4(i_normal, 0)).xyz);
}

