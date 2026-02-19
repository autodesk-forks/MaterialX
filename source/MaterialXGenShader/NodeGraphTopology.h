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
#include <memory>
#include <mutex>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

// Forward declaration
class NodeGraphPermutation;

/// @class NodeGraphTopology
/// Topology analysis result for a single NodeGraph.
/// Identifies "topological" inputs - inputs that when constant (0 or 1)
/// can eliminate entire branches of the graph.
/// Uses reference counting for correct dead code elimination with shared nodes.
/// The analysis is performed in the constructor for better encapsulation.
class MX_GENSHADER_API NodeGraphTopology
{
  public:
    /// Information about a single topological input
    struct TopologicalInput
    {
        string name;                   ///< Input name on the NodeGraph interface
        StringSet nodesToSkipAt0;      ///< Nodes to unconditionally skip when value = 0
        StringSet nodesToSkipAt1;      ///< Nodes to unconditionally skip when value = 1
        StringSet maybeDeadAt0;        ///< Nodes that lose a consumer when value = 0
        StringSet maybeDeadAt1;        ///< Nodes that lose a consumer when value = 1
    };

    /// Construct and analyze a NodeGraph's topology.
    /// @param nodeGraph The NodeGraph to analyze
    explicit NodeGraphTopology(const NodeGraph& nodeGraph);
    
    /// Create a permutation for a specific node instance.
    /// Uses reference counting to correctly handle shared dependencies.
    /// @param node The node instance (call site) to read input values from
    /// @return The permutation, or nullptr if no optimization is possible
    std::unique_ptr<NodeGraphPermutation> createPermutation(const Node& node) const;

  private:
    /// Check if an input qualifies as "topological" (can gate branches)
    static bool isTopologicalInput(const InputPtr& input, const NodeDefPtr& nodeDef);

    /// Analyze which nodes are affected when a mix/multiply/bsdf input is 0 or 1
    void analyzeAffectedNodes(
        const NodePtr& node,
        const InputPtr& input,
        TopologicalInput& topoInput);

    /// Build reference counts and upstream dependency map for the graph
    void buildRefCounts(const NodeGraph& nodeGraph);

    std::map<string, TopologicalInput> _topologicalInputs;  ///< Keyed by input name
    std::unordered_map<string, size_t> _refCounts;          ///< Reference count for each node
    std::unordered_map<string, StringSet> _nodeUpstreams;   ///< Upstream dependencies for each node
};

/// @class NodeGraphPermutation
/// Represents a specific permutation of a NodeGraph based on call-site input values.
/// Created via NodeGraphTopology::createPermutation().
/// Lightweight object used for cache key computation before ShaderNodeImpl creation.
class MX_GENSHADER_API NodeGraphPermutation
{
    friend class NodeGraphTopology;

  public:
    /// Return the permutation key (e.g., "coat=0,sheen=x").
    const string& getKey() const { return _key; }

    /// Check whether a node should be skipped (pruned) for this permutation.
    bool shouldSkip(const string& nodeName) const { return _skipNodes.count(nodeName) != 0; }

  private:
    /// Private constructor - use NodeGraphTopology::createPermutation() instead.
    NodeGraphPermutation(string key, StringSet skipNodes)
        : _key(std::move(key)), _skipNodes(std::move(skipNodes)) {}

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
    const NodeGraphTopology& get(const NodeGraph& nodeGraph);

  private:
    mutable std::mutex _cacheMutex;
    std::unordered_map<string, NodeGraphTopology> _cache;  ///< Keyed by NodeGraph name
};

MATERIALX_NAMESPACE_END

#endif
