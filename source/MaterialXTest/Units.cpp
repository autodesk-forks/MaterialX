//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/UnitConverter.h>
#include <MaterialXFormat/File.h>
#include <MaterialXGenShader/Util.h>

#include <cmath>

namespace mx = MaterialX;

const float EPSILON = 1e-4f;

TEST_CASE("UnitAttribute", "[units]")
{
    mx::DocumentPtr doc = mx::createDocument();

    doc->setUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE, "millimeter");

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    nodeGraph->setName("graph1");

    // Basic get/set unit testing
    mx::NodePtr constant = nodeGraph->addNode("constant");
    constant->setName("constant1");
    constant->setParameterValue("value", mx::Color3(0.5f));
    mx::ParameterPtr param = constant->getParameter("value");
    param->setName("param1");
    param->setUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE, "meter");
    REQUIRE(param->hasUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE));
    REQUIRE(!param->getUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE).empty());

    // Test for valid unit names
    mx::OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);
    output->setUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE, "foot");
    REQUIRE(output->hasUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE));
    REQUIRE(!output->getUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE).empty());

    REQUIRE(doc->validate());

    // Check for target unit search. Parent units are inches,
    // library units are "mile"
    // - Parent doc traversal check
    mx::DocumentPtr parentDoc = mx::createDocument();
    parentDoc->setUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE, "foot");
    mx::NodeGraphPtr nodeGraph2 = parentDoc->addNodeGraph();
    nodeGraph2->setName("parent_graph1");

    mx::ElementPtr nodeGraph3 = parentDoc->getDescendant("/parent_graph1");
    REQUIRE(nodeGraph3);
    const std::string& au = nodeGraph3->getActiveUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE);
    const std::string& u = nodeGraph3->getUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE);
    REQUIRE((au == "foot" && u.empty()));

    // - Imported doc traversal check
    mx::CopyOptions copyOptions;
    copyOptions.skipConflictingElements = true;
    doc->setUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE, "mile");
    parentDoc->importLibrary(doc, &copyOptions);

    mx::ElementPtr const1 = parentDoc->getDescendant("/graph1/constant1");
    REQUIRE(const1);
    const std::string& c1 = const1->getActiveUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE);
    const std::string& c2 = const1->getUnit(mx::Element::LENGTH_UNIT_ATTRIBUTE);
    REQUIRE((c1 == "foot" && c2.empty()));
}

TEST_CASE("length", "[units]")
{
    mx::LengthUnitConverterPtr converter = mx::LengthUnitConverter::create();
    REQUIRE(converter);

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
    float kiloScale = converter->getMetersPerUnit("kilometer");
    float miliScale = converter->getMetersPerUnit("millimeter");

    result = 0.1f * kiloScale/ miliScale;
    REQUIRE((result - 10000.0f) < EPSILON);
}

