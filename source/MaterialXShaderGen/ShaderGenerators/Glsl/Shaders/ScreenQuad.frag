//-
// ==========================================================================
// Copyright 2017 Autodesk, Inc.  All rights reserved.
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
// ==========================================================================
//+
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
