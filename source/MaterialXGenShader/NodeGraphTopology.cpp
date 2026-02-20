//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/NodeGraphTopology.h>
#include <MaterialXGenShader/Exception.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Library.h>

#include <MaterialXTrace/Tracing.h>

#include <map>
#include <set>

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

//
// NodeGraphTopology - implementation detail, not exposed in the header.
// Analyzes a NodeGraph to identify "topological" inputs that when constant
// (0 or 1) can eliminate entire branches of the graph.
//

struct TopologicalInput
{
    string name;
    StringSet nodesToSkipAt0;
    StringSet nodesToSkipAt1;
    StringSet maybeDeadAt0;
    StringSet maybeDeadAt1;
};

class NodeGraphTopology
{
  public:
    explicit NodeGraphTopology(const NodeGraph& nodeGraph);
    std::unique_ptr<NodeGraphPermutation> createPermutation(const Node& node) const;

  private:
    static bool isTopologicalInput(const InputPtr& input, const NodeDefPtr& nodeDef);
    void analyzeAffectedNodes(const NodePtr& node, const InputPtr& input, TopologicalInput& topoInput);
    void buildRefCounts(const NodeGraph& nodeGraph);

    std::map<string, TopologicalInput> _topologicalInputs;
    std::unordered_map<string, size_t> _refCounts;
    std::unordered_map<string, StringSet> _nodeUpstreams;
};

NodeGraphTopology::NodeGraphTopology(const NodeGraph& nodeGraph)
{
    MX_TRACE_FUNCTION(Tracing::Category::ShaderGen);
    MX_TRACE_SCOPE(Tracing::Category::ShaderGen, nodeGraph.getName().c_str());

    NodeDefPtr nodeDef = nodeGraph.getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Can't find nodedef for nodegraph '" + nodeGraph.getName() + "'");
    }

    // Build reference counts and upstream dependency map
    buildRefCounts(nodeGraph);

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
                if (_topologicalInputs.find(interfaceName) == _topologicalInputs.end())
                {
                    TopologicalInput topoInput;
                    topoInput.name = interfaceName;
                    analyzeAffectedNodes(node, mixInput, topoInput);
                    _topologicalInputs[interfaceName] = topoInput;
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
                    if (_topologicalInputs.find(interfaceName) == _topologicalInputs.end())
                    {
                        TopologicalInput topoInput;
                        topoInput.name = interfaceName;
                        analyzeAffectedNodes(node, input, topoInput);
                        _topologicalInputs[interfaceName] = topoInput;
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
                if (_topologicalInputs.find(interfaceName) == _topologicalInputs.end())
                {
                    TopologicalInput topoInput;
                    topoInput.name = interfaceName;
                    analyzeAffectedNodes(node, weightInput, topoInput);
                    _topologicalInputs[interfaceName] = topoInput;
                }
            }
        }
    }
}

bool NodeGraphTopology::isTopologicalInput(const InputPtr& input, const NodeDefPtr& nodeDef)
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
    TopologicalInput& topoInput)
{
    const string& category = node->getCategory();

    if (category == "mix")
    {
        // For mix nodes:
        // - When mix=0, the "fg" (foreground) branch loses a consumer
        // - When mix=1, the "bg" (background) branch loses a consumer
        // The mix node itself stays alive; we just decrement the unused input's upstream ref count
        InputPtr bgInput = node->getActiveInput("bg");
        InputPtr fgInput = node->getActiveInput("fg");

        if (fgInput && fgInput->hasNodeName())
        {
            // mix=0 means fg branch loses this consumer
            topoInput.maybeDeadAt0.insert(fgInput->getNodeName());
        }

        if (bgInput && bgInput->hasNodeName())
        {
            // mix=1 means bg branch loses this consumer
            topoInput.maybeDeadAt1.insert(bgInput->getNodeName());
        }
    }
    else if (category == "multiply")
    {
        // For multiply nodes with input=0:
        // The multiply node stays alive (outputs 0), but the other inputs lose a consumer
        for (const InputPtr& otherInput : node->getActiveInputs())
        {
            if (otherInput != input && otherInput->hasNodeName())
            {
                topoInput.maybeDeadAt0.insert(otherInput->getNodeName());
            }
        }
    }
    else if (kBasePbrNodes.count(category) || kLayerPbrNodes.count(category))
    {
        // For PBR nodes with weight=0:
        // The PBR node itself is unconditionally skipped (replaced with dark/transparent)
        // Its upstream dependencies will be handled via ref count propagation
        topoInput.nodesToSkipAt0.insert(node->getName());
    }
}

void NodeGraphTopology::buildRefCounts(const NodeGraph& nodeGraph)
{
    // Build reference counts: how many downstream nodes consume each node's output
    // Also build upstream dependency map for efficient propagation

    for (const NodePtr& node : nodeGraph.getNodes())
    {
        const string& nodeName = node->getName();
        StringSet upstreams;

        for (const InputPtr& input : node->getActiveInputs())
        {
            if (input->hasNodeName())
            {
                const string& upstreamName = input->getNodeName();
                _refCounts[upstreamName]++;
                upstreams.insert(upstreamName);
            }
        }

        if (!upstreams.empty())
        {
            _nodeUpstreams[nodeName] = std::move(upstreams);
        }
    }

    // Count references from NodeGraph outputs (these are the "roots" of the graph)
    for (const OutputPtr& output : nodeGraph.getOutputs())
    {
        if (output->hasNodeName())
        {
            _refCounts[output->getNodeName()]++;
        }
    }
}

std::unique_ptr<NodeGraphPermutation> NodeGraphTopology::createPermutation(const Node& node) const
{
    MX_TRACE_FUNCTION(Tracing::Category::ShaderGen);

    if (_topologicalInputs.empty())
    {
        return nullptr;
    }

    string key;
    StringSet skipNodes;
    bool hasOptimization = false;

    // Working copy of reference counts for this permutation
    std::unordered_map<string, size_t> refCounts = _refCounts;

    // Worklist for propagating death through the graph
    std::vector<string> worklist;

    // First pass: build the key and collect initial dead nodes
    for (const auto& pair : _topologicalInputs)
    {
        const string& inputName = pair.first;
        const TopologicalInput& topoInput = pair.second;
        char flag = 'x';  // 'x' = not optimized (connected or intermediate value)

        // Check if this input is connected on the node instance
        InputPtr nodeInput = node.getInput(inputName);
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

                    // Unconditionally skip these nodes
                    for (const string& n : topoInput.nodesToSkipAt0)
                    {
                        if (skipNodes.find(n) == skipNodes.end())
                        {
                            skipNodes.insert(n);
                            worklist.push_back(n);
                        }
                    }

                    // These nodes lose a consumer - decrement their ref count
                    for (const string& n : topoInput.maybeDeadAt0)
                    {
                        auto it = refCounts.find(n);
                        if (it != refCounts.end() && it->second > 0)
                        {
                            it->second--;
                            if (it->second == 0 && skipNodes.find(n) == skipNodes.end())
                            {
                                skipNodes.insert(n);
                                worklist.push_back(n);
                            }
                        }
                    }
                }
                else if (value == 1.0f)
                {
                    flag = '1';
                    hasOptimization = true;

                    // Unconditionally skip these nodes
                    for (const string& n : topoInput.nodesToSkipAt1)
                    {
                        if (skipNodes.find(n) == skipNodes.end())
                        {
                            skipNodes.insert(n);
                            worklist.push_back(n);
                        }
                    }

                    // These nodes lose a consumer - decrement their ref count
                    for (const string& n : topoInput.maybeDeadAt1)
                    {
                        auto it = refCounts.find(n);
                        if (it != refCounts.end() && it->second > 0)
                        {
                            it->second--;
                            if (it->second == 0 && skipNodes.find(n) == skipNodes.end())
                            {
                                skipNodes.insert(n);
                                worklist.push_back(n);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // Input not set on node instance - check NodeDef default value
            NodeDefPtr nodeDef = node.getNodeDef();
            if (nodeDef)
            {
                InputPtr defaultInput = nodeDef->getActiveInput(inputName);
                if (defaultInput && defaultInput->hasValue())
                {
                    float value = defaultInput->getValue()->asA<float>();
                    if (value == 0.0f)
                    {
                        flag = '0';
                        hasOptimization = true;

                        // Unconditionally skip these nodes
                        for (const string& n : topoInput.nodesToSkipAt0)
                        {
                            if (skipNodes.find(n) == skipNodes.end())
                            {
                                skipNodes.insert(n);
                                worklist.push_back(n);
                            }
                        }

                        // These nodes lose a consumer - decrement their ref count
                        for (const string& n : topoInput.maybeDeadAt0)
                        {
                            auto it = refCounts.find(n);
                            if (it != refCounts.end() && it->second > 0)
                            {
                                it->second--;
                                if (it->second == 0 && skipNodes.find(n) == skipNodes.end())
                                {
                                    skipNodes.insert(n);
                                    worklist.push_back(n);
                                }
                            }
                        }
                    }
                    else if (value == 1.0f)
                    {
                        flag = '1';
                        hasOptimization = true;

                        // Unconditionally skip these nodes
                        for (const string& n : topoInput.nodesToSkipAt1)
                        {
                            if (skipNodes.find(n) == skipNodes.end())
                            {
                                skipNodes.insert(n);
                                worklist.push_back(n);
                            }
                        }

                        // These nodes lose a consumer - decrement their ref count
                        for (const string& n : topoInput.maybeDeadAt1)
                        {
                            auto it = refCounts.find(n);
                            if (it != refCounts.end() && it->second > 0)
                            {
                                it->second--;
                                if (it->second == 0 && skipNodes.find(n) == skipNodes.end())
                                {
                                    skipNodes.insert(n);
                                    worklist.push_back(n);
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!key.empty())
        {
            key += ",";
        }
        key += inputName + "=" + flag;
    }

    // Propagate death through the graph using reference counting
    // When a node dies, decrement ref counts of all its upstream dependencies
    // If any upstream's ref count hits 0, it's also dead
    while (!worklist.empty())
    {
        string nodeName = worklist.back();
        worklist.pop_back();

        // Find upstream dependencies of this node
        auto upstreamIt = _nodeUpstreams.find(nodeName);
        if (upstreamIt != _nodeUpstreams.end())
        {
            for (const string& upstream : upstreamIt->second)
            {
                auto refIt = refCounts.find(upstream);
                if (refIt != refCounts.end() && refIt->second > 0)
                {
                    refIt->second--;
                    if (refIt->second == 0 && skipNodes.find(upstream) == skipNodes.end())
                    {
                        skipNodes.insert(upstream);
                        worklist.push_back(upstream);
                    }
                }
            }
        }
    }

    if (!hasOptimization)
    {
        return nullptr;
    }

    return std::unique_ptr<NodeGraphPermutation>(
        new NodeGraphPermutation(std::move(key), std::move(skipNodes)));
}

//
// NodeGraphTopologyCache implementation
//

NodeGraphTopologyCache::~NodeGraphTopologyCache() = default;

NodeGraphTopologyCache& NodeGraphTopologyCache::instance()
{
    static NodeGraphTopologyCache theInstance;
    return theInstance;
}

std::unique_ptr<NodeGraphPermutation> NodeGraphTopologyCache::createPermutation(
    const NodeGraph& nodeGraph, const Node& node)
{
    const NodeGraphTopology& topology = getTopology(nodeGraph);
    return topology.createPermutation(node);
}

const NodeGraphTopology& NodeGraphTopologyCache::getTopology(const NodeGraph& nodeGraph)
{
    MX_TRACE_FUNCTION(Tracing::Category::ShaderGen);
    const string& ngName = nodeGraph.getName();

    // Check cache first (with lock)
    {
        std::lock_guard<std::mutex> lock(_cacheMutex);
        auto it = _cache.find(ngName);
        if (it != _cache.end())
        {
            return *it->second;
        }
    }

    // Cache miss - construct outside lock to allow parallel construction
    // of different topologies. Safe because emplace() won't overwrite.
    auto topology = std::make_unique<NodeGraphTopology>(nodeGraph);

    std::lock_guard<std::mutex> lock(_cacheMutex);
    auto [it, inserted] = _cache.emplace(ngName, std::move(topology));
    return *it->second;
}

MATERIALX_NAMESPACE_END
