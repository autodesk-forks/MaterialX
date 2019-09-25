//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/UnitConverter.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/Util.h>

#include <cmath>

namespace mx = MaterialX;

const float EPSILON = 1e-4f;

TEST_CASE("UnitAttribute", "[units]")
{
    mx::DocumentPtr doc = mx::createDocument();
    mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_defs.mtlx"), doc);
    std::vector<mx::UnitTypeDefPtr> unitTypeDefs = doc->getUnitTypeDefs();
    REQUIRE(!unitTypeDefs.empty());

    doc->setUnit("millimeter");

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    nodeGraph->setName("graph1");

    // Basic get/set unit testing
    mx::NodePtr constant = nodeGraph->addNode("constant");
    constant->setName("constant1");
    constant->setParameterValue("value", mx::Color3(0.5f));
    mx::ParameterPtr param = constant->getParameter("value");
    param->setName("param1");
    param->setUnit("meter");
    REQUIRE(param->hasUnit());
    REQUIRE(!param->getUnit().empty());

    // Test for valid unit names
    mx::OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);
    output->setUnit("bad unit");
    REQUIRE(!output->validate());
    output->setUnit("foot");
    REQUIRE(output->hasUnit());
    REQUIRE(!output->getUnit().empty());

    REQUIRE(doc->validate());

    // Check for target unit search. Parent units are inches,
    // library units are "mile"
    // - Parent doc traversal check
    mx::DocumentPtr parentDoc = mx::createDocument();
    parentDoc->setUnit("foot");
    mx::NodeGraphPtr nodeGraph2 = parentDoc->addNodeGraph();
    nodeGraph2->setName("parent_graph1");

    mx::ElementPtr nodeGraph3 = parentDoc->getDescendant("/parent_graph1");
    REQUIRE(nodeGraph3);
    const std::string& au = nodeGraph3->getActiveUnit();
    const std::string& u = nodeGraph3->getUnit();
    REQUIRE((au == "foot" && u.empty()));

    // - Imported doc traversal check
    mx::CopyOptions copyOptions;
    copyOptions.skipConflictingElements = true;
    doc->setUnit("mile");
    parentDoc->importLibrary(doc, &copyOptions);

    mx::ElementPtr const1 = parentDoc->getDescendant("/graph1/constant1");
    REQUIRE(const1);
    const std::string& c1 = const1->getActiveUnit();
    const std::string& c2 = const1->getUnit();
    REQUIRE((c1 == "mile" && c2.empty()));
}

TEST_CASE("UnitEvaluation", "[units]")
{
    mx::DocumentPtr doc = mx::createDocument();
    mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_defs.mtlx"), doc);

    mx::UnitTypeDefPtr lengthTypeDef = doc->getUnitTypeDef(mx::LengthUnitConverter::LENGTH_UNIT);
    REQUIRE(lengthTypeDef);

    mx::UnitConverterRegistryPtr registry = mx::UnitConverterRegistry::create();
    mx::UnitConverterRegistryPtr registry2 = mx::UnitConverterRegistry::create();
    REQUIRE(registry == registry2);

    mx::LengthUnitConverterPtr converter = mx::LengthUnitConverter::create(lengthTypeDef);
    REQUIRE(converter);
    registry->addUnitConverter(lengthTypeDef, converter);
    mx::UnitConverterPtr uconverter = registry->getUnitConverter(lengthTypeDef);
    REQUIRE(uconverter);

    // Use converter to convert
    float result = converter->convert(0.1f, "kilometer", "millimeter");
    REQUIRE((result - 10000.0f) < EPSILON);
    result = converter->convert(2.3f, "meter", "meter");
    REQUIRE((result - 2.3f) < EPSILON);
    result = converter->convert(1.0f, "mile", "meter");
    REQUIRE((result - 0.000621f) < EPSILON);
    result = converter->convert(1.0f, "meter", "mile");
    REQUIRE((result - (1.0 / 0.000621f)) < EPSILON);

    // Use explicit converter values
    const std::unordered_map<std::string, float>& unitScale = converter->getUnitScale();
    result = 0.1f * unitScale.find("kilometer")->second / unitScale.find("millimeter")->second;
    REQUIRE((result - 10000.0f) < EPSILON);
    const std::string& defaultUnit = converter->getDefaultUnit();
    REQUIRE(defaultUnit == lengthTypeDef->getDefault());
}

TEST_CASE("UnitDocument", "[units]")
{
    mx::FilePath libraryPath("libraries/stdlib");
    mx::FilePath examplesPath("resources/Materials/Examples/Units");
    std::string searchPath = libraryPath.asString() +
        mx::PATH_LIST_SEPARATOR +
        examplesPath.asString();

    // Read and validate each example document.
    for (std::string filename : examplesPath.getFilesInDirectory(mx::MTLX_EXTENSION))
    {
        mx::DocumentPtr doc = mx::createDocument();
        mx::readFromXmlFile(doc, filename, searchPath);
        mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_defs.mtlx"), doc);

        mx::UnitTypeDefPtr lengthTypeDef = doc->getUnitTypeDef(mx::LengthUnitConverter::LENGTH_UNIT);
        REQUIRE(lengthTypeDef);

        mx::UnitConverterPtr uconverter = mx::LengthUnitConverter::create(lengthTypeDef);
        REQUIRE(uconverter);
        mx::UnitConverterRegistryPtr registry = mx::UnitConverterRegistry::create();
        registry->addUnitConverter(lengthTypeDef, uconverter);
        uconverter = registry->getUnitConverter(lengthTypeDef);
        REQUIRE(uconverter);

        // Traverse the document tree
        for (mx::ElementPtr elem : doc->traverseTree())
        {
            // If we have nodes with inputs
            mx::NodePtr pNode = elem->asA<mx::Node>();
            if (pNode) 
            {

                if (pNode->getInputCount()) {
                    for (mx::InputPtr input : pNode->getInputs()) {
                        const mx::TypeDesc* type = mx::TypeDesc::get(input->getType());
                        const mx::ValuePtr value = input->getValue();
                        if (input->hasUnit()) {

                            if (type->isScalar() && value)
                            {
                                float originalval = value->asA<float>();
                                float convertedValue = uconverter->convert(originalval, input->getUnit(), lengthTypeDef->getDefault());
                                float reconvert = uconverter->convert(convertedValue, lengthTypeDef->getDefault(), input->getUnit());
                                REQUIRE(originalval == reconvert);
                            }
                        }
                    }
                }

                if (pNode->getParameterCount()) {
                    for (mx::ParameterPtr param: pNode->getParameters()) {
                        const mx::TypeDesc* type = mx::TypeDesc::get(param->getType());
                        const mx::ValuePtr value = param->getValue();
                        std::string value_string = value ? value->getValueString() : "No value ";
                        if (param->hasUnit()) {

                            if (type->isScalar() && value)
                            {
                                float originalval = value->asA<float>();
                                float convertedValue = uconverter->convert(originalval, param->getUnit(), lengthTypeDef->getDefault());
                                float reconvert = uconverter->convert(convertedValue, lengthTypeDef->getDefault(), param->getUnit());
                                REQUIRE(originalval == reconvert);
                            }
                        }
                    }
                }

            }
        }
    }
}
