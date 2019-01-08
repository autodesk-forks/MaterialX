#include <MaterialXRender/Handlers/Mesh.h>

namespace MaterialX
{
const std::string MeshStream::POSITION_ATTRIBUTE("i_position");
const std::string MeshStream::NORMAL_ATTRIBUTE("i_normal");
const std::string MeshStream::TEXCOORD_ATTRIBUTE("i_texcoord");
const std::string MeshStream::TANGENT_ATTRIBUTE("i_tangent");
const std::string MeshStream::BITANGENT_ATTRIBUTE("i_bitangent");
const std::string MeshStream::COLOR_ATTRIBUTE("i_color");

#if 0
void Mesh::generateTangents()
{
    // Based on Eric Lengyel at http://www.terathon.com/code/tangent.html

    for (size_t partIndex = 0; partIndex < getPartitionCount(); partIndex++)
    {
        const Partition& part = getPartition(partIndex);
        for (size_t faceIndex = 0; faceIndex < part.getFaceCount(); faceIndex++)
        {
            int i1 = part.getIndices()[faceIndex * 3 + 0];
            int i2 = part.getIndices()[faceIndex * 3 + 1];
            int i3 = part.getIndices()[faceIndex * 3 + 2];

            const mx::Vector3& v1 = _positions[i1];
            const mx::Vector3& v2 = _positions[i2];
            const mx::Vector3& v3 = _positions[i3];

            const mx::Vector2& w1 = _texcoords[i1];
            const mx::Vector2& w2 = _texcoords[i2];
            const mx::Vector2& w3 = _texcoords[i3];

            float x1 = v2[0] - v1[0];
            float x2 = v3[0] - v1[0];
            float y1 = v2[1] - v1[1];
            float y2 = v3[1] - v1[1];
            float z1 = v2[2] - v1[2];
            float z2 = v3[2] - v1[2];
        
            float s1 = w2[0] - w1[0];
            float s2 = w3[0] - w1[0];
            float t1 = w2[1] - w1[1];
            float t2 = w3[1] - w1[1];
        
            float denom = s1 * t2 - s2 * t1;
            float r = denom ? 1.0f / denom : 0.0f;
            mx::Vector3 dir((t2 * x1 - t1 * x2) * r,
                            (t2 * y1 - t1 * y2) * r,
                            (t2 * z1 - t1 * z2) * r);
        
            _tangents[i1] += dir;
            _tangents[i2] += dir;
            _tangents[i3] += dir;
        }
    }
    
    for (size_t v = 0; v < _vertCount; v++)
    {
        const mx::Vector3& n = _normals[v];
        mx::Vector3& t = _tangents[v];

        // Gram-Schmidt orthogonalize
        if (t != mx::Vector3(0.0f))
        {
            t = (t - n * n.dot(t)).getNormalized();
        }
    }
}

#endif
}