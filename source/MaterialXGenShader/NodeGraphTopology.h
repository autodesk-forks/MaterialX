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

/// @class NodeGraphPermutation
/// Represents a specific permutation of a NodeGraph based on call-site input values.
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
    NodeGraphPermutation(string key, StringSet skipNodes)
        : _key(std::move(key)), _skipNodes(std::move(skipNodes)) {}

    string _key;
    StringSet _skipNodes;
};

/// Describes a single topological input and the nodes it can eliminate.
struct MX_GENSHADER_API TopologicalInput
{
    string name;
    StringSet nodesToSkipAt0;
    StringSet nodesToSkipAt1;
    StringSet maybeDeadAt0;
    StringSet maybeDeadAt1;
};

/// @class NodeGraphTopology
/// Analyzes a NodeGraph to identify "topological" inputs that, when constant
/// (0 or 1), can eliminate entire branches of the graph.
class MX_GENSHADER_API NodeGraphTopology
{
  public:
    explicit NodeGraphTopology(const NodeGraph& nodeGraph);
    std::unique_ptr<NodeGraphPermutation> createPermutation(const Node& node) const;

  private:
    static bool isTopologicalInput(const InputPtr& input, const NodeDefPtr& nodeDef);
    void analyzeAffectedNodes(const NodePtr& node, const InputPtr& input, TopologicalInput& topoInput);
    void buildNodeInfos(const NodeGraph& nodeGraph);

    struct NodeInfo
    {
        size_t downstreamRefCount = 0;
        StringSet upstreams;
    };

    std::map<string, TopologicalInput> _topologicalInputs;
    std::unordered_map<string, NodeInfo> _nodeInfos;
};

/// @class NodeGraphTopologyCache
/// Caches topology analyses per NodeGraph definition and creates permutations.
/// Thread-safe; lives on GenContext so clients control its lifetime and
/// invalidation.
class MX_GENSHADER_API NodeGraphTopologyCache
{
  public:
    NodeGraphTopologyCache() = default;
    ~NodeGraphTopologyCache();

    /// Copy creates a fresh empty cache (it's a cache — rebuilds on demand).
    NodeGraphTopologyCache(const NodeGraphTopologyCache&);
    NodeGraphTopologyCache& operator=(const NodeGraphTopologyCache&);

    /// Analyze a NodeGraph's topology (cached) and create a permutation for a
    /// specific call-site node instance.
    /// @return The permutation, or nullptr if no optimization is possible.
    std::unique_ptr<NodeGraphPermutation> createPermutation(
        const NodeGraph& nodeGraph, const Node& node);

    /// Discard all cached topology analyses.
    void clear();

  private:
    const NodeGraphTopology& getTopology(const NodeGraph& nodeGraph);

    mutable std::mutex _cacheMutex;
    std::unordered_map<string, std::shared_ptr<NodeGraphTopology>> _cache;
};

MATERIALX_NAMESPACE_END

#endif
