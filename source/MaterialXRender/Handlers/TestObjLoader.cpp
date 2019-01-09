#include <MaterialXRender/Handlers/TestObjLoader.h>
#include <MaterialXCore/Util.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <numeric>
#include <iomanip>  
#include <limits>

namespace MaterialX
{ 

bool TestObjLoader::load(const std::string& fileName, MeshList& meshList)
{
    std::ifstream objfile;
    objfile.open(fileName);
    if (!objfile.is_open())
    {
        return nullptr;
    }

    MeshPtr mesh = Mesh::create(fileName);
    MeshStreamPtr positionStream = MeshStream::create(MeshStream::POSITION_ATTRIBUTE, 0);
    MeshFloatBuffer& positionData = positionStream->getData();
    mesh->addStream(positionStream);

    MeshStreamPtr normalStream = MeshStream::create(MeshStream::NORMAL_ATTRIBUTE, 0);
    MeshFloatBuffer& normalData = normalStream->getData();
    mesh->addStream(normalStream);

    MeshStreamPtr texCoordStream = MeshStream::create(MeshStream::TEXCOORD_ATTRIBUTE, 0);
    texCoordStream->setStride(2);
    MeshFloatBuffer& texCoordData = texCoordStream->getData();
    mesh->addStream(texCoordStream);

    MeshFloatBuffer pos;
    MeshFloatBuffer uv;
    MeshFloatBuffer norm;
    MeshIndexBuffer pidx;
    MeshIndexBuffer uvidx;
    MeshIndexBuffer nidx;

    Vector3 minPos = { MAX_FLOAT , MAX_FLOAT , MAX_FLOAT };
    Vector3 maxPos = { -MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT };

    // Enable debugging of read by dumping to disk what was read in.
    // Disabled by default
    std::ofstream dump;
    bool debugDump = false;
    if (debugDump)
    {
        dump.open("dump.obj");
    }

    float val1, val2, val3;
    unsigned int ipos[4], iuv[4], inorm[4];
    std::string line;
    size_t faceCount = 0;
    while (std::getline(objfile, line))
    {
        if (line.substr(0, 2) == "v ")
        {
            std::istringstream valstring(line.substr(2));
            valstring >> val1; valstring >> val2; valstring >> val3;

            if (debugDump)
            {
                dump << "v " << val1 << " " << val2 << " " << val3 << std::endl;
            }

            // Y-flip
            val2 = -val2;

            // Update bounds
            if (val1 < minPos[0]) minPos[0] = val1;
            if (val2 < minPos[1]) minPos[1] = val2;
            if (val3 < minPos[2]) minPos[2] = val3;
            if (val1 > maxPos[0]) maxPos[0] = val1;
            if (val2 > maxPos[1]) maxPos[1] = val2;
            if (val3 > maxPos[2]) maxPos[2] = val3;

            pos.push_back(val1); pos.push_back(val2); pos.push_back(val3);
        }
        else if (line.substr(0, 3) == "vt ")
        {
            std::istringstream valstring(line.substr(3));
            valstring >> val1; valstring >> val2;
            uv.push_back(val1); uv.push_back(val2);

            if (debugDump)
            {
                dump << "vt " << val1 << " " << val2 << std::endl;
            }
        }
        else if (line.substr(0, 3) == "vn ")
        {
            std::istringstream valstring(line.substr(3));
            valstring >> val1; valstring >> val2; valstring >> val3;
            norm.push_back(val1); norm.push_back(val2); norm.push_back(val3);

            if (debugDump)
            {
                dump << "vn " << val1 << " " << val2 << " " << val3 << std::endl;
            }
        }
        else if (line.substr(0, 2) == "f ")
        {
            // Extact out the component parts from face string
            //
            std::istringstream valstring(line.substr(2));
            std::string vertices[4];
            valstring >> vertices[0];
            valstring >> vertices[1];
            valstring >> vertices[2];
            valstring >> vertices[3];

            int vertexCount = 0;
            for (unsigned int i = 0; i < 4; i++)
            {
                if (vertices[i].size())
                {
                    std::vector<string> vertexParts = MaterialX::splitString(vertices[i], "/");
                    if (vertexParts.size() == 3)
                    {
                        ipos[i] = std::stoi(vertexParts[0]);
                        iuv[i] = std::stoi(vertexParts[1]);
                        inorm[i] = std::stoi(vertexParts[2]);
                        vertexCount++;
                    }
                }
            }

            if (vertexCount >= 3)
            {
                faceCount++;

                pidx.push_back(ipos[0] - 1);
                pidx.push_back(ipos[1] - 1);
                pidx.push_back(ipos[2] - 1);

                uvidx.push_back(iuv[0] - 1);
                uvidx.push_back(iuv[1] - 1);
                uvidx.push_back(iuv[2] - 1);

                nidx.push_back(inorm[0] - 1);
                nidx.push_back(inorm[1] - 1);
                nidx.push_back(inorm[2] - 1);

                if (debugDump)
                {
                    dump << "f "
                        << ipos[0] << "/" << iuv[0] << "/" << inorm[0] << " "
                        << ipos[1] << "/" << iuv[1] << "/" << inorm[1] << " "
                        << ipos[2] << "/" << iuv[2] << "/" << inorm[2] << std::endl;
                }

                if (vertexCount >= 4)
                {
                    faceCount++;

                    pidx.push_back(ipos[0] - 1);
                    pidx.push_back(ipos[2] - 1);
                    pidx.push_back(ipos[3] - 1);

                    uvidx.push_back(iuv[0] - 1);
                    uvidx.push_back(iuv[2] - 1);
                    uvidx.push_back(iuv[3] - 1);

                    nidx.push_back(inorm[0] - 1);
                    nidx.push_back(inorm[2] - 1);
                    nidx.push_back(inorm[3] - 1);

                    if (debugDump)
                    {
                        dump << "f "
                            << ipos[0] << "/" << iuv[0] << "/" << inorm[0] << " "
                            << ipos[2] << "/" << iuv[2] << "/" << inorm[2] << " "
                            << ipos[3] << "/" << iuv[3] << "/" << inorm[3] << std::endl;
                    }
                }
            }
        }
    }

    dump.close();
    objfile.close();

    // Set bounds
    mesh->setMinimumBounds(minPos);
    mesh->setMaximumBounds(maxPos);

    // Organize data to get triangles for positions 
    for (unsigned int i = 0; i < pidx.size(); i++)
    {
        unsigned int vertexIndex = 3 * pidx[i];
        positionData.push_back(pos[vertexIndex]);
        positionData.push_back(pos[vertexIndex + 1]);
        positionData.push_back(pos[vertexIndex + 2]);
    }

    // Organize data to get triangles for texture coordinates 
    for (unsigned int i = 0; i < uvidx.size(); i++)
    {
        unsigned int vertexIndex = 2 * uvidx[i];
        texCoordData.push_back(uv[vertexIndex]);
        texCoordData.push_back(uv[vertexIndex + 1]);
    }

    // Organize data to get triangles for normals 
    for (unsigned int i = 0; i < nidx.size(); i++)
    {
        unsigned int vertexIndex = 3 * nidx[i];
        normalData.push_back(norm[vertexIndex]);
        normalData.push_back(norm[vertexIndex + 1]);
        normalData.push_back(norm[vertexIndex + 2]);
    }

    // Set up flattened indexing
    MeshPartitionPtr partition = nullptr;
    if (faceCount && positionData.size() && pidx.size())
    {
        partition = MeshPartition::create();
        partition->setFaceCount(faceCount);
        MeshIndexBuffer& indexing = partition->getIndices();
        indexing.resize(pidx.size());
        std::iota(indexing.begin(), indexing.end(), 0);

        mesh->addPartition(partition);
    }

    // Add additional streams required for testing
    //
    MeshStreamPtr tangentStream = MeshStream::create(MeshStream::TANGENT_ATTRIBUTE, 0);
    partition->generateTangents(positionStream, texCoordStream, normalStream, tangentStream);
    mesh->addStream(tangentStream);

    MeshStreamPtr stream = MeshStream::create(MeshStream::BITANGENT_ATTRIBUTE, 0);
    MeshFloatBuffer& bitangents = stream->getData();
    bitangents.resize(positionData.size());
    std::fill(bitangents.begin(), bitangents.end(), 0.0f);
    mesh->addStream(stream);

    stream = MeshStream::create(MeshStream::COLOR_ATTRIBUTE, 0);
    MeshFloatBuffer& col = stream->getData();
    stream->setStride(4);
    col.resize(texCoordData.size() * 2);
    std::fill(col.begin(), col.end(), 1.0f);
    mesh->addStream(stream);

    // Add in new mesh
    meshList.push_back(mesh);
    return true;
}

}