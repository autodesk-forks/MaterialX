#ifndef MATERIALX_MESH_H
#define MATERIALX_MESH_H

#include <MaterialXCore/Types.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace MaterialX
{
/// Geometry index buffer
using MeshIndexBuffer = std::vector<unsigned int>;
/// Float geometry buffer
using MeshFloatBuffer = std::vector<float>;

/// Shader pointer to a mesh stream
using MeshStreamPtr = std::shared_ptr<class MeshStream>;

/// List of mesh streams 
using MeshStreamList = std::vector<MeshStreamPtr>;

/// @class MeshStream
/// Class to represent a mesh data stream
class MeshStream
{
  public:
    /// Position attribute 
    static const std::string POSITION_ATTRIBUTE;
    /// Normal attribute 
    static const std::string NORMAL_ATTRIBUTE;
    /// Texture coordinate attribute 
    static const std::string TEXCOORD_ATTRIBUTE;
    /// Tangent attribute 
    static const std::string TANGENT_ATTRIBUTE;
    /// Bitangent attribute 
    static const std::string BITANGENT_ATTRIBUTE;
    /// Color attribute 
    static const std::string COLOR_ATTRIBUTE;

    /// Create instance of a mesh stream
    static MeshStreamPtr create(const std::string& name, const std::string& type, unsigned int index=0)
    {
        return std::make_shared<MeshStream>(name, type, index);
    }

    // Constructor.
    MeshStream(const std::string& name, const std::string& type, unsigned int index) :
        _name(name),
        _type(type),
        _index(index),
        _stride(3) {}
    
    ~MeshStream() {}

    /// Resize data to an given number of elements
    void resize(unsigned int elementCount)
    {
        _data.resize(elementCount * _stride);
    }

    /// Get stream name
    const std::string& getName() const
    {
        return _name;
    }

    /// Get stream attribute name
    const std::string& getType() const
    {
        return _type;
    }

    /// Get stream index
    unsigned int getIndex() const
    {
        return _index;
    }

    /// Get stream data
    MeshFloatBuffer& getData()
    {
        return _data;
    }

    /// Get stream data
    const MeshFloatBuffer& getData() const
    {
        return _data;
    }

    /// Get stride between elements
    unsigned int getStride() const
    {
        return _stride;
    }

    /// Set stride between elements
    void setStride(unsigned int stride)
    {
        _stride = stride;
    }

  protected:
    std::string _name;
    std::string _type;
    unsigned int _index;
    MeshFloatBuffer _data;
    unsigned int _stride;
};

/// Shader pointer to a mesh stream
using MeshPartitionPtr = std::shared_ptr<class MeshPartition>;

/// @class @MeshPartition
/// Class that describes a sub-region of a mesh using vertex indexing.
/// Note that a face is considered to be a triangle.
class MeshPartition
{
  public:
    static MeshPartitionPtr create()
    {
        return std::make_shared<MeshPartition>();
    }

    /// Default constructor
    MeshPartition() :
        _faceCount(0)
    {
    }

    /// Default destructor
    ~MeshPartition() { }

    /// Resize data to an given number of indices
    void resize(unsigned int elementCount)
    {
        _indices.resize(elementCount);
    }

    /// Get geometry identifier
    const std::string& getIdentifier() const
    {
        return _identifier;
    }

    /// Set geometry identifier
    void setIdentifier(const std::string& val)
    {
        _identifier = val;
    }

    /// Return indexing
    MeshIndexBuffer& getIndices()
    {
        return _indices;
    }

    /// Return indexing
    const MeshIndexBuffer& getIndices() const
    {
        return _indices;
    }

    /// Return number of faces
    size_t getFaceCount() const
    {
        return _faceCount;
    }

    /// Set face count
    void setFaceCount(size_t val)
    {
        _faceCount = val;
    }

    /// Generate tangents and optionally bitangents for a given 
    /// a set of postions, texture coordinates and normals.
    /// @param positions Positions to use
    /// @param texcoords Texture coordinates to use
    /// @param normals Normals to use
    /// @param tangents Tangents to produce
    /// @param bitangents Bitangents to produce.
    /// Returns true if successful.
    bool generateTangents(MeshStreamPtr positions, MeshStreamPtr texcoords, MeshStreamPtr normals,
                          MeshStreamPtr tangents, MeshStreamPtr bitangents);

  private:
    std::string _identifier;
    MeshIndexBuffer _indices;
    size_t _faceCount;
};


/// Shader pointer to a GeometryMesh
using MeshPtr = std::shared_ptr<class Mesh>;

/// List of meshes
using MeshList = std::vector<MeshPtr>;

/// Map of names to mesh
using MeshMap = std::unordered_map<std::string, MeshPtr>;

/// @class @Mesh
/// Container for mesh data
///
class Mesh
{
  public:
    static MeshPtr create(const std::string& identifier)
    {
        return std::make_shared<Mesh>(identifier);
    }

    Mesh(const std::string& identifier);
    ~Mesh() { }

    /// Get a mesh stream by name
    /// @param name Name of stream
    /// @return Reference to a mesh stream if found
    MeshStreamPtr getStream(const std::string& name) const
    {
        for (auto stream : _streams)
        {
            if (stream->getName() == name)
            {
                return stream;
            }
        }
        return MeshStreamPtr();
    }

    /// Get a mesh stream by type and index
    /// @param type Type of stream
    /// @param index Index of stream
    /// @return Reference to a mesh stream if found
    MeshStreamPtr getStream(const std::string& type, unsigned int index) const
    {
        for (auto stream : _streams)
        {
            if (stream->getType() == type &&
                stream->getIndex() == index)
            {
                return stream;
            }
        }
        return MeshStreamPtr();
    }

    /// Add a mesh stream 
    void addStream(MeshStreamPtr stream)
    {
        _streams.push_back(stream);
    }

    /// Set vertex count
    void setVertexCount(size_t val)
    {
        _vertexCount = val;
    }

    /// Get vertex count
    size_t getVertexCount() const
    {
        return _vertexCount;
    }

    /// Set the minimum bounds for the geometry
    void setMinimumBounds(const Vector3& val)
    {
        _minimumBounds = val;
    }

    /// Return the minimum bounds for the geometry
    const Vector3& getMinimumBounds() const
    {
        return _minimumBounds;
    }

    /// Set the minimum bounds for the geometry
    void setMaximumBounds(const Vector3& v)
    {
        _maximumBounds = v;
    }

    /// Return the minimum bounds for the geometry
    const Vector3& getMaximumBounds() const
    {
        return _maximumBounds;
    }

    /// Set center of the bounding sphere
    void setSphereCenter(const Vector3& val)
    {
        _sphereCenter = val;
    }

    /// Return center of the bounding sphere
    const Vector3& getSphereCenter() const
    {
        return _sphereCenter;
    }

    /// Set radius of the bounding sphere
    void setSphereRadius(float val)
    {
        _sphereRadius = val;
    }

    /// Return radius of the bounding sphere
    float getSphereRadius() const
    {
        return _sphereRadius;
    }

    /// Return the number of mesh partitions
    size_t getPartitionCount() const
    {
        return _partitions.size();
    }

    /// Add a partition
    void addPartition(MeshPartitionPtr partition)
    {
        _partitions.push_back(partition);
    }

    /// Return a reference to a mesh partition
    MeshPartitionPtr getPartition(size_t partIndex) const
    {
        return _partitions[partIndex];
    }

  private:
    std::string _identifier;

    Vector3 _minimumBounds;
    Vector3 _maximumBounds;

    Vector3 _sphereCenter;
    float _sphereRadius;

    MeshStreamList _streams;
    size_t _vertexCount;
    std::vector<MeshPartitionPtr> _partitions;
};

}
#endif // MATERIALX_MESH_H
