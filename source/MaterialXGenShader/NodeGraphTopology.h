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

    string nodeGraphName;
    std::map<string, TopologicalInput> topologicalInputs;  ///< Keyed by input name
    
    bool empty() const { return topologicalInputs.empty(); }

    /// Compute a permutation key based on constant input values.
    /// @param node The node instance (to read constant values from)
    /// @return A string key like "coat=0,sheen=x" or empty if no optimization possible
    string computePermutationKey(ConstNodePtr node) const;

    /// Get the set of nodes to skip for a given permutation key.
    /// @param permutationKey The key from computePermutationKey()
    /// @return Set of node names that can be skipped
    StringSet getNodesToSkip(const string& permutationKey) const;
};

/// @class NodeGraphTopologyCache
/// Caches NodeGraphTopology analyses per NodeGraph definition.
/// Thread-safe singleton for use during shader generation.
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

    /// Analyze a NodeGraph and cache the result.
    /// Returns the cached topology (creates if not present).
    const NodeGraphTopology& analyze(const NodeGraph& nodeGraph);

    /// Get cached topology for a NodeGraph, or nullptr if not analyzed.
    const NodeGraphTopology* getTopology(const string& nodeGraphName) const;

    /// Clear the cache (mainly for testing)
    void clearCache();

  private:
    /// Check if an input qualifies as "topological" (can gate branches)
    bool isTopologicalInput(const InputPtr& input, const NodeDefPtr& nodeDef) const;

    /// Analyze which nodes are affected when a mix/multiply/bsdf input is 0 or 1
    void analyzeAffectedNodes(
        const NodePtr& node,
        const InputPtr& input,
        NodeGraphTopology::TopologicalInput& topoInput,
        const NodeGraph& nodeGraph) const;

    /// Build reverse connection map for a NodeGraph
    std::map<string, StringSet> buildReverseConnectionMap(const NodeGraph& nodeGraph) const;

    /// Recursively find all upstream nodes from a given node
    void collectUpstreamNodes(
        const string& nodeName,
        const NodeGraph& nodeGraph,
        StringSet& collected) const;

    mutable std::mutex _cacheMutex;
    std::unordered_map<string, NodeGraphTopology> _cache;  ///< Keyed by NodeGraph name
};

MATERIALX_NAMESPACE_END

#endif
