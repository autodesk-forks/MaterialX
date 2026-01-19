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
    static const string name = "optReplaceBsdfMixWithLinearCombination";
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

        // Try cascaded mix-of-mix first, then fall back to simple mix
        if (!optimizeMixMixBsdf(graph, node, context))
        {
            if (optimizeMixBsdf(graph, node, context))
            {
                anyChanges = true;
            }
        }
        else
        {
            anyChanges = true;
        }
    }

    return anyChanges;
}

namespace
{

// Helper function to redirect the incoming connection from one input port to another.
void redirectInput(ShaderInput* fromPort, ShaderInput* toPort)
{
    ShaderOutput* connection = fromPort->getConnection();
    if (connection)
    {
        toPort->makeConnection(connection);
    }
    else
    {
        toPort->setValue(fromPort->getValue());
    }
}

// Helper function to connect two nodes together
void connectNodes(ShaderNode* fromNode, const string& fromPortName, 
                  ShaderNode* toNode, const string& toPortName)
{
    ShaderOutput* fromPort = fromNode->getOutput(fromPortName);
    ShaderInput* toPort = toNode->getInput(toPortName);
    if (fromPort && toPort)
    {
        fromPort->makeConnection(toPort);
    }
}

// Helper struct for required node definitions
struct PremultNodeDefs
{
    ConstNodeDefPtr addBsdf;
    ConstNodeDefPtr floatInvert;
    ConstNodeDefPtr floatMult;
    
    bool isValid() const { return addBsdf && floatInvert && floatMult; }
};

// Get required node definitions for premultiplied add optimization
PremultNodeDefs getPremultNodeDefs(ConstDocumentPtr doc)
{
    PremultNodeDefs defs;
    defs.addBsdf = doc->getNodeDef("ND_add_bsdf");
    defs.floatInvert = doc->getNodeDef("ND_invert_float");
    defs.floatMult = doc->getNodeDef("ND_multiply_float");
    return defs;
}

// Replace a mix node's output connections with a new node's output
void replaceMixWithAdd(ShaderGraph& graph, ShaderNode* mixNode, ShaderNode* addNode)
{
    ShaderOutput* mixNodeOutput = mixNode->getOutput("out");
    ShaderInputVec mixNodeOutputConns = mixNodeOutput->getConnections();
    for (ShaderInput* conn : mixNodeOutputConns)
    {
        addNode->getOutput("out")->makeConnection(conn);
    }
    graph.removeNode(mixNode);
}

} // anonymous namespace

bool PremultipliedAddPass::optimizeMixBsdf(ShaderGraph& graph, ShaderNode* mixNode, GenContext& context)
{
    ShaderInput* mixFgInput = mixNode->getInput("fg");
    ShaderInput* mixBgInput = mixNode->getInput("bg");
    if (!mixFgInput || !mixBgInput)
        return false;

    ShaderNode* fgNode = mixFgInput->getConnectedSibling();
    ShaderNode* bgNode = mixBgInput->getConnectedSibling();
    if (!fgNode || !bgNode)
        return false;

    ShaderInput* fgNodeWeightInput = fgNode->getInput("weight");
    ShaderInput* bgNodeWeightInput = bgNode->getInput("weight");
    if (!fgNodeWeightInput || !bgNodeWeightInput)
        return false;

    PremultNodeDefs defs = getPremultNodeDefs(graph.getDocument());
    if (!defs.isValid())
        return false;

    ShaderInput* mixWeightInput = mixNode->getInput("mix");
    const string& baseName = mixNode->getName();

    // Create inverted mix weight: (1.0 - mix)
    ShaderNode* invertMixNode = graph.createNode(baseName + "_INV", defs.floatInvert, context);
    redirectInput(mixWeightInput, invertMixNode->getInput("in"));

    // Create premultiplied weights
    ShaderNode* multFgNode = graph.createNode(baseName + "_MULT_FG", defs.floatMult, context);
    redirectInput(fgNodeWeightInput, multFgNode->getInput("in1"));
    redirectInput(mixWeightInput, multFgNode->getInput("in2"));

    ShaderNode* multBgNode = graph.createNode(baseName + "_MULT_BG", defs.floatMult, context);
    redirectInput(bgNodeWeightInput, multBgNode->getInput("in1"));
    connectNodes(invertMixNode, "out", multBgNode, "in2");

    // Connect premultiplied weights to BSDF nodes
    connectNodes(multFgNode, "out", fgNode, "weight");
    connectNodes(multBgNode, "out", bgNode, "weight");

    // Create add node and replace mix
    ShaderNode* addNode = graph.createNode(baseName + "_ADD", defs.addBsdf, context);
    connectNodes(bgNode, "out", addNode, "in1");
    connectNodes(fgNode, "out", addNode, "in2");

    replaceMixWithAdd(graph, mixNode, addNode);
    return true;
}

bool PremultipliedAddPass::optimizeMixMixBsdf(ShaderGraph& graph, ShaderNode* mix2Node, GenContext& context)
{
    // Pattern: mix2(fg=mix1(A, B), bg=C) where A, B, C are BSDFs with weight inputs
    
    ShaderInput* mix2FgInput = mix2Node->getInput("fg");
    ShaderInput* mix2BgInput = mix2Node->getInput("bg");
    if (!mix2FgInput || !mix2BgInput)
        return false;

    ShaderNode* mix1Node = mix2FgInput->getConnectedSibling();
    ShaderNode* cNode = mix2BgInput->getConnectedSibling();
    if (!mix1Node || !cNode)
        return false;

    // fg must be another mix_bsdf
    if (!mix1Node->hasClassification(ShaderNode::Classification::MIX_BSDF))
        return false;

    ShaderInput* mix1FgInput = mix1Node->getInput("fg");
    ShaderInput* mix1BgInput = mix1Node->getInput("bg");
    if (!mix1FgInput || !mix1BgInput)
        return false;

    ShaderNode* aNode = mix1FgInput->getConnectedSibling();
    ShaderNode* bNode = mix1BgInput->getConnectedSibling();
    if (!aNode || !bNode)
        return false;

    // All three leaf BSDFs must have weight inputs
    ShaderInput* aWeightInput = aNode->getInput("weight");
    ShaderInput* bWeightInput = bNode->getInput("weight");
    ShaderInput* cWeightInput = cNode->getInput("weight");
    if (!aWeightInput || !bWeightInput || !cWeightInput)
        return false;

    PremultNodeDefs defs = getPremultNodeDefs(graph.getDocument());
    if (!defs.isValid())
        return false;

    ShaderInput* mix1WeightInput = mix1Node->getInput("mix");
    ShaderInput* mix2WeightInput = mix2Node->getInput("mix");
    const string& baseName = mix2Node->getName();

    // Create inverted weights: (1-mix1) and (1-mix2)
    ShaderNode* invertMix1Node = graph.createNode(baseName + "_INV1", defs.floatInvert, context);
    redirectInput(mix1WeightInput, invertMix1Node->getInput("in"));

    ShaderNode* invertMix2Node = graph.createNode(baseName + "_INV2", defs.floatInvert, context);
    redirectInput(mix2WeightInput, invertMix2Node->getInput("in"));

    // Premultiplied weight for A: A.weight * mix1 * mix2
    ShaderNode* multA1 = graph.createNode(baseName + "_MULT_A1", defs.floatMult, context);
    redirectInput(aWeightInput, multA1->getInput("in1"));
    redirectInput(mix1WeightInput, multA1->getInput("in2"));
    
    ShaderNode* multA2 = graph.createNode(baseName + "_MULT_A2", defs.floatMult, context);
    connectNodes(multA1, "out", multA2, "in1");
    redirectInput(mix2WeightInput, multA2->getInput("in2"));

    // Premultiplied weight for B: B.weight * (1-mix1) * mix2
    ShaderNode* multB1 = graph.createNode(baseName + "_MULT_B1", defs.floatMult, context);
    redirectInput(bWeightInput, multB1->getInput("in1"));
    connectNodes(invertMix1Node, "out", multB1, "in2");
    
    ShaderNode* multB2 = graph.createNode(baseName + "_MULT_B2", defs.floatMult, context);
    connectNodes(multB1, "out", multB2, "in1");
    redirectInput(mix2WeightInput, multB2->getInput("in2"));

    // Premultiplied weight for C: C.weight * (1-mix2)
    ShaderNode* multC = graph.createNode(baseName + "_MULT_C", defs.floatMult, context);
    redirectInput(cWeightInput, multC->getInput("in1"));
    connectNodes(invertMix2Node, "out", multC, "in2");

    // Connect premultiplied weights to BSDF nodes
    connectNodes(multA2, "out", aNode, "weight");
    connectNodes(multB2, "out", bNode, "weight");
    connectNodes(multC, "out", cNode, "weight");

    // Create add nodes: add1(A, B), then add2(add1, C)
    ShaderNode* add1Node = graph.createNode(baseName + "_ADD1", defs.addBsdf, context);
    connectNodes(bNode, "out", add1Node, "in1");
    connectNodes(aNode, "out", add1Node, "in2");

    ShaderNode* add2Node = graph.createNode(baseName + "_ADD2", defs.addBsdf, context);
    connectNodes(cNode, "out", add2Node, "in1");
    connectNodes(add1Node, "out", add2Node, "in2");

    // Replace mix2's output connections with add2
    replaceMixWithAdd(graph, mix2Node, add2Node);
    
    // Remove mix1 (no longer needed)
    graph.removeNode(mix1Node);

    return true;
}

MATERIALX_NAMESPACE_END

