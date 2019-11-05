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
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    mx::ShaderGeneratorPtr generator = mx::OslShaderGenerator::create();
    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));

    mx::FilePath outputDirector = mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/reference/osl");
    outputDirector.createDirectory();

    const std::vector<mx::NodeDefPtr> nodedefs = doc->getNodeDefs();
    for (const mx::NodeDefPtr& nodedef : nodedefs)
    {
        std::string nodeName = nodedef->getName();
        if (nodeName.size() > 3 && nodeName.substr(0, 3) == "ND_")
        {
            nodeName = nodeName.substr(3);
        }

        mx::NodePtr node = doc->addNode(nodedef->getNodeString(), nodeName, nodedef->getType());
        REQUIRE(node);

        try
        {
            mx::ShaderPtr shader = generator->generate(node->getName(), node, context);

            mx::FilePath filename = outputDirector / (nodeName + ".osl");

            std::ofstream file;
            file.open(filename.asString());
            file << shader->getSourceCode();
            file.close();
        }
        catch (mx::ExceptionShaderGenError e)
        {
            std::cout << "Failed generating node '" << nodeName << " : ";
            std::cout << e.what() << std::endl;
        }

        doc->removeChild(node->getName());
    }
}
