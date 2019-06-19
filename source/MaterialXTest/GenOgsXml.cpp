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
                mx::ShaderPtr shader = glslGenerator->generate(shaderRef->getName(), shaderRef, glslContext);
                std::ofstream file(shaderRef->getName() + ".xml");
                xmlGenerator.generate(shader.get(), nullptr, file);

                std::string _fragmentName;
                std::string _fragmentWrapper;
                MaterialX::StringVec _globalsList;
                MaterialX::StringMap _pathInputMap;
                MaterialX::StringMap _pathOutputMap;

                const MaterialX::ShaderStage& ps = shader->getStage(MaterialX::Stage::PIXEL);

                // Cache global parameters
                for (auto uniformsIt : ps.getUniformBlocks())
                {
                    const MaterialX::VariableBlock& uniforms = *uniformsIt.second;

                    // Skip light uniforms
                    if (uniforms.getName() == MaterialX::HW::LIGHT_DATA)
                    {
                        continue;
                    }

                    //bool isPrivate = uniforms.getName() == MaterialX::HW::PRIVATE_UNIFORMS;
                    for (size_t i = 0; i < uniforms.size(); i++)
                    {
                        const MaterialX::ShaderPort* port = uniforms[i];
                        if (!port)
                        {
                            continue;
                        }
                        const std::string& name = port->getVariable();
                        if (name.empty())
                        {
                            continue;
                        }
                        const std::string& path = port->getPath();
                        // Globals have no path
                        if (path.empty())
                        {
                            std::cout << "Add input globals name: " << name << std::endl;
                            _globalsList.push_back(name);
                        }
                        else
                        {
                            std::cout << "Add path: " << path << ". Frag name: " << name << std::endl;
                            _pathInputMap[path] = name;
                        }
                    }
                }

                // Cache output parameters
                for (auto uniformsIt : ps.getOutputBlocks())
                {
                    const MaterialX::VariableBlock& uniforms = *uniformsIt.second;
                    for (size_t i = 0; i < uniforms.size(); ++i)
                    {
                        const MaterialX::ShaderPort* v = uniforms[i];
                        std::string name = v->getVariable();
                        std::string path = v->getPath();
                        if (name.empty() || path.empty())
                        {
                            continue;
                        }
                        std::cout << "Add output path: " << path << ". Frag name: " << name << std::endl;
                        _pathOutputMap[path] = name;
                    }
                }
            }
        }
    }
}
