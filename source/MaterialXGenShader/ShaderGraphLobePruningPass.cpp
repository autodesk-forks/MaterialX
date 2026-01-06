//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/ShaderGraphLobePruningPass.h>

#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/GenOptions.h>

#include <set>
#include <string>

MATERIALX_NAMESPACE_BEGIN

namespace {

// Standard PBR BSDF node categories from MaterialX specification
const std::set<string> kPrunableBsdfCategories = {
    "oren_nayar_diffuse_bsdf",
    "burley_diffuse_bsdf",
    "conductor_bsdf",
    "subsurface_bsdf",
    "dielectric_bsdf",
    "generalized_schlick_bsdf",
    "sheen_bsdf",
    "translucent_bsdf",
    "dielectric_tf_bsdf",
    "generalized_schlick_tf_82_bsdf",
    "sheen_zeltner_bsdf"
};

} // namespace

const string& LobePruningPass::getName() const
{
    static const string name = "LobePruning";
    return name;
}

bool LobePruningPass::run(ShaderGraph& graph, GenContext& context)
{
    (void)context; // Suppress unused parameter warning (will be used when dark closure creation is implemented)
    
    bool anyChanges = false;

    // Iterate over all nodes in the graph
    // We collect node names first since we may delete nodes during iteration
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

        // Get node category from the nodedef
        // Note: ShaderNode doesn't expose category directly, we check via classification and inputs
        
        // Check for prunable mix nodes (MIX_BSDF classification or has "mix" input)
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
                            continue;
                        }
                    }
                }
            }
        }
        
        // Check for prunable BSDF nodes with zero weight
        if (isPrunableBsdfNode(node))
        {
            ShaderInput* weightInput = node->getInput("weight");
            if (weightInput && isLobeWeightInput(weightInput))
            {
                float weight;
                if (getConstantFloatValue(weightInput, weight) && weight == 0.0f)
                {
                    replaceBsdfWithDark(graph, node, context);
                    anyChanges = true;
                }
            }
        }
    }

    return anyChanges;
}

bool LobePruningPass::isLobeWeightInput(const ShaderInput* input) const
{
    if (!input || input->getConnection())
        return false;

    // Must be float type
    if (input->getType() != Type::FLOAT)
        return false;

    // TODO: Check uimin/uimax metadata when we have access to NodeDef
    // For now, assume any float input named "weight" or "mix" could be a lobe weight
    const string& name = input->getName();
    return (name == "weight" || name == "mix");
}

bool LobePruningPass::getConstantFloatValue(const ShaderInput* input, float& outValue) const
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

bool LobePruningPass::isPrunableBsdfNode(const ShaderNode* node) const
{
    if (!node)
        return false;

    // Check if this node has BSDF classification
    if (!node->hasClassification(ShaderNode::Classification::BSDF))
        return false;

    // Check if it's one of the known prunable categories
    // TODO: Get the actual category from the node when that API is available
    return true;
}

void LobePruningPass::replaceBsdfWithDark(ShaderGraph& graph, ShaderNode* node, GenContext& context)
{
    // TODO: This requires creating "dark" BSDF node definitions and implementations
    // For now, we'll just bypass the node and propagate a default BSDF value
    // This is a placeholder - the real implementation needs dark closures
    
    // For now, just disconnect the node but don't remove it
    // (Full implementation requires dark_base_bsdf and dark_layer_bsdf nodes)
    
    // Suppress unused parameter warnings until implementation is complete
    (void)graph;
    (void)node;
    (void)context;
}

bool LobePruningPass::pruneMixNode(ShaderGraph& graph, ShaderNode* mixNode, float mixValue)
{
    // For mix(bg, fg, mix):
    // - if mix == 0, use bg input
    // - if mix == 1, use fg input
    
    const string& inputName = (mixValue == 0.0f) ? "bg" : "fg";
    ShaderInput* selectedInput = mixNode->getInput(inputName);
    
    if (!selectedInput)
        return false;

    // Get the output that downstream nodes connect to
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

bool LobePruningPass::pruneMultiplyNode(ShaderGraph& graph, ShaderNode* multiplyNode)
{
    // If any input to multiply is 0, the result is 0
    // Propagate zero to all downstream connections
    
    ShaderOutput* multiplyOutput = multiplyNode->getOutput(0);
    if (!multiplyOutput)
        return false;

    // Get the type of the multiply node to create appropriate zero value
    const TypeDesc& outputType = multiplyOutput->getType();
    
    ShaderInputVec downstreamConnections = multiplyOutput->getConnections();
    
    for (ShaderInput* downstream : downstreamConnections)
    {
        multiplyOutput->breakConnection(downstream);
        
        // Set downstream input to zero value of appropriate type
        if (outputType == Type::FLOAT)
        {
            downstream->setValue(Value::createValue<float>(0.0f));
        }
        else if (outputType == Type::COLOR3)
        {
            downstream->setValue(Value::createValue<Color3>(Color3(0.0f)));
        }
        else if (outputType == Type::VECTOR3)
        {
            downstream->setValue(Value::createValue<Vector3>(Vector3(0.0f)));
        }
        // TODO: Handle other vector/color types
    }

    // Remove the multiply node
    graph.removeNode(multiplyNode);
    return true;
}

MATERIALX_NAMESPACE_END

