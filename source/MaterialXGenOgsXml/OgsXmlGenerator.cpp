//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOgsXml/OgsXmlGenerator.h>
#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

namespace
{
    // Semantics used by OgsXml
    static const StringMap OGSXML_SEMANTICS_MAP =
    {
        { "i_position", "POSITION"},
        { "i_normal", "NORMAL" },
        { "i_tangent", "TANGENT" },
        { "i_bitangent", "BINORMAL" },

        { "i_texcoord_0", "TEXCOORD0" },
        { "i_texcoord_1", "TEXCOORD1" },
        { "i_texcoord_2", "TEXCOORD2" },
        { "i_texcoord_3", "TEXCOORD3" },
        { "i_texcoord_4", "TEXCOORD4" },
        { "i_texcoord_5", "TEXCOORD5" },
        { "i_texcoord_6", "TEXCOORD6" },
        { "i_texcoord_7", "TEXCOORD7" },

        { "i_color_0", "COLOR0" },

        { "u_worldMatrix", "World" },
        { "u_worldInverseMatrix", "WorldInverse" },
        { "u_worldTransposeMatrix", "WorldTranspose" },
        { "u_worldInverseTransposeMatrix", "WorldInverseTranspose" },

        { "u_viewMatrix", "View" },
        { "u_viewInverseMatrix", "ViewInverse" },
        { "u_viewTransposeMatrix", "ViewTranspose" },
        { "u_viewInverseTransposeMatrix", "ViewInverseTranspose" },

        { "u_projectionMatrix", "Projection" },
        { "u_projectionInverseMatrix", "ProjectionInverse" },
        { "u_projectionTransposeMatrix", "ProjectionTranspose" },
        { "u_projectionInverseTransposeMatrix", "ProjectionInverseTranspose" },

        { "u_worldViewMatrix", "WorldView" },
        { "u_viewProjectionMatrix", "ViewProjection" },
        { "u_worldViewProjectionMatrix", "WorldViewProjection" },

        { "u_viewDirection", "ViewDirection" },
        { "u_viewPosition", "WorldCameraPosition" },

        { "u_frame", "Frame" },
        { "u_time", "Time" }
    };
}


OgsXmlGenerator::OgsXmlGenerator()
{
}

void OgsXmlGenerator::generate(const Shader* glsl, const Shader* /*hlsl*/, std::ostream& stream)
{
    if (glsl)
    {
        stream << glsl->getSourceCode(Stage::PIXEL);
    }
}

} // namespace MaterialX
