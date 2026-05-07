//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/ShaderGraphHash.h>

#include <functional>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Combines a new hash value into an existing seed using a variation of
// the boost::hash_combine algorithm. 
void hashCombine(size_t& seed, size_t value)
{
    // The magic number 0x9e3779b9 is the fractional part of the golden ratio, used here
    // to achieve a well-distributed mix of the seed and value when combining.
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

void hashString(size_t& seed, const string& str)
{
    hashCombine(seed, std::hash<string>()(str));
}

void hashBool(size_t& seed, bool value)
{
    hashCombine(seed, std::hash<bool>()(value));
}

void hashUint32(size_t& seed, uint32_t value)
{
    hashCombine(seed, std::hash<uint32_t>()(value));
}

void hashSize(size_t& seed, size_t value)
{
    hashCombine(seed, std::hash<size_t>()(value));
}

// Hash the GenOptions settings to incorporate them into the hash seed
void hashGenOptions(size_t& seed, const GenOptions& options)
{
    hashUint32(seed, static_cast<uint32_t>(options.shaderInterfaceType));
    hashBool(seed, options.emitColorTransforms);
    hashBool(seed, options.elideConstantNodes);
    hashBool(seed, options.addUpstreamDependencies);
    hashString(seed, options.targetColorSpaceOverride);
    hashString(seed, options.targetDistanceUnit);
}

// This function adds the key structural properties of a shader port to the hash seed.
// It does this by hashing important attributes (type, semantic, color space, unit, geom property)
// into the seed. It also hashes only the "structural" flags (UNIFORM and BIND_INPUT), 
// so the hash only depends on properties that affect the structural identity of the port.
void hashPortStructure(size_t& seed, const ShaderPort* port)
{
    hashString(seed, port->getType().getName());
    hashString(seed, port->getSemantic());
    hashString(seed, port->getColorSpace());
    hashString(seed, port->getUnit());
    hashString(seed, port->getGeomProp());

    // Only hash the relevant structural flags (UNIFORM and BIND_INPUT)
    uint32_t structuralFlags = port->getFlags() &
        (ShaderPortFlag::UNIFORM | ShaderPortFlag::BIND_INPUT);
    hashUint32(seed, structuralFlags);
}

size_t findOutputIndex(const ShaderOutput* output)
{
    const ShaderNode* node = output->getNode();
    for (size_t i = 0; i < node->numOutputs(); ++i)
    {
        if (node->getOutput(i) == output)
            return i;
    }
    return SIZE_MAX;
}

} // anonymous namespace

size_t computeStructuralHash(const ShaderGraph& graph)
{
    size_t seed = 0;

    // 1. Hash graph-level input sockets (count + structural type info, no names or values)
    hashSize(seed, graph.numInputSockets());
    for (const ShaderGraphInputSocket* socket : graph.getInputSockets())
    {
        hashPortStructure(seed, socket);
    }

    // 2. Hash graph-level output sockets (count + structural type info)
    hashSize(seed, graph.numOutputSockets());
    for (const ShaderGraphOutputSocket* socket : graph.getOutputSockets())
    {
        hashPortStructure(seed, socket);
    }

    // 3. Build a stable index for each node using topological order
    const auto& nodes = graph.getNodes();
    std::unordered_map<const ShaderNode*, size_t> nodeIndex;
    nodeIndex.reserve(nodes.size() + 1);
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        nodeIndex[nodes[i]] = i;
    }
    // Special connection-source indices that won't collide with valid
    // topological node indices (which start at 0 and count up).
    enum ConnectionSourceIndex : size_t
    {
        GraphSelf  = SIZE_MAX,       // The graph itself is the source (graph input sockets)
        Unconnected = SIZE_MAX - 1,  // The input has no upstream connection
        Unresolved = SIZE_MAX - 2    // The source node was not found in the graph
    };
    nodeIndex[&graph] = GraphSelf;

    // 4. Hash each node in topological order
    hashSize(seed, nodes.size());
    for (const ShaderNode* node : nodes)
    {
        hashSize(seed, node->getImplementation().getHash());
        hashUint32(seed, node->getClassification());

        // Node inputs
        hashSize(seed, node->numInputs());
        for (const ShaderInput* input : node->getInputs())
        {
            hashPortStructure(seed, input);

            // Hash the source node's topological index and output port name to
            // ensure consistent results across graph instances.
            const ShaderOutput* conn = input->getConnection();
            if (conn)
            {
                auto it = nodeIndex.find(conn->getNode());
                size_t srcIdx = (it != nodeIndex.end()) ? it->second : Unresolved;
                hashSize(seed, srcIdx);
                hashSize(seed, findOutputIndex(conn));
            }
            else
            {
                hashSize(seed, Unconnected);
            }
        }

        // Node outputs
        hashSize(seed, node->numOutputs());
        for (const ShaderOutput* output : node->getOutputs())
        {
            hashPortStructure(seed, output);
        }
    }

    // 5. Hash graph output socket connections
    for (const ShaderGraphOutputSocket* socket : graph.getOutputSockets())
    {
        const ShaderOutput* conn = socket->getConnection();
        if (conn)
        {
            auto it = nodeIndex.find(conn->getNode());
            size_t srcIdx = (it != nodeIndex.end()) ? it->second : Unresolved;
            hashSize(seed, srcIdx);
            hashSize(seed, findOutputIndex(conn));
        }
        else
        {
            hashSize(seed, Unconnected);
        }
    }

    return seed;
}

size_t computeStructuralHash(const ShaderGraph& graph, const GenOptions& options)
{
    size_t seed = 0;
    hashGenOptions(seed, options);
    hashCombine(seed, computeStructuralHash(graph));
    return seed;
}

MATERIALX_NAMESPACE_END
