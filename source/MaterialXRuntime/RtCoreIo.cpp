//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtCoreIo.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtPortDef.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtTypeDef.h>

#include <MaterialXRuntime/Private/PrvStage.h>
#include <MaterialXRuntime/Private/PrvNodeDef.h>
#include <MaterialXRuntime/Private/PrvNode.h>
#include <MaterialXRuntime/Private/PrvNodeGraph.h>

#include <sstream>

namespace MaterialX
{

namespace
{
    // Ignore lists for attributes which are handled explicitly by import.
    // These do not need to be stored as string attributes since at export 
    // they are added explicitly.
    //
    // TODO: Make into a TokenSet
    //
    static const StringSet nodedefIgnoreAttr    = { "name", "type", "node" };
    static const StringSet portdefIgnoreAttr    = { "name", "type", "value", "nodename", "output", "colorspace", "unit" };
    static const StringSet nodeIgnoreAttr       = { "name", "type", "node" };
    static const StringSet nodegraphIgnoreAttr  = { "name", "nodedef" };
    static const StringSet unknownIgnoreAttr    = { "name" };

    RtValue readValue(const ValuePtr& v, const RtToken& type, RtLargeValueStorage& storage)
    {
        if (!v)
        {
            return RtValue();
        }
        RtValue value;
        value.setValueString(type, v->getValueString(), storage);
        return value;
    }

    void readAttributes(const ElementPtr src, PrvElement* dest, const StringSet& ignoreList)
    {
        // Read in any attributes so we can export the element again
        // without loosing data. Since it's just for storage we can
        // keep the attributes as strings (tokens).
        for (const string& name : src->getAttributeNames())
        {
            if (!ignoreList.count(name))
            {
                // Save all attributes as string tokens.
                const RtToken attrName(name);
                const RtToken attrValue(src->getAttribute(name));
                dest->addAttribute(attrName, RtType::TOKEN, RtValue(attrValue));
            }
        }
    }

    void writeAttributes(const PrvElement* elem, ElementPtr dest)
    {
        for (size_t i = 0; i < elem->numAttributes(); ++i)
        {
            const RtAttribute* attr = elem->getAttribute(i);
            dest->setAttribute(attr->getName(), attr->getValue().getValueString(attr->getType()));
        }
    }

    PrvObjectHandle readNodeDef(const NodeDefPtr& src)
    {
        const RtToken name(src->getName());
        const RtToken category(src->getNodeString());

        PrvObjectHandle nodedefH = PrvNodeDef::createNew(name, category);
        PrvNodeDef* nodedef = nodedefH->asA<PrvNodeDef>();

        readAttributes(src, nodedef, nodedefIgnoreAttr);

        // Add outputs
        if (src->getOutputCount() > 0)
        {
            for (auto elem : src->getOutputs())
            {
                const RtToken portName(elem->getName());
                const RtToken portType(elem->getType());
                PrvObjectHandle outputH = PrvPortDef::createNew(portName, portType, RtValue(), 
                                                                RtPortFlag::OUTPUT);
                PrvPortDef* output = outputH->asA<PrvPortDef>();
                readAttributes(elem, output, portdefIgnoreAttr);
                nodedef->addPort(outputH);
            }
        }
        else
        {
            const RtToken type(src->getType());
            PrvObjectHandle outputH = PrvPortDef::createNew(PrvPortDef::DEFAULT_OUTPUT_NAME, type, RtValue(),
                                                            RtPortFlag::OUTPUT);
            nodedef->addPort(outputH);
        }

        // Add inputs
        for (auto elem : src->getChildrenOfType<ValueElement>())
        {
            if (elem->isA<Input>())
            {
                const RtToken portName(elem->getName());
                const RtToken portType(elem->getType());
                const RtValue portValue(readValue(elem->getValue(), portType, nodedef->getValueStorage()));

                PrvObjectHandle inputH = PrvPortDef::createNew(portName, portType, portValue);
                PrvPortDef* input = inputH->asA<PrvPortDef>();
                input->setColorSpace(RtToken(elem->getColorSpace()));
                // TODO: fix when units are implemented in core
                // input->setUnit(RtToken(elem->getUnit()));
                readAttributes(elem, input, portdefIgnoreAttr);
                nodedef->addPort(inputH);
            }
            else if (elem->isA<Parameter>())
            {
                const RtToken portName(elem->getName());
                const RtToken portType(elem->getType());
                const RtValue portValue(readValue(elem->getValue(), portType, nodedef->getValueStorage()));

                PrvObjectHandle inputH = PrvPortDef::createNew(portName, portType, portValue, RtPortFlag::UNIFORM);
                PrvPortDef* input = inputH->asA<PrvPortDef>();
                input->setColorSpace(RtToken(elem->getColorSpace()));
                // TODO: fix when units are implemented in core
                // input->setUnit(RtToken(elem->getUnit()));
                readAttributes(elem, input, portdefIgnoreAttr);
                nodedef->addPort(inputH);
            }
        }

        return nodedefH;
    }

    PrvObjectHandle readNode(const NodePtr& src, PrvStage* stage)
    {
        NodeDefPtr srcNodedef = src->getNodeDef();
        if (!srcNodedef)
        {
            throw ExceptionRuntimeError("No matching nodedef was found for node '" + src->getName() + "'");
        }

        const RtToken nodedefName(srcNodedef->getName());
        PrvObjectHandle nodedefH = stage->findChildByName(nodedefName);
        if (!nodedefH)
        {
            // NodeDef is not loaded yet so create it now.
            nodedefH = readNodeDef(srcNodedef);
            stage->addChild(nodedefH);
        }

        const RtToken nodeName(src->getName());
        PrvObjectHandle nodeH = PrvNode::createNew(nodeName, nodedefH);
        PrvNode* node = nodeH->asA<PrvNode>();

        readAttributes(src, node, nodeIgnoreAttr);

        // Copy input values.
        for (auto elem : src->getChildrenOfType<ValueElement>())
        {
            const RtToken portName(elem->getName());
            RtPort port = node->findPort(portName);
            if (!port)
            {
                throw ExceptionRuntimeError("No port named '" + elem->getName() + "' was found on runtime node '" + node->getName().str() + "'");
            }
            const RtToken portType(elem->getType());
            const RtValue portValue(readValue(elem->getValue(), portType, node->getValueStorage()));
            port.setValue(portValue);
        }

        return nodeH;
    }

    PrvObjectHandle readNodeGraph(const NodeGraphPtr& src, PrvStage* stage)
    {
        const RtToken nodegraphName(src->getName());
        PrvObjectHandle nodegraphH = PrvNodeGraph::createNew(nodegraphName);
        PrvNodeGraph* nodegraph = nodegraphH->asA<PrvNodeGraph>();

        readAttributes(src, nodegraph, nodegraphIgnoreAttr);

        bool fixedInterface = false;
        NodeDefPtr srcNodeDef = src->getNodeDef();
        if (srcNodeDef)
        {
            fixedInterface = true;

            vector<ValueElementPtr> elements = srcNodeDef->getChildrenOfType<ValueElement>();

            // Create outputs first
            for (auto elem : elements)
            {
                if (elem->isA<Output>())
                {
                    const RtToken name(elem->getName());
                    const RtToken type(elem->getType());
                    nodegraph->addPort(PrvPortDef::createNew(name, type, RtValue(), RtPortFlag::OUTPUT));
                }
            }
            // Create inputs second
            for (auto elem : elements)
            {
                if (!elem->isA<Output>())
                {
                    const RtToken name(elem->getName());
                    const RtToken type(elem->getType());
                    const uint32_t flags = elem->isA<Parameter>() ? RtPortFlag::DEFAULTS | RtPortFlag::UNIFORM : RtPortFlag::DEFAULTS;
                    nodegraph->addPort(PrvPortDef::createNew(name, type, RtValue(), flags));
                }
            }
        }
        else
        {
            // No nodedef interface was set on the graph.
            // Create all outputs here, inputs are created
            // from internal interface connections below.
            for (auto elem : src->getOutputs())
            {
                const RtToken name(elem->getName());
                const RtToken type(elem->getType());
                nodegraph->addPort(PrvPortDef::createNew(name, type, RtValue(), RtPortFlag::OUTPUT));
            }
        }

        // Create all nodes.
        for (auto child : src->getChildren())
        {
            NodePtr srcNnode = child->asA<Node>();
            if (srcNnode)
            {
                PrvObjectHandle nodeH = readNode(srcNnode, stage);
                nodegraph->addChild(nodeH);

                // Check for connections to the graph interface
                PrvNode* node = nodeH->asA<PrvNode>();
                for (auto elem : srcNnode->getChildrenOfType<ValueElement>())
                {
                    const string& interfaceName = elem->getInterfaceName();
                    if (!interfaceName.empty())
                    {
                        const RtToken internalInputName(interfaceName);
                        RtPort inputSocket = nodegraph->findInputSocket(internalInputName);
                        if (!inputSocket)
                        {
                            // Create the input on the graph
                            if (fixedInterface)
                            {
                                // The input should have been created already.
                                // This is an error so throw up.
                                throw ExceptionRuntimeError("Interface name '" + interfaceName + "' does not match an input on the nodedef set for nodegraph '" +
                                    nodegraph->getName().str() + "'");
                            }
                            const RtToken graphInputType(elem->getType());
                            const RtValue graphInputValue(readValue(elem->getValue(), graphInputType, nodegraph->getValueStorage()));
                            nodegraph->addPort(PrvPortDef::createNew(internalInputName, graphInputType, graphInputValue));
                        }
                        const RtToken inputName(elem->getName());
                        RtPort input = node->findPort(inputName);
                        PrvNode::connect(inputSocket, input);
                    }
                }
            }
        }

        // Create all connections.
        std::set<Edge> processedEdges;
        std::set<Element*> processedInterfaces;
        for (auto elem : src->getOutputs())
        {
            for (Edge edge : elem->traverseGraph())
            {
                if (processedEdges.count(edge))
                {
                    continue;
                }

                ElementPtr downstreamElem = edge.getDownstreamElement();
                ElementPtr connectingElem = edge.getConnectingElement();
                ElementPtr upstreamElem = edge.getUpstreamElement();

                if (upstreamElem->isA<Node>() && !processedInterfaces.count(upstreamElem.get()))
                {
                    const RtToken upstreamNodeName(upstreamElem->getName());
                    PrvNode* upstreamNode = nodegraph->findNode(upstreamNodeName);

                    if (downstreamElem->isA<Output>())
                    {
                        RtPort output = upstreamNode->getPort(0); // TODO: Fixme!
                        // Single outputs can have arbitrary names,
                        // so access by index in that case.
                        RtPort outputSocket = nodegraph->numOutputs() == 1 ?
                            nodegraph->getOutputSocket(0) :
                            nodegraph->findOutputSocket(RtToken(downstreamElem->getName()));
                        PrvNode::connect(output, outputSocket);
                    }
                    else
                    {
                        const RtToken downstreamNodeName(downstreamElem->getName());
                        const RtToken downstreamInputName(connectingElem->getName());
                        PrvNode* downstreamNode = nodegraph->findNode(downstreamNodeName);
                        RtPort input = downstreamNode->findPort(downstreamInputName);
                        RtPort output = upstreamNode->getPort(0); // TODO: Fixme!
                        PrvNode::connect(output, input);
                    }

                    processedInterfaces.insert(upstreamElem.get());
                }

                processedEdges.insert(edge);
            }
        }

        return nodegraphH;
    }

    PrvObjectHandle readUnknown(const ElementPtr& src)
    {
        const RtToken name(src->getName());
        const RtToken category(src->getCategory());

        PrvObjectHandle elemH = PrvUnknown::createNew(name, category);
        PrvUnknown* elem = elemH->asA<PrvUnknown>();

        readAttributes(src, elem, unknownIgnoreAttr);

        for (auto child : src->getChildren())
        {
            elem->addChild(readUnknown(child));
        }

        return elemH;
    }

    void writeNodeDef(const PrvNodeDef* nodedef, DocumentPtr dest)
    {
        const size_t numPorts = nodedef->numPorts();
        const size_t numOutputs = nodedef->numOutputs();

        NodeDefPtr destNodeDef = dest->addNodeDef(nodedef->getName(), EMPTY_STRING, nodedef->getNodeName());
        writeAttributes(nodedef, destNodeDef);

        for (size_t i = numOutputs; i < numPorts; ++i)
        {
            const PrvPortDef* input = nodedef->getPort(i);

            ValueElementPtr destInput;
            if (input->isUniform())
            {
                destInput = destNodeDef->addParameter(input->getName(), input->getType().str());
            }
            else
            {
                destInput = destNodeDef->addInput(input->getName(), input->getType().str());
            }

            destInput->setValueString(input->getValue().getValueString(input->getType()));

            if (input->getColorSpace())
            {
                destInput->setColorSpace(input->getColorSpace().str());
            }
            if (input->getUnit())
            {
                // TODO: fix when units are implemented in core.
                // destInput->setUnit(input->getUnit().str());
            }

            writeAttributes(input, destInput);
        }
        for (size_t i = 0; i < numOutputs; ++i)
        {
            const PrvPortDef* output = nodedef->getPort(i);
            OutputPtr destOutput = destNodeDef->addOutput(output->getName(), output->getType().str());
            writeAttributes(output, destOutput);
        }
    }

    template<typename T>
    void writeNode(const PrvNode* node, T dest)
    {
        const PrvNodeDef* nodedef = node->nodedef();

        const string type = nodedef->numOutputs() == 1 ? nodedef->getPort(0)->getType().str() : "multioutput";
        NodePtr destNode = dest->addNode(nodedef->getNodeName(), node->getName().str(), type);

        for (size_t i = 0; i < nodedef->numInputs(); ++i)
        {
            const PrvPortDef* inputDef = nodedef->getInput(i);
            RtPort input = const_cast<PrvNode*>(node)->findPort(inputDef->getName());
            if (input.isConnected() || input.getValue() != inputDef->getValue())
            {
                ValueElementPtr valueElem;
                if (inputDef->isUniform())
                {
                    valueElem = destNode->addParameter(input.getName().str(), input.getType().str());
                    if (input.isConnected())
                    {
                        RtPort sourcePort = input.getSourcePort();
                        if (sourcePort.isSocket())
                        {
                            // This is a connection to the internal node of a graph
                            valueElem->setInterfaceName(sourcePort.getName());
                        }
                    }
                    else
                    {
                        valueElem->setValueString(input.getValueString());
                    }
                }
                else
                {
                    valueElem = destNode->addInput(input.getName().str(), input.getType().str());
                    if (input.isConnected())
                    {
                        RtPort sourcePort = input.getSourcePort();
                        if (sourcePort.isSocket())
                        {
                            // This is a connection to the internal node of a graph
                            valueElem->setInterfaceName(sourcePort.getName());
                        }
                        else
                        {
                            PrvNode* sourceNode = sourcePort.data()->asA<PrvNode>();
                            InputPtr inputElem = valueElem->asA<Input>();
                            inputElem->setNodeName(sourceNode->getName());
                            if (sourceNode->numOutputs() > 1)
                            {
                                inputElem->setOutputString(sourcePort.getName());
                            }
                        }
                    }
                    else
                    {
                        valueElem->setValueString(input.getValueString());
                    }
                }

                if (input.getColorSpace())
                {
                    valueElem->setColorSpace(input.getColorSpace().str());
                }
                if (input.getUnit())
                {
                    // TODO: fix when units are implemented in core.
                    // valueElem->setUnit(input->getUnit().str());
                }
            }
        }
        for (size_t i = 0; i < nodedef->numOutputs(); ++i)
        {
            const PrvPortDef* output = nodedef->getOutput(i);
            OutputPtr destOutput = destNode->addOutput(output->getName(), output->getType().str());
        }

        writeAttributes(node, destNode);
    }

    void writeNodeGraph(const PrvNodeGraph* nodegraph, DocumentPtr dest)
    {
        NodeGraphPtr destNodeGraph = dest->addNodeGraph(nodegraph->getName());
        writeAttributes(nodegraph, destNodeGraph);

        for (auto node : nodegraph->getChildren())
        {
            writeNode(node->asA<PrvNode>(), destNodeGraph);
        }


    }

    void writeUnknown(const PrvUnknown* unknown, ElementPtr dest)
    {
        ElementPtr unknownElem = dest->addChildOfCategory(unknown->getCategory(), unknown->getName());
        writeAttributes(unknown, unknownElem);

        for (auto child : unknown->getChildren())
        {
            writeUnknown(child->asA<PrvUnknown>(), unknownElem);
        }
    }

} // end anonymous namespace

RtCoreIo::RtCoreIo(RtObject stage) :
    RtApiBase(stage)
{
}

RtApiType RtCoreIo::getApiType() const
{
    return RtApiType::CORE_IO;
}

void RtCoreIo::read(const DocumentPtr& doc, RtCoreIo::ReadFilter filter)
{
    PrvStage* stage = data()->asA<PrvStage>();
    readAttributes(doc, stage, {});

    for (auto elem : doc->getChildren())
    {
        if (!filter || filter(elem))
        {
            PrvObjectHandle objH;
            if (elem->isA<NodeDef>())
            {
                // Make sure the nodedef has not been loaded already.
                // When reading node instances their nodedef is loaded as well.
                if (stage->findChildByName(RtToken(elem->getName())))
                {
                    continue;
                }
                objH = readNodeDef(elem->asA<NodeDef>());
            }
            else if (elem->isA<Node>())
            {
                objH = readNode(elem->asA<Node>(), stage);
            }
            else if (elem->isA<NodeGraph>())
            {
                objH = readNodeGraph(elem->asA<NodeGraph>(), stage);
            }
            else
            {
                objH = readUnknown(elem);
            }
            stage->addChild(objH);
        }
    }
}

void RtCoreIo::write(DocumentPtr& doc, RtCoreIo::WriteFilter filter)
{
    PrvStage* stage = data()->asA<PrvStage>();
    writeAttributes(stage, doc);

    for (size_t i = 0; i < stage->numChildren(); ++i)
    {
        PrvObjectHandle elem = stage->getChild(i);
        if (!filter || filter(RtObject(elem)))
        {
            if (elem->getObjType() == RtObjType::NODEDEF)
            {
                writeNodeDef(elem->asA<PrvNodeDef>(), doc);
            }
            else if (elem->getObjType() == RtObjType::NODE)
            {
                writeNode(elem->asA<PrvNode>(), doc);
            }
            else if (elem->getObjType() == RtObjType::NODEGRAPH)
            {
                writeNodeGraph(elem->asA<PrvNodeGraph>(), doc);
            }
            else if (elem->getObjType() == RtObjType::UNKNOWN)
            {
                writeUnknown(elem->asA<PrvUnknown>(), doc->asA<Element>());
            }
            else
            {
                std::stringstream str;
                str << "Exporting object type '" << size_t(elem->getObjType()) << "' is not supported";
                throw ExceptionRuntimeError(str.str());
            }
        }
    }
}

}
