//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/NodeGraphTopology.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Library.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{
// Base PBR nodes - when weight=0, output is completely dark/opaque
const std::set<string> kBasePbrNodes = {
    "oren_nayar_diffuse_bsdf",
    "compensating_oren_nayar_diffuse_bsdf",
    "burley_diffuse_bsdf",
    "conductor_bsdf",
    "subsurface_bsdf",
    "translucent_bsdf",
};

// Layer PBR nodes - when weight=0, output is transparent (pass-through)
const std::set<string> kLayerPbrNodes = {
    "dielectric_bsdf",
    "generalized_schlick_bsdf",
    "sheen_bsdf",
    "dielectric_tf_bsdf",
    "generalized_schlick_tf_82_bsdf",
    "sheen_zeltner_bsdf",
};

} // anonymous namespace

NodeGraphTopology& NodeGraphTopology::instance()
{
    static NodeGraphTopology theInstance;
    return theInstance;
}

const NodeGraphTopology::Analysis& NodeGraphTopology::analyze(const NodeGraph& nodeGraph)
{
    const string& ngName = nodeGraph.getName();

    // Check cache first (with lock)
    {
        std::lock_guard<std::mutex> lock(_cacheMutex);
        auto it = _cache.find(ngName);
        if (it != _cache.end())
        {
            return it->second;
        }
    }

    // Not in cache - perform analysis
    Analysis analysis;
    analysis.nodeGraphName = ngName;

    // Get the NodeDef for this NodeGraph
    NodeDefPtr nodeDef = nodeGraph.getNodeDef();
    if (!nodeDef)
    {
        // Try to find via nodeDefString
        const string& nodeDefString = nodeGraph.getNodeDefString();
        if (!nodeDefString.empty())
        {
            ConstDocumentPtr doc = nodeGraph.getDocument();
            nodeDef = doc->getNodeDef(nodeDefString);
        }
    }

    if (!nodeDef)
    {
        // Can't analyze without a NodeDef - cache empty result
        std::lock_guard<std::mutex> lock(_cacheMutex);
        _cache[ngName] = analysis;
        return _cache[ngName];
    }

    // Scan all nodes in the NodeGraph for optimization opportunities
    for (const NodePtr& node : nodeGraph.getNodes())
    {
        const string& category = node->getCategory();

        if (category == "mix")
        {
            // Mix nodes: mix input at 0 or 1 can eliminate a branch
            InputPtr mixInput = node->getActiveInput("mix");
            if (mixInput && isTopologicalInput(mixInput, nodeDef))
            {
                const string& interfaceName = mixInput->getInterfaceName();
                if (analysis.topologicalInputs.find(interfaceName) == analysis.topologicalInputs.end())
                {
                    TopologicalInput topoInput;
                    topoInput.name = interfaceName;
                    analyzeAffectedNodes(node, mixInput, topoInput, nodeGraph);
                    analysis.topologicalInputs[interfaceName] = topoInput;
                }
            }
        }
        else if (category == "multiply")
        {
            // Multiply nodes: any float input at 0 zeroes the output
            for (const InputPtr& input : node->getActiveInputs())
            {
                if (input && isTopologicalInput(input, nodeDef))
                {
                    const string& interfaceName = input->getInterfaceName();
                    if (analysis.topologicalInputs.find(interfaceName) == analysis.topologicalInputs.end())
                    {
                        TopologicalInput topoInput;
                        topoInput.name = interfaceName;
                        analyzeAffectedNodes(node, input, topoInput, nodeGraph);
                        analysis.topologicalInputs[interfaceName] = topoInput;
                    }
                }
            }
        }
        else if (kBasePbrNodes.count(category) || kLayerPbrNodes.count(category))
        {
            // PBR nodes: weight at 0 makes the node output dark/transparent
            InputPtr weightInput = node->getActiveInput("weight");
            if (weightInput && isTopologicalInput(weightInput, nodeDef))
            {
                const string& interfaceName = weightInput->getInterfaceName();
                if (analysis.topologicalInputs.find(interfaceName) == analysis.topologicalInputs.end())
                {
                    TopologicalInput topoInput;
                    topoInput.name = interfaceName;
                    analyzeAffectedNodes(node, weightInput, topoInput, nodeGraph);
                    analysis.topologicalInputs[interfaceName] = topoInput;
                }
            }
        }
    }

    // Cache and return
    std::lock_guard<std::mutex> lock(_cacheMutex);
    _cache[ngName] = analysis;
    return _cache[ngName];
}

const NodeGraphTopology::Analysis* NodeGraphTopology::getAnalysis(const string& nodeGraphName) const
{
    std::lock_guard<std::mutex> lock(_cacheMutex);
    auto it = _cache.find(nodeGraphName);
    return (it != _cache.end()) ? &it->second : nullptr;
}

bool NodeGraphTopology::isTopologicalInput(const InputPtr& input, const NodeDefPtr& nodeDef) const
{
    // Must be connected to the NodeGraph interface
    if (!input->hasInterfaceName())
    {
        return false;
    }

    // Must be float type
    if (input->getType() != "float")
    {
        return false;
    }

    // Get the corresponding NodeDef input
    const string& interfaceName = input->getInterfaceName();
    InputPtr ndInput = nodeDef->getActiveInput(interfaceName);
    if (!ndInput)
    {
        return false;
    }

    // Check for uimin=0, uimax=1 (indicates a 0-1 weight parameter)
    if (!ndInput->hasAttribute("uimin") || !ndInput->hasAttribute("uimax"))
    {
        return false;
    }

    try
    {
        float minVal = std::stof(ndInput->getAttribute("uimin"));
        float maxVal = std::stof(ndInput->getAttribute("uimax"));
        return (minVal == 0.0f && maxVal == 1.0f);
    }
    catch (...)
    {
        return false;
    }
}

void NodeGraphTopology::analyzeAffectedNodes(
    const NodePtr& node,
    const InputPtr& input,
    TopologicalInput& topoInput,
    const NodeGraph& nodeGraph) const
{
    const string& category = node->getCategory();

    if (category == "mix")
    {
        // For mix nodes:
        // - When mix=0, the "fg" (foreground) branch is unused
        // - When mix=1, the "bg" (background) branch is unused
        InputPtr bgInput = node->getActiveInput("bg");
        InputPtr fgInput = node->getActiveInput("fg");

        if (fgInput && fgInput->hasNodeName())
        {
            // mix=0 means fg branch is dead
            collectUpstreamNodes(fgInput->getNodeName(), nodeGraph, topoInput.nodesAffectedAt0);
        }

        if (bgInput && bgInput->hasNodeName())
        {
            // mix=1 means bg branch is dead
            collectUpstreamNodes(bgInput->getNodeName(), nodeGraph, topoInput.nodesAffectedAt1);
        }
    }
    else if (category == "multiply")
    {
        // For multiply nodes with input=0:
        // All upstream nodes of the other input(s) become dead
        for (const InputPtr& otherInput : node->getActiveInputs())
        {
            if (otherInput != input && otherInput->hasNodeName())
            {
                collectUpstreamNodes(otherInput->getNodeName(), nodeGraph, topoInput.nodesAffectedAt0);
            }
        }
    }
    else if (kBasePbrNodes.count(category) || kLayerPbrNodes.count(category))
    {
        // For PBR nodes with weight=0:
        // All upstream nodes feeding this PBR node become dead
        for (const InputPtr& nodeInput : node->getActiveInputs())
        {
            if (nodeInput->getName() != "weight" && nodeInput->hasNodeName())
            {
                collectUpstreamNodes(nodeInput->getNodeName(), nodeGraph, topoInput.nodesAffectedAt0);
            }
        }
        // Also include the PBR node itself (it gets replaced with a "dark" node)
        topoInput.nodesAffectedAt0.insert(node->getName());
    }
}

std::map<string, StringSet> NodeGraphTopology::buildReverseConnectionMap(
    const NodeGraph& nodeGraph) const
{
    std::map<string, StringSet> reverseMap;

    for (const NodePtr& node : nodeGraph.getNodes())
    {
        for (const InputPtr& input : node->getActiveInputs())
        {
            if (input->hasNodeName())
            {
                const string& sourceName = input->getNodeName();
                reverseMap[sourceName].insert(node->getName());
            }
        }
    }

    return reverseMap;
}

void NodeGraphTopology::collectUpstreamNodes(
    const string& nodeName,
    const NodeGraph& nodeGraph,
    StringSet& collected) const
{
    // Avoid cycles
    if (collected.count(nodeName))
    {
        return;
    }

    NodePtr node = nodeGraph.getNode(nodeName);
    if (!node)
    {
        return;
    }

    collected.insert(nodeName);

    // Recursively collect upstream nodes
    for (const InputPtr& input : node->getActiveInputs())
    {
        if (input->hasNodeName())
        {
            collectUpstreamNodes(input->getNodeName(), nodeGraph, collected);
        }
    }
}

string NodeGraphTopology::computePermutationKey(const Analysis& analysis, ConstNodePtr node) const
{
    if (analysis.empty())
    {
        return EMPTY_STRING;
    }

    string key;
    bool hasOptimization = false;

    // Iterate through topological inputs in sorted order (for stable keys)
    for (const auto& pair : analysis.topologicalInputs)
    {
        const string& inputName = pair.first;
        char flag = 'x';  // 'x' = not optimized (connected or intermediate value)

        // Check if this input is connected on the node instance
        InputPtr nodeInput = node->getInput(inputName);
        if (nodeInput)
        {
            // If connected to another node, can't optimize
            if (nodeInput->hasNodeName() || nodeInput->hasOutputString() || nodeInput->hasInterfaceName())
            {
                flag = 'x';
            }
            else if (nodeInput->hasValue())
            {
                // Has a constant value - check if it's 0 or 1
                float value = nodeInput->getValue()->asA<float>();
                if (value == 0.0f)
                {
                    flag = '0';
                    hasOptimization = true;
                }
                else if (value == 1.0f)
                {
                    flag = '1';
                    hasOptimization = true;
                }
            }
        }
        else
        {
            // No input specified - use default from NodeDef
            // For now, treat as 'x' (could look up default value)
            flag = 'x';
        }

        if (!key.empty())
        {
            key += ",";
        }
        key += inputName + "=" + flag;
    }

    return hasOptimization ? key : EMPTY_STRING;
}

StringSet NodeGraphTopology::getNodesToSkip(
    const Analysis& analysis,
    const string& permutationKey) const
{
    StringSet nodesToSkip;

    if (permutationKey.empty())
    {
        return nodesToSkip;
    }

    // Parse the permutation key and collect affected nodes
    // Key format: "inputName=flag,inputName=flag,..."
    size_t pos = 0;
    while (pos < permutationKey.size())
    {
        size_t eqPos = permutationKey.find('=', pos);
        if (eqPos == string::npos) break;

        string inputName = permutationKey.substr(pos, eqPos - pos);
        char flag = permutationKey[eqPos + 1];

        auto it = analysis.topologicalInputs.find(inputName);
        if (it != analysis.topologicalInputs.end())
        {
            if (flag == '0')
            {
                nodesToSkip.insert(
                    it->second.nodesAffectedAt0.begin(),
                    it->second.nodesAffectedAt0.end());
            }
            else if (flag == '1')
            {
                nodesToSkip.insert(
                    it->second.nodesAffectedAt1.begin(),
                    it->second.nodesAffectedAt1.end());
            }
        }

        // Move to next entry
        size_t commaPos = permutationKey.find(',', eqPos);
        pos = (commaPos == string::npos) ? permutationKey.size() : commaPos + 1;
    }

    return nodesToSkip;
}

void NodeGraphTopology::clearCache()
{
    std::lock_guard<std::mutex> lock(_cacheMutex);
    _cache.clear();
}

MATERIALX_NAMESPACE_END
