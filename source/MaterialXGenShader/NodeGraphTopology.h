//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_NODEGRAPHTOPOLOGY_H
#define MATERIALX_NODEGRAPHTOPOLOGY_H

#include <MaterialXGenShader/Export.h>

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Node.h>

#include <memory>
#include <mutex>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

class NodeGraphTopology;

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

/// @class NodeGraphTopologyCache
/// Caches topology analyses per NodeGraph definition and creates permutations.
/// Thread-safe singleton; kept as an explicit object so its lifetime can be
/// controlled (e.g., moved to GenContext in the future).
class MX_GENSHADER_API NodeGraphTopologyCache
{
  public:
    NodeGraphTopologyCache() = default;
    ~NodeGraphTopologyCache();

    NodeGraphTopologyCache(const NodeGraphTopologyCache&) = delete;
    NodeGraphTopologyCache& operator=(const NodeGraphTopologyCache&) = delete;

    /// Get the global instance.
    static NodeGraphTopologyCache& instance();

    /// Analyze a NodeGraph's topology (cached) and create a permutation for a
    /// specific call-site node instance.
    /// @return The permutation, or nullptr if no optimization is possible.
    std::unique_ptr<NodeGraphPermutation> createPermutation(
        const NodeGraph& nodeGraph, const Node& node);

  private:
    const NodeGraphTopology& getTopology(const NodeGraph& nodeGraph);

    mutable std::mutex _cacheMutex;
    std::unordered_map<string, std::unique_ptr<NodeGraphTopology>> _cache;
};

MATERIALX_NAMESPACE_END

#endif
