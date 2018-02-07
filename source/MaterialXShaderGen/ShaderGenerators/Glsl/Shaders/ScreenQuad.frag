#version 330 core
uniform sampler2D inputImage;
in vec2 texCoordsOut;
out vec4 fragColor;
void main(void)
{
   float gamma = 2.2;
   fragColor = texture( inputImage, texCoordsOut );
   fragColor.rgb = pow(fragColor.rgb, vec3(1.0/gamma));
}
