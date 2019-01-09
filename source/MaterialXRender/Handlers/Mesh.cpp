#include <MaterialXRender/Handlers/Mesh.h>

namespace MaterialX
{
const std::string MeshStream::POSITION_ATTRIBUTE("i_position");
const std::string MeshStream::NORMAL_ATTRIBUTE("i_normal");
const std::string MeshStream::TEXCOORD_ATTRIBUTE("i_texcoord");
const std::string MeshStream::TANGENT_ATTRIBUTE("i_tangent");
const std::string MeshStream::BITANGENT_ATTRIBUTE("i_bitangent");
const std::string MeshStream::COLOR_ATTRIBUTE("i_color");

bool MeshPartition::generateTangents(MeshStreamPtr positionStream, MeshStreamPtr texcoordStream, MeshStreamPtr normalStream,
                                     MeshStreamPtr tangentStream, MeshStreamPtr bitangentStream)
{
    // Based on Eric Lengyel at http://www.terathon.com/code/tangent.html

    const MeshIndexBuffer& indicies = getIndices();
    MeshFloatBuffer& positions = positionStream->getData();
    unsigned int positionStride = positionStream->getStride();
    MeshFloatBuffer& texcoords = texcoordStream->getData();
    unsigned int texcoordStride = texcoordStream->getStride();
    MeshFloatBuffer& normals = normalStream->getData();
    unsigned int normalStride = normalStream->getStride();

    size_t vertexCount = positions.size() / positionStride;
    size_t uvCount = texcoords.size() / texcoordStride;
    size_t normalCount = normals.size() / normalStride;
    if (vertexCount != uvCount ||
        vertexCount != normalCount)
    {
        return false;
    }

    // Prepare tangent stream data
    MeshFloatBuffer& tangents = tangentStream->getData();
    tangents.resize(positions.size());
    std::fill(tangents.begin(), tangents.end(), 0.0f);
    const unsigned int tangentStride = 3;
    tangentStream->setStride(tangentStride);

    for (size_t faceIndex = 0; faceIndex < getFaceCount(); faceIndex++)
    {
        int i1 = indicies[faceIndex * 3 + 0];
        int i2 = indicies[faceIndex * 3 + 1];
        int i3 = indicies[faceIndex * 3 + 2];

        Vector3& v1 = *reinterpret_cast<Vector3*>(&(positions[i1 * positionStride]));
        Vector3& v2 = *reinterpret_cast<Vector3*>(&(positions[i2 * positionStride]));
        Vector3& v3 = *reinterpret_cast<Vector3*>(&(positions[i3 * positionStride]));

        Vector2& w1 = *reinterpret_cast<Vector2*>(&(texcoords[i1 * texcoordStride]));
        Vector2& w2 = *reinterpret_cast<Vector2*>(&(texcoords[i2 * texcoordStride]));
        Vector2& w3 = *reinterpret_cast<Vector2*>(&(texcoords[i3 * texcoordStride]));

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
        Vector3 dir((t2 * x1 - t1 * x2) * r,
                    (t2 * y1 - t1 * y2) * r,
                    (t2 * z1 - t1 * z2) * r);

        Vector3& tan1 = *reinterpret_cast<Vector3*>(&(tangents[i1 * tangentStride]));
        Vector3& tan2 = *reinterpret_cast<Vector3*>(&(tangents[i2 * tangentStride]));
        Vector3& tan3 = *reinterpret_cast<Vector3*>(&(tangents[i3 * tangentStride]));
        tan1 += dir;
        tan2 += dir;
        tan3 += dir;
    }
    
    MeshFloatBuffer* bitangents = nullptr;
    unsigned int bitangentStride = 0;
    if (bitangentStream)
    {
        bitangents = &(bitangentStream->getData());
        bitangents->resize(positions.size());
        std::fill(tangents.begin(), tangents.end(), 0.0f);
    }

    for (size_t v = 0; v < vertexCount; v++)
    {
        Vector3& n = *reinterpret_cast<Vector3*>(&(normals[v * normalStride]));
        Vector3& t = *reinterpret_cast<Vector3*>(&(tangents[v * tangentStride]));
        Vector3* b = bitangents ? reinterpret_cast<Vector3*>(&(bitangents[v * bitangentStride])) : nullptr;

        // Gram-Schmidt orthogonalize
        if (t != Vector3(0.0f))
        {
            t = (t - n * n.dot(t)).getNormalized();
            if (b)
            {
                *b = n.cross(t);
            }
        }
    }
    return true;
}

}