// Copyright(c) 2019, NVIDIA CORPORATION. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace vk
{
  namespace su
  {
    vk::ShaderModule createShaderModule( vk::Device const & device, vk::ShaderStageFlagBits shaderStage, std::string const & shaderText );

    bool GLSLtoSPV( const vk::ShaderStageFlagBits shaderType, std::string const & glslShader, std::vector<unsigned int> & spvShader );
  }  // namespace su
}  // namespace vk

//ASHWIN: Dummy debug shader that draws two triangles
//        It ignores all inputs like inColor, pos and uniform buffer
const std::string vertexShaderText_PC_C_TRI = R"(
#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform buffer
{
  mat4 mvp;
} uniformBuffer;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main()
{
    const vec2 positions[4] = vec2[](
        vec2(-0.5, -0.5),
        vec2(+0.5, -0.5),
        vec2(-0.5, +0.5),
        vec2(+0.5, +0.5)
    );
    const vec2 coords[4] = vec2[](
        vec2(0, 0),
        vec2(1, 0),
        vec2(0, 1),
        vec2(1, 1)
    );
  //force blue color, ignore all inputs
  outColor = vec4(1,0,0,0.5);
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
)";
   

   
// vertex shader with (P)osition and (C)olor in and (C)olor out
const std::string vertexShaderText_PC_C = R"(
#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform buffer
{
  mat4 mvp;
} uniformBuffer;

layout (location = 0) in vec3 pos;

layout (location = 0) out vec4 outColor;

void main()
{
    const vec2 positions[3] = vec2[](
        vec2(-0.5, -0.5),
        vec2(+0.5, -0.5),
        vec2(-0.5, +0.5)
    );

    const vec4 colors[3] = vec4[](
        vec4(1,0,0,1),
        vec4(0,1,0,1),
        vec4(0,0,1,1)
    );
   

  gl_Position = vec4(pos, 1.0);
  outColor = vec4(0,0,1,0);
}
)";

// fragment shader with (C)olor in and (C)olor out
const std::string fragmentShaderText_C_C = R"(
#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 color;

layout (location = 0) out vec4 outColor;

void main()
{
  outColor = color;
}
)";


// vertex shader with (P)osition and (T)exCoord in and (T)exCoord out
const std::string vertexShaderText_PT_T = R"(
#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform buffer
{
  mat4 mvp;
} uniformBuffer;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 inTexCoord;

layout (location = 0) out vec2 outTexCoord;

void main()
{
  outTexCoord = inTexCoord;
  gl_Position = uniformBuffer.mvp * pos;
}
)";


// fragment shader with (T)exCoord in and (C)olor out
const std::string fragmentShaderText_T_C = R"(
#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D tex;

layout (location = 0) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;

void main()
{
  outColor = texture(tex, inTexCoord);
}
)";

//const std::string vertexShaderText_PC_M = R"(
//#version 450
//
//#extension GL_ARB_separate_shader_objects : enable
//#extension GL_ARB_shading_language_420pack : enable
//
//layout (std140, binding = 0) uniform UBO
//{
//    mat4 mvp;
//    mat4 model;
//    vec3 lightDir;
//    float pad;
//} ubo;
//
//layout (location = 0) in vec3 inPosition;
//layout (location = 1) in vec3 inNormal;
//layout (location = 2) in vec2 inUV;
//
//layout (location = 0) out vec3 outNormal;
//layout (location = 1) out vec3 outLightDir;
//layout (location = 2) out float outXPos;
//layout (location = 3) out vec2 outUV; 
//
//void main()
//{
//    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
//    gl_Position = ubo.mvp * vec4(inPosition, 1.0);
//
//    outNormal = normalize(mat3(ubo.model) * inNormal);
//    outLightDir = normalize(ubo.lightDir);
//    outXPos = worldPos.x;
//}
//
//
//)";
//
//const std::string fragmentShaderText_C_M = R"(
//#version 450
//
//layout(location = 0) in vec3 fragNormal;
//layout(location = 1) in vec3 fragLightDir;
//layout(location = 2) in float fragXPos;
//layout (location = 3) in vec2 fragUV;
//
//layout(location = 0) out vec4 outColor;
//
//
//void main()
//{
//    vec3 N = normalize(fragNormal);
//    vec3 L = normalize(fragLightDir);
//    vec3 V = normalize(vec3(0.0, 0.0, 1.0)); 
//    vec3 H = normalize(L + V); 
//
//    float diff = max(dot(N, L), 0.0);
//    float spec = pow(max(dot(N, H), 0.0), 250.0); 
//    float t = clamp((fragXPos + 1.0) * 0.5, 0.0, 1.0);
//    vec3 baseColor = mix(vec3(0.0, 1.0, 0.0), vec3(0.0, 1.0, 1.0), t); 
//    vec3 ambient = vec3(0.07);
//
//    vec3 finalColor = ambient + baseColor * diff + vec3(1.0) * spec;
//
//    finalColor = clamp(finalColor, 0.0, 1.0);
//
//    outColor = vec4(finalColor, 1.0);
//}
//
//
//)";


const std::string vertexShaderText_PC_M = R"(
#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(std140, binding = 0) uniform UBO
{
    mat4 mvp;
    mat4 model;
    vec3 lightDir;
    float pad;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outLightDir;
layout(location = 2) out float outXPos;
layout(location = 3) out vec2 outUV;

void main()
{
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.mvp * vec4(inPosition, 1.0);

    outNormal = normalize(mat3(ubo.model) * inNormal);
    outLightDir = normalize(vec3(1.0, 1.0, 1.0));
    outXPos = worldPos.x;
    outUV = inUV ;
}

)";

const std::string fragmentShaderText_C_M = R"(
#version 450

layout (binding = 1) uniform sampler2D tex;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragLightDir;
layout(location = 2) in float fragXPos;
layout(location = 3) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

// Returns 1.0 or 0.0 depending on checkerboard pattern for given UV and scale
float checkerboard(vec2 uv, float scale)
{
    vec2 grid = floor(uv * scale);
    float checker = mod(grid.x + grid.y, 2.0);
    return checker;
}

void main()
{
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(fragLightDir);
    vec3 V = normalize(vec3(0.0, 0.0, 1.0));
    vec3 H = normalize(L + V); 

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 64.0); 

    float checker = checkerboard(fragUV, 8.0);

    // Mix between black and white squares based on checker pattern
    vec3 baseColor = mix(vec3(1.0), vec3(0.0), checker);

    vec3 ambient = vec3(0.1);

    vec3 finalColor = ambient + baseColor * diff + vec3(1.0) * spec;

    finalColor = clamp(finalColor, 0.0, 1.0);

    //outColor = vec4(finalColor, 1.0);
    outColor = texture(tex, fragUV);
    //outColor = vec4(fragUV, 1.0, 1.0);

}


)";