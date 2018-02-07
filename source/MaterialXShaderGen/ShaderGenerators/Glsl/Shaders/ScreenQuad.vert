#version 330 core
uniform mat4 modelView;
uniform mat4 proj;
in vec2 texCoordsIn;
out vec2 texCoordsOut;
void main(void)
{
   // converting uv to NDC [-1,1] as vertex input
   gl_Position = proj * modelView * vec4( texCoordsIn*2.f-1.f, 0.0, 1.0 );
   texCoordsOut = texCoordsIn;
}
