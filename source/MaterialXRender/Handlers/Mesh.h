#ifndef MATERIALX_MESH_H
#define MATERIALX_MESH_H

#include <MaterialXCore/Types.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace MaterialX
{
const float MAX_FLOAT = std::numeric_limits<float>::max();

/// Geometry index buffer
using MeshIndexBuffer = std::vector<unsigned int>;
/// Float geometry buffer
using MeshFloatBuffer = std::vector<float>;

/// Shader pointer to a mesh stream
using MeshStreamPtr = std::shared_ptr<class MeshStream>;

/// List of mesh streams 
using MeshStreamList = std::vector<MeshStreamPtr>;

/// @class MeshStream
/// Class that contains the contents for a stream
class MeshStream
{
  public:
    /// Preset geometric attribute types
    static const std::string POSITION_ATTRIBUTE;
    static const std::string NORMAL_ATTRIBUTE;
    static const std::string TEXCOORD_ATTRIBUTE;
    static const std::string TANGENT_ATTRIBUTE;
    static const std::string BITANGENT_ATTRIBUTE;
    static const std::string COLOR_ATTRIBUTE;

    static MeshStreamPtr create(const std::string type, unsigned int index=0)
    {
        return std::make_shared<MeshStream>(type, index);
    }

    // Constructor. Default index is 0 and default stride is 3
    MeshStream(const std::string type, unsigned int index) :
        _type(type),        
        _index(index),
        _stride(3) {}
    
    ~MeshStream() {}

    const std::string& getType() const
    {
        return _type;
    }

    unsigned int getIndex() const
    {
        return _index;
    }

    MeshFloatBuffer& getData()
    {
        return _data;
    }

    unsigned int getStride() const
    {
        return _stride;
    }

    void setStride(unsigned int stride)
    {
        _stride = stride;
    }

  protected:
    /// Attribute type
    std::string _type;
    /// Attribute Index
    unsigned int _index;
    /// Data buffer
    MeshFloatBuffer _data;
    /// Stride between elements in buffer
    unsigned int _stride;
};

/// Shader pointer to a mesh stream
using MeshPartitionPtr = std::shared_ptr<class MeshPartition>;

/// @class @MeshPartition
/// Class that describes a sub-region of a mesh using vertex indexing
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

    Mesh(const std::string& identifier) :
        _identifier(identifier),
        _minimumBounds(MAX_FLOAT, MAX_FLOAT, MAX_FLOAT),
        _maximumBounds(-MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT),
        _sphereCenter(0.0f, 0.0f, 0.0f),
        _sphereRadius(0.0f),
        _vertCount(0)
    {
    }
    ~Mesh() { }
    
    /// Get a mesh stream
    /// @param name Name of stream
    MeshStreamPtr getStream(const std::string& type, unsigned int index=0) const
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

    /// Add a mesh stream. 
    void addStream(MeshStreamPtr stream)
    {
        _streams.push_back(stream);
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
    size_t _vertCount; 
    std::vector<MeshPartitionPtr> _partitions;
};

}
#endif // MATERIALX_MESH_H
