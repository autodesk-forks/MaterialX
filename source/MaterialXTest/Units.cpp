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

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();

    mx::NodePtr constant = nodeGraph->addNode("constant");
    constant->setParameterValue("value", mx::Color3(0.5f));
    mx::ParameterPtr param = constant->getParameter("value");
    param->setUnitString("meter");
    REQUIRE(param->hasUnitString());
    REQUIRE(!param->getUnitString().empty());

    mx::OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);
    output->setUnitString("bad unit");
    REQUIRE(!output->validate());
    output->setUnitString("foot");
    REQUIRE(output->hasUnitString());
    REQUIRE(!output->getUnitString().empty());

    REQUIRE(doc->validate());
}

TEST_CASE("Length", "[units]")
{
    mx::DocumentPtr doc = mx::createDocument();
    mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_defs.mtlx"), doc);

    mx::UnitTypeDefPtr lengthTypeDef = doc->getUnitTypeDef(mx::LengthUnitConverter::LENGTH_UNIT);
    REQUIRE(lengthTypeDef);

    mx::UnitConverterPtr uconverter = mx::LengthUnitConverter::create(lengthTypeDef);
    REQUIRE(uconverter);
    doc->addUnitConverter(lengthTypeDef, uconverter);
    uconverter = doc->getUnitConverter(lengthTypeDef);
    REQUIRE(uconverter);

    mx::LengthUnitConverterPtr converter = std::dynamic_pointer_cast<mx::LengthUnitConverter>(uconverter);

    // Use converter to convert
    float result = converter->convert(0.1f, "kilometer", "millimeter");
    REQUIRE((result - 10000.0f) < EPSILON);
    result = converter->convert(1.0f, "meter", "meter");
    REQUIRE((result - 1.0f) < EPSILON);
    result = converter->convert(1.0f, "mile", "meter");
    REQUIRE((result - 0.000621f) < EPSILON);
    result = converter->convert(1.0f, "meter", "mile");
    REQUIRE((result - (1.0 / 0.000621f)) < EPSILON);

    // Use explicit converter values
    const std::unordered_map<std::string, float>& unitScale = converter->getUnitScale();
    result = 0.1f * unitScale.find("kilometer")->second / unitScale.find("millimeter")->second;
    REQUIRE((result - 10000.0f) < EPSILON);
    const std::string& defaultUnit = converter->getGefaultUnit();
    REQUIRE(defaultUnit == lengthTypeDef->getDefault());
}
#if 0
TEST_CASE("Units Test", "[unitstesting]")
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

        mx::UnitConverterPtr converter = mx::LengthUnitConverter::create(lengthTypeDef);
        REQUIRE(converter);
        doc->addUnitConverter(lengthTypeDef, converter);
        converter = doc->getUnitConverter(lengthTypeDef);
        REQUIRE(converter);
        

        // Traverse the document tree
        std::cout << "Default unit is:" << lengthTypeDef->getDefault() << std::endl;
        for (mx::ElementPtr elem : doc->traverseTree())
        {
            // If we have nodes with inputs
            mx::NodePtr pNode = elem->asA<mx::Node>();
            if (pNode) 
            {
                std::string category = pNode->getCategory();
                std::cout << "category:" << category << std::endl;

                if (pNode->getInputCount()) {
                    for (mx::InputPtr input : pNode->getInputs()) {
                        const mx::TypeDesc* type = mx::TypeDesc::get(input->getType());
                        const mx::ValuePtr value = input->getValue();
                        std::string value_string = value ? value->getValueString() : "No value ";

                        std::cout << "input_name: " << input->getName() << std::endl
                                  << "input_type: " << type->getName() << std::endl
                                  << "input_value:" << value_string << std::endl;

                        if (input->hasUnitString()) {
                            std::cout << "input_unit_type: " << input->getUnitString() << std::endl;

                            if (type->isScalar() && value)
                            {
                                float val = value->asA<float>();
                                float cval = converter->convert(val, input->getUnitString(), lengthTypeDef->getDefault());
                                std::cout << "converted_value:" << cval << std::endl;
                            }
                        }
                    }
                }
            
                if (pNode->getParameterCount()) {
                    for (mx::ParameterPtr param: pNode->getParameters()) {
                        const mx::TypeDesc* type = mx::TypeDesc::get(param->getType());
                        const mx::ValuePtr value = param->getValue();
                        std::string value_string = value ? value->getValueString() : "No value ";

                        std::cout << "param_name: " << param->getName() << std::endl
                            << "param_type: " << type->getName() << std::endl
                            << "param_value: " << param->getValueString() << std::endl;

                        if (param->hasUnitString()) {
                            std::cout << "param_unit_type: " << param->getUnitString() << std::endl;

                            if (type->isScalar() && value)
                            {
                                float val = value->asA<float>();
                                float cval = converter->convert(val, param->getUnitString(), lengthTypeDef->getDefault());
                                std::cout << "From: " + param->getUnitString() << std::endl
                                    << "To: " << lengthTypeDef->getDefault() << std::endl
                                    << "converted_value: " << cval << std::endl;
                            }
                        }
                    }
                }
            }
        }
    }
}
#endif
