//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/ShaderGraphStandardPasses.h>

#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/GenOptions.h>

#include <set>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

//
// ConstantFoldingPass
//

const string& ConstantFoldingPass::getName() const
{
    static const string name = "ConstantFolding";
    return name;
}

bool ConstantFoldingPass::run(ShaderGraph& graph, GenContext& context)
{
    size_t numEdits = 0;
    
    for (ShaderNode* node : graph.getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::CONSTANT))
        {
            // Constant nodes can be elided by moving their value downstream.
            bool canElide = context.getOptions().elideConstantNodes;
            if (!canElide)
            {
                // We always elide filename constant nodes regardless of the option
                ShaderInput* in = node->getInput("value");
                if (in && in->getType() == Type::FILENAME)
                {
                    canElide = true;
                }
            }
            if (canElide)
            {
                graph.bypass(node, 0);
                ++numEdits;
            }
        }
        else if (node->hasClassification(ShaderNode::Classification::DOT))
        {
            // Filename dot nodes must be elided so they do not create extra samplers.
            ShaderInput* in = node->getInput("in");
            if (in && in->getType() == Type::FILENAME)
            {
                graph.bypass(node, 0);
                ++numEdits;
            }
        }
    }

    if (numEdits > 0)
    {
        // Dead code elimination: Remove nodes that are no longer connected
        std::set<ShaderNode*> usedNodesSet;
        std::vector<ShaderNode*> usedNodesVec;

        // Traverse the graph to find nodes still in use
        for (ShaderGraphOutputSocket* outputSocket : graph.getOutputSockets())
        {
            ShaderOutput* upstreamPort = outputSocket->getConnection();
            if (upstreamPort && upstreamPort->getNode() != &graph)
            {
                for (ShaderGraphEdge edge : ShaderGraph::traverseUpstream(upstreamPort))
                {
                    ShaderNode* node = edge.upstream->getNode();
                    if (usedNodesSet.count(node) == 0)
                    {
                        usedNodesSet.insert(node);
                        usedNodesVec.push_back(node);
                    }
                }
            }
        }

        // Remove any unused nodes
        std::vector<ShaderNode*> allNodes = graph.getNodes();
        for (ShaderNode* node : allNodes)
        {
            if (usedNodesSet.count(node) == 0)
            {
                graph.removeNode(node);
            }
        }
    }

    return numEdits > 0;
}

//
// PremultipliedAddPass
//

const string& PremultipliedAddPass::getName() const
{
    static const string name = "PremultipliedAdd";
    return name;
}

bool PremultipliedAddPass::run(ShaderGraph& graph, GenContext& context)
{
    // We store node names (not raw pointers) because optimization may delete nodes
    // via shared_ptr. Raw pointers would dangle.
    vector<string> mixBsdfNodeNames;
    for (ShaderNode* node : graph.getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::MIX_BSDF))
        {
            mixBsdfNodeNames.push_back(node->getName());
        }
    }

    bool anyChanges = false;
    for (const string& name : mixBsdfNodeNames)
    {
        // Look up fresh each iteration - node may have been removed by prior optimization
        ShaderNode* node = graph.getNode(name);
        if (!node)
            continue;

        if (optimizeMixBsdf(graph, node, context))
        {
            anyChanges = true;
        }
    }

    return anyChanges;
}

// Helper function to redirect the incoming connection from one input port to another.
static void redirectInput(ShaderInput* fromPort, ShaderInput* toPort)
{
    ShaderOutput* connection = fromPort->getConnection();
    if (connection)
    {
        // we have a connection - so transfer it
        toPort->makeConnection(connection);
    }
    else
    {
        // we just remap the value.
        toPort->setValue(fromPort->getValue());
    }
}

// Helper function to connect two nodes together
static void connectNodes(ShaderNode* fromNode, const string& fromPortName, 
                        ShaderNode* toNode, const string& toPortName)
{
    ShaderOutput* fromPort = fromNode->getOutput(fromPortName);
    ShaderInput* toPort = toNode->getInput(toPortName);
    if (!fromPort || !toPort)
        return;

    fromPort->makeConnection(toPort);
}

bool PremultipliedAddPass::optimizeMixBsdf(ShaderGraph& graph, ShaderNode* mixNode, GenContext& context)
{
    ShaderInput* mixFgInput = mixNode->getInput("fg");
    ShaderInput* mixBgInput = mixNode->getInput("bg");

    if (!mixFgInput || !mixBgInput)
        return false;

    ShaderNode* fgNode = mixFgInput->getConnectedSibling();
    ShaderNode* bgNode = mixBgInput->getConnectedSibling();

    if (!fgNode && !bgNode)
        return false;

    ShaderInput* fgNodeWeightInput = fgNode->getInput("weight");
    ShaderInput* bgNodeWeightInput = bgNode->getInput("weight");

    if (!fgNodeWeightInput || !bgNodeWeightInput)
        return false;

    ConstNodeDefPtr addBsdfNodeDef = graph.getDocument()->getNodeDef("ND_add_bsdf");
    ConstNodeDefPtr floatInvertNodeDef = graph.getDocument()->getNodeDef("ND_invert_float");
    ConstNodeDefPtr floatMultNodeDef = graph.getDocument()->getNodeDef("ND_multiply_float");
    if (!addBsdfNodeDef || !floatInvertNodeDef || !floatMultNodeDef)
        return false;

    ShaderInput* mixWeightInput = mixNode->getInput("mix");

    // create a node that represents the inverted mix value, ie. 1.0-mix
    ShaderNode* invertMixNode = graph.createNode(mixNode->getName()+"_INV", floatInvertNodeDef, context);
    redirectInput(mixWeightInput, invertMixNode->getInput("in"));

    // create a multiply node to calculate the new weight value, weighted by the mix value.
    ShaderNode* multFgWeightNode = graph.createNode(mixNode->getName()+"_MULT_FG", floatMultNodeDef, context);
    redirectInput(fgNodeWeightInput, multFgWeightNode->getInput("in1"));
    redirectInput(mixWeightInput, multFgWeightNode->getInput("in2"));

    // create a multiply node to calculate the new weight value, weighted by the inverted mix value.
    ShaderNode* multBgWeightNode = graph.createNode(mixNode->getName()+"_MULT_BG", floatMultNodeDef, context);
    redirectInput(bgNodeWeightInput, multBgWeightNode->getInput("in1"));
    connectNodes(invertMixNode, "out", multBgWeightNode, "in2");

    // connect the two newly created weights to the fg and bg BSDF nodes.
    connectNodes(multFgWeightNode, "out", fgNode, "weight");
    connectNodes(multBgWeightNode, "out", bgNode, "weight");

    // Create the ND_add_bsdf node that will add the two BSDF nodes with the modified weights
    ShaderNode* addNode = graph.createNode(mixNode->getName()+"_ADD", addBsdfNodeDef, context);
    connectNodes(bgNode, "out", addNode, "in1");
    connectNodes(fgNode, "out", addNode, "in2");

    // Finally for all the previous outgoing connections from the original mix node
    // replace those with the outgoing connection from the new add node.
    ShaderOutput* mixNodeOutput = mixNode->getOutput("out");
    ShaderInputVec mixNodeOutputConns = mixNodeOutput->getConnections();
    for (ShaderInput* conn : mixNodeOutputConns)
    {
        addNode->getOutput("out")->makeConnection(conn);
    }

    graph.removeNode(mixNode);

    return true;
}

MATERIALX_NAMESPACE_END

