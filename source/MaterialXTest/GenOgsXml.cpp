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
    INFO("Start genogsxml");
    mx::DocumentPtr doc = mx::createDocument();

    const mx::FilePath librariesPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    INFO("Load lib 1 START");
    loadLibraries({ "stdlib", "pbrlib", "bxdf", "lights" }, librariesPath, doc);
    INFO("Load lib 2 END");

    const mx::FilePath resourcesPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources");
    mx::FileSearchPath searchPath;
    searchPath.append(resourcesPath);
    searchPath.append(librariesPath);
    INFO("Load lib 1 START");
    loadLibraries({ "Materials/TestSuite/libraries/metal", "Materials/Examples" }, searchPath, doc);
    INFO("Load lib 2 END");

    INFO("Create glsl generator START");
    mx::ShaderGeneratorPtr glslGenerator = mx::GlslFragmentGenerator::create();
    INFO("Create glsl generator end");
    INFO("Create context START");
    mx::GenContext glslContext(glslGenerator);
    INFO("Create context end");
    glslContext.registerSourceCodeSearchPath(librariesPath);
    glslContext.getOptions().fileTextureVerticalFlip = true;

    mx::OgsXmlGenerator xmlGenerator;

    //mx::StringVec testGraphs = { };
    mx::StringVec testMaterials = { "Tiled_Brass", "Brass_Wire_Mesh" };
#if 0
    for (const auto& testGraph : testGraphs)
    {
        mx::NodeGraphPtr graph = doc->getNodeGraph(testGraph);
        if (graph)
        {
            std::vector<mx::OutputPtr> outputs = graph->getOutputs();
            for (const auto& output : outputs)
            {
                const std::string name = graph->getName() + "_" + output->getName();
                mx::ShaderPtr shader = glslGenerator->generate(name, output, glslContext);
                std::ofstream file(name + ".xml");
                std::string shaderName = output->getNamePath();
                shaderName = MaterialX::createValidName(shaderName);
                xmlGenerator.generate(shaderName, shader.get(), nullptr, file);
            }
        }
    }
#endif
    for (const auto& testMaterial : testMaterials)
    {
        mx::MaterialPtr mtrl = doc->getMaterial(testMaterial);
        INFO("Scan material: " + testMaterial);
        if (mtrl)
        {
            std::vector<mx::ShaderRefPtr> shaderRefs = mtrl->getShaderRefs();
            for (const auto& shaderRef : shaderRefs)
            {
                INFO("Generate shaderref: " + shaderRef->getNamePath());
                mx::ShaderPtr shader = nullptr;
                try
                {
                    shader = glslGenerator->generate(shaderRef->getName(), shaderRef, glslContext);
                }
                catch (mx::Exception& e)
                {
                    INFO("Failed to generate GLSL: " + std::string(e.what()));
                }
                if (shader)
                {
                    std::ofstream file(shaderRef->getName() + ".xml");
                    std::string shaderName = shaderRef->getNamePath();
                    shaderName = MaterialX::createValidName(shaderName);
                    try
                    {
                        xmlGenerator.generate(shaderName, shader.get(), nullptr, file);
                    }
                    catch (mx::Exception& e)
                    {
                        INFO("Failed to generate GLSL XML: " + std::string(e.what()));
                    }
                }
            }
        }
    }
}
