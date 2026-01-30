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
/// Analyzes a NodeGraph to identify "topological" inputs - inputs that when
/// constant (0 or 1) can eliminate entire branches of the graph.
///
/// This analysis is cached per NodeGraph definition and used to:
/// 1. Compute permutation keys for CompoundNode caching
/// 2. Identify which nodes can be skipped during ShaderGraph construction
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

    /// Analysis result for a NodeGraph
    struct Analysis
    {
        string nodeGraphName;
        std::map<string, TopologicalInput> topologicalInputs;  ///< Keyed by input name
        
        bool empty() const { return topologicalInputs.empty(); }
    };

    NodeGraphTopology() = default;
    ~NodeGraphTopology() = default;

    // Non-copyable, non-movable (singleton-like usage)
    NodeGraphTopology(const NodeGraphTopology&) = delete;
    NodeGraphTopology& operator=(const NodeGraphTopology&) = delete;

    /// Get the global instance
    static NodeGraphTopology& instance();

    /// Analyze a NodeGraph and cache the result.
    /// Returns the cached analysis (creates if not present).
    const Analysis& analyze(const NodeGraph& nodeGraph);

    /// Get cached analysis for a NodeGraph, or nullptr if not analyzed.
    const Analysis* getAnalysis(const string& nodeGraphName) const;

    /// Compute a permutation key based on constant input values.
    /// @param analysis The topology analysis for the NodeGraph
    /// @param node The node instance (to read constant values from)
    /// @return A string key like "coat=0,sheen=x" or empty if no optimization possible
    string computePermutationKey(const Analysis& analysis, ConstNodePtr node) const;

    /// Get the set of nodes to skip for a given permutation key.
    /// @param analysis The topology analysis
    /// @param permutationKey The key from computePermutationKey()
    /// @return Set of node names that can be skipped
    StringSet getNodesToSkip(const Analysis& analysis, const string& permutationKey) const;

    /// Clear the cache (mainly for testing)
    void clearCache();

  private:
    /// Check if an input qualifies as "topological" (can gate branches)
    bool isTopologicalInput(const InputPtr& input, const NodeDefPtr& nodeDef) const;

    /// Analyze which nodes are affected when a mix/multiply/bsdf input is 0 or 1
    void analyzeAffectedNodes(
        const NodePtr& node,
        const InputPtr& input,
        TopologicalInput& topoInput,
        const NodeGraph& nodeGraph) const;

    /// Build reverse connection map for a NodeGraph
    std::map<string, StringSet> buildReverseConnectionMap(const NodeGraph& nodeGraph) const;

    /// Recursively find all upstream nodes from a given node
    void collectUpstreamNodes(
        const string& nodeName,
        const NodeGraph& nodeGraph,
        StringSet& collected) const;

    mutable std::mutex _cacheMutex;
    std::unordered_map<string, Analysis> _cache;  ///< Keyed by NodeGraph name
};

MATERIALX_NAMESPACE_END

#endif
