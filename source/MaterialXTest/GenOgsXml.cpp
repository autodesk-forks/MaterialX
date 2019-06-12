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

class OgsXmlGeneratorTester : public GlslShaderGeneratorTester
{
  public:
      OgsXmlGeneratorTester(const mx::FilePathVec& testRootPaths, const mx::FilePath& libSearchPath,
                                 const mx::FileSearchPath& srcSearchPath, const mx::FilePath& logFilePath) :
            GlslShaderGeneratorTester(mx::GlslFragmentGenerator::create(), testRootPaths, libSearchPath, srcSearchPath, logFilePath)
    {}

    void setTestStages() override
    {
        _testStages.push_back(mx::Stage::PIXEL);
    }

  protected:
    void getImplementationWhiteList(mx::StringSet& whiteList) override
    {
        whiteList =
        {
            "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
            "volumeshader", "IM_constant_", "IM_dot_", "IM_geomattrvalue", "IM_light_genglsl",
            "IM_point_light_genglsl", "IM_spot_light_genglsl", "IM_directional_light_genglsl"
        };
    }
};

static void generateOgsXml()
{
    const mx::FilePath testRootPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    const mx::FilePath testRootPath2 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/StandardSurface");
    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(testRootPath);
    const mx::FilePath libSearchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    const mx::FileSearchPath srcSearchPath(libSearchPath.asString());
    const mx::FilePath logPath("ogsxml_generate_test.txt");
    OgsXmlGeneratorTester tester(testRootPaths, libSearchPath, srcSearchPath, logPath);

    const mx::GenOptions genOptions;
    mx::FilePath optionsFilePath = testRootPath / mx::FilePath("_options.mtlx");
    tester.validate(genOptions, optionsFilePath);
}

TEST_CASE("GenShader: OGS Fragment Generation", "[ogsxml]")
{
    generateOgsXml();
}

TEST_CASE("GenShader: OGS XML Generation", "[ogsxml]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    GenShaderUtil::loadLibraries({ "stdlib", "pbrlib" }, searchPath, doc);

    mx::ShaderGeneratorPtr glslGenerator = mx::GlslFragmentGenerator::create();
    mx::GenContext glslContext(glslGenerator);
    glslContext.registerSourceCodeSearchPath(searchPath);

    mx::NodeGraphPtr graph = doc->getNodeGraph("NG_tiledimage_float");
    mx::OutputPtr output = graph->getOutput("N_out_float");

    mx::ShaderPtr glsl = glslGenerator->generate(graph->getName(), output, glslContext);

    mx::OgsXmlGenerator xmlGenerator;
    std::ofstream file(graph->getName() + ".xml");
    xmlGenerator.generate(glsl.get(), nullptr, file);
}
