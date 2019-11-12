//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Observer.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/File.h>

#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPortDef.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtCoreIo.h>
#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXGenShader/Util.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

namespace mx = MaterialX;

TEST_CASE("Runtime: Token", "[runtime]")
{
    mx::RtToken tok1 = "hej";
    mx::RtToken tok2 = "hey";
    mx::RtToken tok3 = "hej";
    REQUIRE(tok1 != tok2);
    REQUIRE(tok1 == tok3);
    REQUIRE(tok1 == "hej");
    REQUIRE(tok1 == std::string("hej"));
    REQUIRE("hej" == tok1);
    REQUIRE(std::string("hej") == tok1);

    mx::RtTokenMap<int> intMap;
    intMap["one"] = 1;
    intMap["two"] = 2;
    intMap["three"] = 3;
    REQUIRE(intMap.size() == 3);
    REQUIRE(intMap.count("one"));
    REQUIRE(intMap["one"] == 1);
    REQUIRE(intMap.count("two"));
    REQUIRE(intMap["two"] == 2);
    REQUIRE(intMap.count("three"));
    REQUIRE(intMap["three"] == 3);

    mx::RtTokenSet intSet;
    intSet.insert("one");
    intSet.insert("two");
    intSet.insert("three");
    REQUIRE(intSet.size() == 3);
    REQUIRE(intSet.count("one"));
    REQUIRE(intSet.count("two"));
    REQUIRE(intSet.count("three"));
    REQUIRE(!intSet.count("four"));
}

TEST_CASE("Runtime: Values", "[runtime]")
{
    mx::RtValue v1, v2, v3;
    v1.asInt() = 42;
    v2.asFloat() = 1.0f;
    v3.asFloat() = 2.0f;
    REQUIRE(v1.asInt() == 42);
    REQUIRE(v2.asFloat() + v3.asFloat() == 3.0f);

    mx::RtValue color3;
    color3.asColor3() = mx::Color3(1.0f, 0.0f, 0.0f);
    REQUIRE(color3.asColor3()[0] == 1.0f);
    REQUIRE(color3.asFloat() == 1.0f);

    mx::RtValue vector4;
    vector4.asVector4() = mx::Vector4(4.0f, 3.0f, 2.0f, 1.0f);
    REQUIRE(vector4.asVector4()[0] == 4.0f);
    REQUIRE(vector4.asVector4()[1] == 3.0f);
    REQUIRE(vector4.asVector4()[2] == 2.0f);
    REQUIRE(vector4.asVector4()[3] == 1.0f);

    mx::RtValue v4 = vector4;
    REQUIRE(v4.asVector4()[0] == 4.0f);

    mx::RtValue ptr;
    ptr.asPtr() = &vector4;
    REQUIRE(ptr.asPtr() == &vector4);
    ptr.clear();
    REQUIRE(ptr.asPtr() == (void*)0);

    mx::RtValueStore<std::string> stringStore;
    mx::RtValueStore<mx::Matrix33> mtx33Store;

    const std::string teststring("MaterialX");
    mx::RtValue str(teststring, stringStore);
    REQUIRE(str.asString() == teststring);

    const mx::Matrix33 testmatrix(mx::Matrix33::IDENTITY);
    mx::RtValue mtx33(testmatrix, mtx33Store);
    REQUIRE(mtx33.asMatrix33().isEquivalent(testmatrix, 1e-6f));
    mtx33.asMatrix33()[0][0] = 42.0f;
    REQUIRE(!mtx33.asMatrix33().isEquivalent(testmatrix, 1e-6f));

    mx::RtValue value;
    mx::RtLargeValueStorage storage;
    value.setValueString(mx::RtType::BOOLEAN, "true", storage);
    REQUIRE(value.asBool());
    value.setValueString(mx::RtType::BOOLEAN, "false", storage);
    REQUIRE(!value.asBool());
    value.setValueString(mx::RtType::INTEGER, "23", storage);
    REQUIRE(value.asInt() == 23);
    value.setValueString(mx::RtType::FLOAT, "1234.5678", storage);
    REQUIRE(fabs(value.asFloat() - 1234.5678f) < 1e-3f);
    value.setValueString(mx::RtType::COLOR2, "1.0, 2.0", storage);
    REQUIRE(value.asColor2() == mx::Color2(1.0, 2.0));
    value.setValueString(mx::RtType::COLOR3, "1.0, 2.0, 3.0", storage);
    REQUIRE(value.asColor3() == mx::Color3(1.0, 2.0, 3.0));
    value.setValueString(mx::RtType::COLOR4, "1.0, 2.0, 3.0, 4.0", storage);
    REQUIRE(value.asColor4() == mx::Color4(1.0, 2.0, 3.0, 4.0));
    value.setValueString(mx::RtType::MATRIX33, "1.0, 0.0, 0.0,  0.0, 1.0, 0.0,  0.0, 0.0, 1.0", storage);
    REQUIRE(value.asMatrix33() == mx::Matrix33::IDENTITY);
    value.setValueString(mx::RtType::MATRIX44, "1.0, 0.0, 0.0, 0.0,  0.0, 1.0, 0.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 0.0, 1.0", storage);
    REQUIRE(value.asMatrix44() == mx::Matrix44::IDENTITY);
    value.setValueString(mx::RtType::STRING, "materialx", storage);
    REQUIRE(value.asString() == "materialx");
    value.setValueString(mx::RtType::TOKEN, "materialx", storage);
    REQUIRE(value.asToken() == mx::RtToken("materialx"));
    REQUIRE_THROWS(value.setValueString(mx::RtType::INTEGER, "true", storage));
}

TEST_CASE("Runtime: Types", "[runtime]")
{
    // Make sure the standard types are registered
    const mx::RtTypeDef* floatType = mx::RtTypeDef::findType("float");
    REQUIRE(floatType != nullptr);
    REQUIRE(floatType->getBaseType() == mx::RtTypeDef::BASETYPE_FLOAT);
    const mx::RtTypeDef* integerType = mx::RtTypeDef::findType("integer");
    REQUIRE(integerType != nullptr);
    REQUIRE(integerType->getBaseType() == mx::RtTypeDef::BASETYPE_INTEGER);
    const mx::RtTypeDef* booleanType = mx::RtTypeDef::findType("boolean");
    REQUIRE(booleanType != nullptr);
    REQUIRE(booleanType->getBaseType() == mx::RtTypeDef::BASETYPE_BOOLEAN);
    const mx::RtTypeDef* color2Type = mx::RtTypeDef::findType("color2");
    REQUIRE(color2Type != nullptr);
    REQUIRE(color2Type->getBaseType() == mx::RtTypeDef::BASETYPE_FLOAT);
    REQUIRE(color2Type->getSemantic() == mx::RtTypeDef::SEMANTIC_COLOR);
    REQUIRE(color2Type->isFloat2());
    const mx::RtTypeDef* color3Type = mx::RtTypeDef::findType("color3");
    REQUIRE(color3Type != nullptr);
    REQUIRE(color3Type->getBaseType() == mx::RtTypeDef::BASETYPE_FLOAT);
    REQUIRE(color3Type->getSemantic() == mx::RtTypeDef::SEMANTIC_COLOR);
    REQUIRE(color3Type->isFloat3());
    const mx::RtTypeDef* color4Type = mx::RtTypeDef::findType("color4");
    REQUIRE(color4Type != nullptr);
    REQUIRE(color4Type->getBaseType() == mx::RtTypeDef::BASETYPE_FLOAT);
    REQUIRE(color4Type->getSemantic() == mx::RtTypeDef::SEMANTIC_COLOR);
    REQUIRE(color4Type->isFloat4());

    // Make sure we can register a new custom type
    const mx::RtTypeDef* fooType = mx::RtTypeDef::registerType("foo", mx::RtTypeDef::BASETYPE_FLOAT, mx::RtTypeDef::SEMANTIC_COLOR, 5);
    REQUIRE(fooType != nullptr);
    const mx::RtTypeDef* fooType2 = mx::RtTypeDef::findType("foo");
    REQUIRE(fooType2 == fooType);

    // Make sure we can't use a name already take
    REQUIRE_THROWS(mx::RtTypeDef::registerType("color3", mx::RtTypeDef::BASETYPE_FLOAT));

    // Make sure we can't request an unknown type
    REQUIRE(mx::RtTypeDef::findType("bar") == nullptr);

    // Make sure a type is connectable to its own type
    // TODO: Extend to test more types when type auto cast is implemented.
    REQUIRE(floatType->getValidConnectionTypes().count(floatType->getName()));
    REQUIRE(!floatType->getValidConnectionTypes().count(color4Type->getName()));

    // Test channel index/name
    REQUIRE(color4Type->getChannelIndex('r') == 0);
    REQUIRE(color4Type->getChannelIndex('g') == 1);
    REQUIRE(color4Type->getChannelIndex('b') == 2);
    REQUIRE(color4Type->getChannelIndex('a') == 3);
    REQUIRE(color4Type->getChannelIndex('q') == -1);
    REQUIRE(color4Type->getChannelName(0) == 'r');
    REQUIRE(color4Type->getChannelName(1) == 'g');
    REQUIRE(color4Type->getChannelName(2) == 'b');
    REQUIRE(color4Type->getChannelName(3) == 'a');
    REQUIRE(color4Type->getChannelName(7) == -1);
}

TEST_CASE("Runtime: Nodes", "[runtime]")
{
    mx::RtObject stageObj = mx::RtStage::createNew("root");
    mx::RtStage stage(stageObj);

    // Test validity/bool operators
    REQUIRE(stageObj);
    REQUIRE(!stageObj == false);
    REQUIRE(stage);
    REQUIRE(!stage == false);

    // Create a new nodedef object for defining an add node
    mx::RtObject addDefObj = mx::RtNodeDef::createNew("ND_add_float", "add", stageObj);
    REQUIRE(addDefObj.isValid());
    REQUIRE(addDefObj.hasApi(mx::RtApiType::NODEDEF));

    // Attach the nodedef API to this object
    mx::RtNodeDef addDef(addDefObj);

    // Add an attribute to the nodedef
    addDef.addAttribute("version", "float", mx::RtValue(1.0f));
    REQUIRE(addDef.numAttributes() == 1);

    // Add ports to the nodedef
    mx::RtPortDef::createNew("in1", "float", mx::RtValue(1.0f), mx::RtPortFlag::DEFAULTS, addDefObj);
    mx::RtPortDef::createNew("in2", "float", mx::RtValue(42.0f), mx::RtPortFlag::DEFAULTS, addDefObj);
    mx::RtPortDef::createNew("in3", "float", mx::RtValue(42.0f), mx::RtPortFlag::DEFAULTS, addDefObj);
    mx::RtPortDef::createNew("out", "float", mx::RtValue(0.0f), mx::RtPortFlag::OUTPUT, addDefObj);
    REQUIRE(addDef.numPorts() == 4);

    // Delete a port
    mx::RtObject in3Port = addDef.findPort("in3");
    addDef.removePort(in3Port);
    REQUIRE(addDef.numPorts() == 3);

    // Test the new ports
    mx::RtPortDef out = addDef.findPort("out");
    REQUIRE(out.isValid());
    REQUIRE(out.isOutput());
    REQUIRE(out.isConnectable());
    REQUIRE(!out.isUniform());
    REQUIRE(out.getType() == "float");
    REQUIRE(out.getValue().asFloat() == 0.0f);
    mx::RtPortDef foo = addDef.findPort("foo");
    REQUIRE(!foo.isValid());
    mx::RtPortDef in1 = addDef.findPort("in1");
    REQUIRE(in1.isValid());
    REQUIRE(in1.isInput());
    REQUIRE(in1.isConnectable());
    in1.getValue().asFloat() = 7.0f;
    REQUIRE(in1.getValue().asFloat() == 7.0f);

    // Try to create a node from an invalid nodedef object
    REQUIRE_THROWS(mx::RtNode::createNew("foo", mx::RtObject(), stageObj));

    // Create two new node instances from the add nodedef
    mx::RtObject add1Obj = mx::RtNode::createNew("add1", addDefObj, stageObj);
    mx::RtObject add2Obj = mx::RtNode::createNew("add2", addDefObj, stageObj);
    REQUIRE(add1Obj.isValid());
    REQUIRE(add2Obj.isValid());

    // Attach the node API to these objects
    mx::RtNode add1(add1Obj);
    mx::RtNode add2(add2Obj);

    // Get the node instance ports
    mx::RtPort add1_in1 = add1.findPort("in1");
    mx::RtPort add1_in2 = add1.findPort("in2");
    mx::RtPort add1_out = add1.findPort("out");
    mx::RtPort add2_in1 = add2.findPort("in1");
    mx::RtPort add2_in2 = add2.findPort("in2");
    mx::RtPort add2_out = add2.findPort("out");

    // Test port connectability
    REQUIRE(add1_out.canConnectTo(add2_in1));
    REQUIRE(add2_in1.canConnectTo(add1_out));
    REQUIRE(!add1_out.canConnectTo(add1_in1));
    REQUIRE(!add1_out.canConnectTo(add2_out));
    REQUIRE(!add2_in1.canConnectTo(add1_in1));

    // Make port connections
    mx::RtNode::connect(add1_out, add2_in1);
    REQUIRE(add1_out.isConnected());
    REQUIRE(add2_in1.isConnected());

    // Try connecting already connected ports
    REQUIRE_THROWS(mx::RtNode::connect(add1_out, add2_in1));

    // Break port connections
    mx::RtNode::disconnect(add1_out, add2_in1);
    REQUIRE(!add1_out.isConnected());
    REQUIRE(!add2_in1.isConnected());

    // Make more port connections, now testing
    // the port connectTo method
    add1_out.connectTo(add2_in1);
    add1_out.connectTo(add2_in2);
    size_t numDest = add1_out.numDestinationPorts();
    REQUIRE(numDest == 2);
    REQUIRE(add1_out.getDestinationPort(0) == add2_in1);
    REQUIRE(add1_out.getDestinationPort(1) == add2_in2);
    REQUIRE(add2_in1.getSourcePort() == add1_out);
    REQUIRE(add2_in2.getSourcePort() == add1_out);

    // Find object by path
    mx::RtObject elem1 = stage.findElementByPath("/add1/in2");
    REQUIRE(elem1.isValid());
    REQUIRE(elem1.hasApi(mx::RtApiType::PORTDEF));
    REQUIRE(mx::RtPortDef(elem1).getName() == "in2");
    REQUIRE(mx::RtPortDef(elem1).isInput());

}

TEST_CASE("Runtime: NodeGraphs", "[runtime]")
{
    mx::RtObject stageObj = mx::RtStage::createNew("root");
    mx::RtStage stage(stageObj);

    // Create a new nodedef for a multiply node.
    mx::RtNodeDef multiplyDef = mx::RtNodeDef::createNew("ND_multiply_float", "multiply", stageObj);
    mx::RtPortDef::createNew("in1", "float", mx::RtValue(0.0f), mx::RtPortFlag::DEFAULTS, multiplyDef.getObject());
    mx::RtPortDef::createNew("in2", "float", mx::RtValue(0.0f), mx::RtPortFlag::DEFAULTS, multiplyDef.getObject());
    mx::RtPortDef::createNew("out", "float", mx::RtValue(0.0f), mx::RtPortFlag::OUTPUT, multiplyDef.getObject());

    // Create a nodegraph object.
    mx::RtObject graphObj1 = mx::RtNodeGraph::createNew("graph1", stageObj);
    REQUIRE(graphObj1.isValid());

    // Attach the nodegraph API to the object.
    mx::RtNodeGraph graph1(graphObj1);

    // Create two multiply nodes in the graph.
    mx::RtObject mulObj1 = mx::RtNode::createNew("add1", multiplyDef.getObject(), graphObj1);
    mx::RtObject mulObj2 = mx::RtNode::createNew("add2", multiplyDef.getObject(), graphObj1);
    REQUIRE(graph1.numNodes() == 2);

    // Test deleting a node.
    mx::RtObject mulObj3 = mx::RtNode::createNew("add3", multiplyDef.getObject(), graphObj1);
    REQUIRE(graph1.numNodes() == 3);
    graph1.removeNode(mulObj3);
    REQUIRE(graph1.numNodes() == 2);

    // Add interface port definitions to the graph.
    mx::RtPortDef::createNew("a", "float", mx::RtValue(0.0f), mx::RtPortFlag::DEFAULTS, graphObj1);
    mx::RtPortDef::createNew("b", "float", mx::RtValue(0.0f), mx::RtPortFlag::DEFAULTS, graphObj1);
    mx::RtPortDef::createNew("out", "float", mx::RtValue(0.0f), mx::RtPortFlag::OUTPUT, graphObj1);
    REQUIRE(graph1.numPorts() == 3);
    REQUIRE(graph1.findInputSocket("a").isValid());
    REQUIRE(graph1.findInputSocket("b").isValid());
    REQUIRE(graph1.findOutputSocket("out").isValid());
    REQUIRE(graph1.getInputSocket(0).isOutput());
    REQUIRE(graph1.getInputSocket(1).isOutput());
    REQUIRE(graph1.getOutputSocket(0).isInput());

    // Test deleting a port.
    mx::RtPortDef::createNew("c", "float", mx::RtValue(0.0f), mx::RtPortFlag::DEFAULTS, graphObj1);
    REQUIRE(graph1.numPorts() == 4);
    REQUIRE(graph1.findInputSocket("c").isValid());
    mx::RtObject cPort = mx::RtNodeDef(graph1.getNodeDef()).findPort("c");
    REQUIRE(cPort);
    graph1.removePort(cPort);
    REQUIRE(graph1.numPorts() == 3);
    REQUIRE(!graph1.findInputSocket("c").isValid());

    // Attach the node API for the two node objects.
    mx::RtNode mul1(mulObj1);
    mx::RtNode mul2(mulObj2);

    // Connect the graph nodes to each other and the interface.
    mx::RtNode::connect(graph1.findInputSocket("a"), mul1.findPort("in1"));
    mx::RtNode::connect(graph1.findInputSocket("b"), mul1.findPort("in2"));
    mx::RtNode::connect(mul1.findPort("out"), mul2.findPort("in1"));
    mx::RtNode::connect(graph1.findInputSocket("a"), mul2.findPort("in2"));
    mx::RtNode::connect(mul2.findPort("out"), graph1.findOutputSocket("out"));
    REQUIRE(graph1.findInputSocket("a").numDestinationPorts() == 2);
    REQUIRE(graph1.findInputSocket("b").numDestinationPorts() == 1);
    REQUIRE(graph1.findOutputSocket("out").getSourcePort() == mul2.findPort("out"));

    // Test finding a port by path.
    mx::RtObject node = stage.findElementByPath("/graph1/add2");
    mx::RtObject portdef = stage.findElementByPath("/graph1/add2/out");
    REQUIRE(node.isValid());
    REQUIRE(portdef.isValid());
    REQUIRE(node.hasApi(mx::RtApiType::NODE));
    REQUIRE(portdef.hasApi(mx::RtApiType::PORTDEF));

    // Test getting a port instance from node and portdef.
    mx::RtPort port1(node, portdef);
    REQUIRE(port1 == mul2.findPort("out"));
}

TEST_CASE("Runtime: CoreIo", "[runtime]")
{
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");

    {
        // Create a document.
        mx::DocumentPtr doc = mx::createDocument();

        // Load in stdlib
        loadLibraries({ "stdlib" }, searchPath, doc);

        // Create a stage and import the document data.
        mx::RtObject stageObj = mx::RtStage::createNew("test1");
        mx::RtCoreIo stageIo(stageObj);
        stageIo.read(doc);

        // Get a nodegraph and write a dot file for inspection.
        mx::RtStage stage(stageObj);
        mx::RtNodeGraph graph = stage.findElementByName("NG_tiledimage_float");
        REQUIRE(graph);
        std::ofstream dotfile;
        dotfile.open(graph.getName().str() + ".dot");
        dotfile << graph.asStringDot();
        dotfile.close();

        // Get a nodedef and create two new instances of it.
        mx::RtObject multiplyObj = stage.findElementByName("ND_multiply_color3");
        REQUIRE(multiplyObj);
        mx::RtNode mul1 = mx::RtNode::createNew("mul1", multiplyObj, stageObj);
        mx::RtNode mul2 = mx::RtNode::createNew("mul2", multiplyObj, stageObj);
        REQUIRE(mul1);
        REQUIRE(mul2);
        mul2.findPort("in1").getValue().asColor3() = mx::Color3(0.3f, 0.5f, 0.4f);
        mul2.findPort("in2").getValue().asColor3() = mx::Color3(0.6f, 0.3f, 0.5f);
        mul2.findPort("in2").setColorSpace("srgb_texture");

        // Write the full stage to a new document
        // and save it to file for inspection.
        mx::DocumentPtr exportDoc = mx::createDocument();
        stageIo.write(exportDoc);
        mx::writeToXmlFile(exportDoc, stage.getName().str() + "_export.mtlx");

        // Write only nodegraphs to a new document
        // and save it to file for inspection.
        auto nodeGraphFilter = [](const mx::RtObject& obj) -> bool
        {
            return obj.hasApi(mx::RtApiType::NODEGRAPH);
        };
        mx::DocumentPtr exportNodeGraphDoc = mx::createDocument();
        stageIo.write(exportNodeGraphDoc, nodeGraphFilter);
        mx::writeToXmlFile(exportNodeGraphDoc, stage.getName().str() + "_nodegraph_export.mtlx");
    }

    {
        // Load stdlib into a stage
        mx::DocumentPtr stdlibDoc = mx::createDocument();
        loadLibraries({ "stdlib" }, searchPath, stdlibDoc);

        // Create a stage and import the stdlib data.
        mx::RtStage stdlibStage = mx::RtStage::createNew("stdlib");
        mx::RtCoreIo(stdlibStage.getObject()).read(stdlibDoc);

        // Create a new working space stage.
        mx::RtStage stage = mx::RtStage::createNew("test2");

        // Add reference to stdlib
        stage.addReference(stdlibStage.getObject());

        // Create a small node network.
        mx::RtObject tiledimageDef = stage.findElementByName("ND_tiledimage_color3");
        mx::RtObject texcoordDef = stage.findElementByName("ND_texcoord_vector2");
        REQUIRE(tiledimageDef);
        REQUIRE(texcoordDef);
        mx::RtNode tiledimage1 = mx::RtNode::createNew("tiledimage1", tiledimageDef, stage.getObject());
        mx::RtNode texcoord1 = mx::RtNode::createNew("texcoord1", texcoordDef, stage.getObject());
        REQUIRE(tiledimage1);
        REQUIRE(texcoord1);
        mx::RtPort tiledimage1_texcoord = tiledimage1.findPort("texcoord");
        mx::RtPort texcoord1_index = texcoord1.findPort("index");
        mx::RtPort texcoord1_out = texcoord1.findPort("out");
        REQUIRE(tiledimage1_texcoord);
        REQUIRE(texcoord1_index);
        REQUIRE(texcoord1_out);
        texcoord1_out.connectTo(tiledimage1_texcoord);
        texcoord1_index.getValue().asInt() = 2;

        mx::DocumentPtr exportDoc = mx::createDocument();
        mx::RtCoreIo(stage.getObject()).write(exportDoc);
        mx::writeToXmlFile(exportDoc, stage.getName().str() + "_export.mtlx");
    }
}

TEST_CASE("Runtime: Stage References", "[runtime]")
{
    // Create a main stage.
    mx::RtObject mainStageObj = mx::RtStage::createNew("main");
    mx::RtStage mainStage(mainStageObj);

    // Load in stdlib in another stage.
    mx::RtObject libStageObj = mx::RtStage::createNew("libs");
    mx::DocumentPtr doc = mx::createDocument();
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    loadLibraries({ "stdlib", "pbrlib" }, searchPath, doc);
    mx::RtCoreIo libStageIo(libStageObj);
    libStageIo.read(doc);

    // Reference the library stage.
    mainStage.addReference(libStageObj);

    // Test access and usage of contents from the referenced library.
    mx::RtNodeDef nodedef = mainStage.findElementByName("ND_complex_ior");
    REQUIRE(nodedef.isValid());
    mx::RtObject nodeObj = mx::RtNode::createNew("complex1", nodedef.getObject(), mainStage.getObject());
    REQUIRE(nodeObj.isValid());

    // Write the stage to a new document, 
    // writing only the non-referenced content.
    mx::DocumentPtr doc2 = mx::createDocument();
    mx::RtCoreIo mainStageIo(mainStageObj);
    mainStageIo.write(doc2);
    mx::writeToXmlFile(doc2, mainStage.getName().str() + ".mtlx");

    // Make sure removal of the non-referenced node works
    const mx::RtToken nodeName = mx::RtNode(nodeObj).getName();
    mainStage.removeElement(nodeName);
    nodeObj = mainStage.findElementByName(nodeName);
    REQUIRE(!nodeObj.isValid());
}

TEST_CASE("Runtime: Traversal", "[runtime]")
{
    // Create a main stage.
    mx::RtStage mainStage = mx::RtStage::createNew("main");

    // Load stdlib in a seperate stage.
    mx::RtStage stdlibStage = mx::RtStage::createNew("stdlib");
    mx::DocumentPtr doc = mx::createDocument();
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    loadLibraries({ "stdlib", "pbrlib" }, searchPath, doc);
    mx::RtCoreIo coreIO(stdlibStage.getObject());
    coreIO.read(doc);

    // Count elements traversing the full stdlib stage
    size_t nodeCount = 0, nodeDefCount = 0, nodeGraphCount = 0;
    for (auto it = stdlibStage.traverseStage(); !it.isDone(); ++it)
    {
        switch ((*it).getObjType())
        {
        case mx::RtObjType::NODE:
            nodeCount++;
            break;
        case mx::RtObjType::NODEDEF:
            nodeDefCount++;
            break;
        case mx::RtObjType::NODEGRAPH:
            nodeGraphCount++;
            break;
        default:
            break;
        }
    }
    REQUIRE(nodeCount == doc->getNodes().size());
    REQUIRE(nodeDefCount == doc->getNodeDefs().size());
    REQUIRE(nodeGraphCount == doc->getNodeGraphs().size());

    mainStage.addReference(stdlibStage.getObject());
    
    mx::RtNodeDef nodedef = mainStage.findElementByName("ND_subtract_vector3");
    REQUIRE(nodedef);
    mx::RtObject nodeObj = mx::RtNode::createNew("sub1", nodedef.getObject(), mainStage.getObject());
    REQUIRE(nodeObj);

    // Travers using a filter to return only node objects.
    mx::RtObjectFilter<mx::RtObjType::NODE> nodeFilter;
    nodeCount = 0, nodeDefCount = 0, nodeGraphCount = 0;
    for (auto it = mainStage.traverseStage(nodeFilter); !it.isDone(); ++it)
    {
        switch ((*it).getObjType())
        {
        case mx::RtObjType::NODE:
            nodeCount++;
            break;
        case mx::RtObjType::NODEDEF:
            nodeDefCount++;
            break;
        case mx::RtObjType::NODEGRAPH:
            nodeGraphCount++;
            break;
        default:
            break;
        }
    }
    REQUIRE(nodeCount == 1);
    REQUIRE(nodeDefCount == 0);
    REQUIRE(nodeGraphCount == 0);

    // Travers using a filter to return only objects supporting the nodegraph API.
    mx::RtApiFilter<mx::RtApiType::NODEGRAPH> apiFilter;
    nodeCount = 0, nodeDefCount = 0, nodeGraphCount = 0;
    for (auto it = mainStage.traverseStage(apiFilter); !it.isDone(); ++it)
    {
        switch ((*it).getObjType())
        {
        case mx::RtObjType::NODE:
            nodeCount++;
            break;
        case mx::RtObjType::NODEDEF:
            nodeDefCount++;
            break;
        case mx::RtObjType::NODEGRAPH:
            nodeGraphCount++;
            break;
        default:
            break;
        }
    }
    REQUIRE(nodeCount == 0);
    REQUIRE(nodeDefCount == 0);
    REQUIRE(nodeGraphCount == 65);

    // Travers a nodegraph using tree traversal.
    mx::RtNodeGraph nodegraph = mainStage.findElementByName("NG_tiledimage_float");
    REQUIRE(nodegraph);
    nodeCount = 0;
    for (auto it = nodegraph.traverseTree(); !it.isDone(); ++it)
    {
        if ((*it).getObjType() == mx::RtObjType::NODE)
        {
            nodeCount++;
        }
    }
    REQUIRE(nodeCount == 3);

    // Filter for finding input ports.
    auto inputFilter = [](const mx::RtObject& obj) -> bool
    {
        if (obj.hasApi(mx::RtApiType::PORTDEF))
        {
            mx::RtPortDef portdef(obj);
            return portdef.isInput();
        }
        return false;
    };

    // Travers a nodedef finding all its inputs.
    mx::RtNodeDef generalized_schlick_brdf = mainStage.findElementByName("ND_generalized_schlick_brdf");
    REQUIRE(generalized_schlick_brdf);
    size_t inputCount = 0;
    for (auto it = generalized_schlick_brdf.traverseTree(inputFilter); !it.isDone(); ++it)
    {
        inputCount++;
    }
    REQUIRE(inputCount == 9);

    // Filter for finding nodedefs of BSDF nodes.
    auto bsdfFilter = [](const mx::RtObject& obj) -> bool
    {
        if (obj.hasApi(mx::RtApiType::NODEDEF))
        {
            mx::RtNodeDef nodedef(obj);
            return nodedef.numOutputs() == 1 && mx::RtPortDef(nodedef.getPort(0)).getType() == mx::RtType::BSDF;
        }
        return false;
    };

    // Travers to find all nodedefs for BSDF nodes.
    size_t bsdfCount = 0;
    for (auto it = mainStage.traverseStage(bsdfFilter); !it.isDone(); ++it)
    {
        bsdfCount++;
    }
    REQUIRE(bsdfCount == 14);

    // Find the output port on the nodegraph above,
    // and test traversing the graph upstream from 
    // this output.
    mx::RtPort outSocket = nodegraph.getOutputSocket(0);
    REQUIRE(outSocket.isInput());
    REQUIRE(outSocket.isConnected());
    REQUIRE(outSocket.getSourcePort());
    size_t numEdges = 0;
    for (auto it = outSocket.traverseUpstream(); !it.isDone(); ++it)
    {
        ++numEdges;
    }
    REQUIRE(numEdges == 12);
}
