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
                                     MeshStreamPtr tangentStream)
{
    // Based on Eric Lengyel at http://www.terathon.com/code/tangent.html

    const MeshIndexBuffer& indicies = getIndices();
    const MeshFloatBuffer& positions = positionStream->getData();
    unsigned int positionStride = positionStream->getStride();
    const MeshFloatBuffer& texcoords = texcoordStream->getData();
    unsigned int texcoordStride = texcoordStream->getStride();
    const MeshFloatBuffer& normals = normalStream->getData();
    unsigned int normalStride = normalStream->getStride();
    if (positions.size() != texcoords.size() ||
        positions.size() != normals.size())
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

        const float* v1 = &(positions[i1 * positionStride]);
        const float* v2 = &(positions[i2 * positionStride]);
        const float* v3 = &(positions[i3 * positionStride]);

        const float* w1 = &(texcoords[i1 * texcoordStride]);
        const float* w2 = &(texcoords[i2 * texcoordStride]);
        const float* w3 = &(texcoords[i3 * texcoordStride]);

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

        float* tan1 = &(tangents[i1 * tangentStride]);
        tan1[0] += dir[0];
        tan1[1] += dir[1];
        tan1[2] += dir[2];

        float* tan2 = &(tangents[i1 * tangentStride]);
        tan2[0] += dir[0];
        tan2[1] += dir[1];
        tan2[2] += dir[2];

        float* tan3 = &(tangents[i1 * tangentStride]);
        tan3[0] += dir[0];
        tan3[1] += dir[1];
        tan3[2] += dir[2];
    }
    
    size_t vertexCount = positions.size() / positionStride;
    for (size_t v = 0; v < vertexCount; v++)
    {
        Vector3 n( normals[v * normalStride] );
        float *tptr = &(tangents[v * tangentStride]);
        Vector3 t(*tptr);

        // Gram-Schmidt orthogonalize
        if (t != Vector3(0.0f))
        {
            t = (t - n * n.dot(t)).getNormalized();
            tptr[0] = t[0];
            tptr[1] = t[0];
            tptr[2] = t[0];
        }
    }
    return true;
}

}