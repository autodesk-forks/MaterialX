//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXTest/GenShaderUtil.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>

namespace mx = MaterialX;

TEST_CASE("GenShader: OSL Reference", "[genshader]")
{
    mx::DocumentPtr stdlibDoc = mx::createDocument();
    mx::DocumentPtr implDoc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    loadLibraries({ "stdlib" }, searchPath, stdlibDoc);

    mx::FilePath librariesPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    mx::FilePath outputPathRel = "stdlib/reference/osl";
    mx::FilePath outputPath    = librariesPath / outputPathRel;

    // Create output directory
    outputPath.getParentPath().createDirectory();
    outputPath.createDirectory();

    mx::ShaderGeneratorPtr generator = mx::OslShaderGenerator::create();
    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(librariesPath);

    const std::vector<mx::NodeDefPtr> nodedefs = stdlibDoc->getNodeDefs();
    for (const mx::NodeDefPtr& nodedef : nodedefs)
    {
        std::string nodeName = nodedef->getName();
        if (nodeName.size() > 3 && nodeName.substr(0, 3) == "ND_")
        {
            nodeName = nodeName.substr(3);
        }

        mx::NodePtr node = stdlibDoc->addNode(nodedef->getNodeString(), nodeName, nodedef->getType());
        REQUIRE(node);

        const std::string filename = nodeName + ".osl";
        try
        {
            mx::ShaderPtr shader = generator->generate(node->getName(), node, context);
            std::ofstream file;
            const std::string filepath = (outputPath / filename).asString();
            file.open(filepath);
            REQUIRE(file.is_open());
            file << shader->getSourceCode();
            file.close();
        }
        catch (mx::ExceptionShaderGenError e)
        {
            std::cout << "Failed generating node '" << nodeName << " : ";
            std::cout << e.what() << std::endl;
        }

        stdlibDoc->removeChild(node->getName());

        mx::ImplementationPtr impl = implDoc->addImplementation("IM_" + nodeName + "_osl");
        impl->setNodeDef(nodedef);
        impl->setFile(outputPathRel / filename);
        impl->setFunction(node->getName());
        impl->setLanguage("osl");
    }

    mx::writeToXmlFile(implDoc, outputPath / "stdlib_osl_impl.mtlx");
}
