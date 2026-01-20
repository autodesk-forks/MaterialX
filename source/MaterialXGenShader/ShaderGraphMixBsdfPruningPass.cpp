//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/ShaderGraphMixBsdfPruningPass.h>

#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

const string& MixBsdfPruningPass::getName() const
{
    static const string name = "optPruneMixBsdf";
    return name;
}

bool MixBsdfPruningPass::run(ShaderGraph& graph, GenContext& context)
{
    (void)context;
    
    bool anyChanges = false;

    // Collect node names first since we may delete nodes during iteration
    std::vector<string> nodeNames;
    for (const ShaderNode* node : graph.getNodes())
    {
        nodeNames.push_back(node->getName());
    }

    for (const string& nodeName : nodeNames)
    {
        ShaderNode* node = graph.getNode(nodeName);
        if (!node)
            continue; // May have been deleted by a previous optimization

        // Check for prunable mix nodes (closure with "mix" input)
        ShaderInput* mixInput = node->getInput("mix");
        if (mixInput && node->hasClassification(ShaderNode::Classification::CLOSURE))
        {
            if (!mixInput->getConnection())
            {
                float mixValue;
                if (getConstantFloatValue(mixInput, mixValue))
                {
                    if (mixValue == 0.0f || mixValue == 1.0f)
                    {
                        if (pruneMixNode(graph, node, mixValue))
                        {
                            anyChanges = true;
                        }
                    }
                }
            }
        }
    }

    return anyChanges;
}

bool MixBsdfPruningPass::getConstantFloatValue(const ShaderInput* input, float& outValue) const
{
    if (!input || input->getConnection())
        return false;

    if (!input->getValue())
        return false;

    if (input->getType() == Type::FLOAT)
    {
        outValue = input->getValue()->asA<float>();
        return true;
    }

    return false;
}

bool MixBsdfPruningPass::pruneMixNode(ShaderGraph& graph, ShaderNode* mixNode, float mixValue)
{
    // For mix(bg, fg, mix):
    // - if mix == 0, use bg input
    // - if mix == 1, use fg input
    
    const string& inputName = (mixValue == 0.0f) ? "bg" : "fg";
    ShaderInput* selectedInput = mixNode->getInput(inputName);
    
    if (!selectedInput)
        return false;

    ShaderOutput* mixOutput = mixNode->getOutput(0);
    if (!mixOutput)
        return false;

    // Redirect all downstream connections
    ShaderOutput* upstream = selectedInput->getConnection();
    ShaderInputVec downstreamConnections = mixOutput->getConnections();
    
    for (ShaderInput* downstream : downstreamConnections)
    {
        mixOutput->breakConnection(downstream);
        
        if (upstream)
        {
            downstream->makeConnection(upstream);
        }
        else
        {
            // No upstream connection, use the value from selectedInput
            downstream->setValue(selectedInput->getValue());
            downstream->setPath(selectedInput->getPath());
        }
    }

    // Remove the mix node
    graph.removeNode(mixNode);
    return true;
}

MATERIALX_NAMESPACE_END

