//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenShader/Shader.h>

#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>

#include <MaterialXTest/GenShaderUtil.h>
#include <MaterialXTest/GenGlsl.h>

namespace mx = MaterialX;

TEST_CASE("GenShader: OGS XML Generation", "[ogsxml]")
{
    mx::DocumentPtr doc = mx::createDocument();

    const mx::FilePath librariesPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    GenShaderUtil::loadLibraries({ "stdlib", "pbrlib", "bxdf" }, librariesPath, doc);

    const mx::FilePath resourcesPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources");
    GenShaderUtil::loadLibraries({ "Materials/TestSuite", "Materials/Examples" }, resourcesPath, doc);

    mx::ShaderGeneratorPtr glslGenerator = mx::GlslFragmentGenerator::create();
    mx::GenContext glslContext(glslGenerator);
    glslContext.registerSourceCodeSearchPath(librariesPath);

    mx::OgsXmlGenerator xmlGenerator;

    mx::StringVec testMaterials = { "Tiled_Brass" };

    for (auto testMaterial : testMaterials)
    {
        mx::MaterialPtr mtrl = doc->getMaterial(testMaterial);
        if (mtrl)
        {
            std::vector<mx::ShaderRefPtr> shaderRefs = mtrl->getShaderRefs();
            for (auto shaderRef : shaderRefs)
            {
                mx::ShaderPtr glsl = glslGenerator->generate(shaderRef->getName(), shaderRef, glslContext);
                std::ofstream file(shaderRef->getName() + ".xml");
                xmlGenerator.generate(glsl.get(), nullptr, file);
            }
        }
    }
}
