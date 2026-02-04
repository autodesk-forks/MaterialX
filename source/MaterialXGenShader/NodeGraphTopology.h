//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_NODEGRAPHTOPOLOGY_H
#define MATERIALX_NODEGRAPHTOPOLOGY_H

#include <MaterialXGenShader/Export.h>

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Node.h>

#include <map>
#include <mutex>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

/// @class NodeGraphTopology
/// Topology analysis result for a single NodeGraph.
/// Identifies "topological" inputs - inputs that when constant (0 or 1)
/// can eliminate entire branches of the graph.
/// The analysis is performed in the constructor for better encapsulation.
class MX_GENSHADER_API NodeGraphTopology
{
  public:
    /// Information about a single topological input
    struct TopologicalInput
    {
        string name;                   ///< Input name on the NodeGraph interface
        StringSet nodesAffectedAt0;    ///< Nodes to prune when value = 0
        StringSet nodesAffectedAt1;    ///< Nodes to prune when value = 1
    };

    /// Construct and analyze a NodeGraph's topology.
    /// @param nodeGraph The NodeGraph to analyze
    explicit NodeGraphTopology(const NodeGraph& nodeGraph);

    /// Default constructor for empty topology (used internally)
    NodeGraphTopology() = default;

    const string& getNodeGraphName() const { return _nodeGraphName; }
    const std::map<string, TopologicalInput>& getTopologicalInputs() const { return _topologicalInputs; }
    
    bool empty() const { return _topologicalInputs.empty(); }

    /// Compute a permutation key based on constant input values.
    /// @param node The node instance (to read constant values from)
    /// @return A string key like "coat=0,sheen=x" or empty if no optimization possible
    string computePermutationKey(ConstNodePtr node) const;

    /// Get the set of nodes to skip for a given permutation key.
    /// @param permutationKey The key from computePermutationKey()
    /// @return Set of node names that can be skipped
    StringSet getNodesToSkip(const string& permutationKey) const;

  private:
    /// Check if an input qualifies as "topological" (can gate branches)
    static bool isTopologicalInput(const InputPtr& input, const NodeDefPtr& nodeDef);

    /// Analyze which nodes are affected when a mix/multiply/bsdf input is 0 or 1
    void analyzeAffectedNodes(
        const NodePtr& node,
        const InputPtr& input,
        TopologicalInput& topoInput,
        const NodeGraph& nodeGraph);

    /// Recursively find all upstream nodes from a given node
    static void collectUpstreamNodes(
        const string& nodeName,
        const NodeGraph& nodeGraph,
        StringSet& collected);

    string _nodeGraphName;
    std::map<string, TopologicalInput> _topologicalInputs;  ///< Keyed by input name
};

/// @class NodeGraphPermutation
/// Represents a specific permutation of a NodeGraph based on call-site input values.
/// Created from a shared NodeGraphTopology and a specific node instance.
/// Lightweight object used for cache key computation before ShaderNodeImpl creation.
class MX_GENSHADER_API NodeGraphPermutation
{
  public:
    /// Construct a permutation from topology analysis and a node instance.
    /// Computes the permutation key and skip nodes based on constant input values.
    /// @param topology The shared topology analysis for this NodeGraph
    /// @param node The node instance (call site) to read input values from
    NodeGraphPermutation(const NodeGraphTopology& topology, ConstNodePtr node);

    /// Return the permutation key (e.g., "coat=0,sheen=x").
    /// Empty string means no optimization possible.
    const string& getKey() const { return _key; }

    /// Return the set of nodes to skip for this permutation.
    const StringSet& getSkipNodes() const { return _skipNodes; }

    /// Check if this permutation has any optimizations.
    bool hasOptimizations() const { return !_skipNodes.empty(); }

  private:
    string _key;
    StringSet _skipNodes;
};

/// @class NodeGraphTopologyCache
/// Caches NodeGraphTopology analyses per NodeGraph definition.
/// Thread-safe singleton for use during shader generation.
/// The cache only manages storage/lookup; analysis logic lives in NodeGraphTopology.
class MX_GENSHADER_API NodeGraphTopologyCache
{
  public:
    NodeGraphTopologyCache() = default;
    ~NodeGraphTopologyCache() = default;

    // Non-copyable, non-movable (singleton-like usage)
    NodeGraphTopologyCache(const NodeGraphTopologyCache&) = delete;
    NodeGraphTopologyCache& operator=(const NodeGraphTopologyCache&) = delete;

    /// Get the global instance
    static NodeGraphTopologyCache& instance();

    /// Get or create the topology for a NodeGraph.
    /// Returns the cached topology (creates if not present).
    const NodeGraphTopology& analyze(const NodeGraph& nodeGraph);

    /// Get cached topology for a NodeGraph, or nullptr if not analyzed.
    const NodeGraphTopology* getTopology(const string& nodeGraphName) const;

    /// Clear the cache (mainly for testing)
    void clearCache();

  private:
    mutable std::mutex _cacheMutex;
    std::unordered_map<string, NodeGraphTopology> _cache;  ///< Keyed by NodeGraph name
};

MATERIALX_NAMESPACE_END

#endif
