#include <MaterialXView/Handlers/DefaultGeometryHandler.h>
#include <MaterialXCore/Util.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <numeric>
#include <iomanip>  
#include <limits>

namespace MaterialX
{ 
DefaultGeometryHandler::DefaultGeometryHandler() :
    GeometryHandler()
{
}

DefaultGeometryHandler::~DefaultGeometryHandler()
{
}

void DefaultGeometryHandler::clearData()
{
    _indexing.clear();
    _positionData.clear();
    _normalData.clear();
    _texcoordData[0].clear();
    _tangentData[0].clear();
    _bitangentData[0].clear();
    _colorData[0].clear();
    _texcoordData[1].clear();
    _tangentData[1].clear();
    _bitangentData[1].clear();
    _colorData[1].clear();
}

void DefaultGeometryHandler::setIdentifier(const std::string identifier)
{
    if (identifier != _identifier)
    {
        GeometryHandler::setIdentifier(identifier);
        clearData();
    }

    // Read in an obj file
    std::string extension = (_identifier.substr(_identifier.find_last_of(".") + 1));
    if (extension == "obj")
    {
        FloatBuffer pos;
        FloatBuffer uv;
        FloatBuffer norm;
        IndexBuffer pidx;
        IndexBuffer uvidx;
        IndexBuffer nidx;

        const float MAX_FLOAT = std::numeric_limits<float>::max();
        const float MIN_FLOAT = std::numeric_limits<float>::min();
        float minPos[3] = { MAX_FLOAT , MAX_FLOAT , MAX_FLOAT };
        float maxPos[3] = { MIN_FLOAT, MIN_FLOAT, MIN_FLOAT };

        std::ifstream objfile;
        objfile.open(_identifier);
        if (!objfile.is_open())
        {
            // Set to default if can't read file
            GeometryHandler::setIdentifier(SCREEN_ALIGNED_QUAD);
            return;
        }

        std::string line;

        std::ofstream dump;
        dump.open("dump.obj");

        float val1, val2, val3;
        unsigned int ipos[4], iuv[4], inorm[4];
        while (std::getline(objfile, line))
        {
            if (line.substr(0, 2) == "v ")
            {
                std::istringstream valstring(line.substr(2));
                valstring >> val1; valstring >> val2; valstring >> val3;
                
                // Update bounds
                if (val1 < minPos[0]) minPos[0] = val1;
                if (val2 < minPos[1]) minPos[1] = val2;
                if (val3 < minPos[2]) minPos[2] = val3;
                if (val1 > maxPos[0]) maxPos[0] = val1;
                if (val2 > maxPos[1]) maxPos[1] = val2;
                if (val3 > maxPos[2]) maxPos[2] = val3;

                pos.push_back(val1); pos.push_back(-val2); pos.push_back(val3);

                dump << "v " << val1 << " " << val2 << " " << val3 << std::endl;
            }
            else if (line.substr(0, 3) == "vt ")
            {
                std::istringstream valstring(line.substr(3));
                valstring >> val1; valstring >> val2;
                uv.push_back(val1); uv.push_back(val2);

                dump << "vt " << val1 << " " << val2 << std::endl;
            }
            else if (line.substr(0, 3) == "vn ")
            {
                std::istringstream valstring(line.substr(3));
                valstring >> val1; valstring >> val2; valstring >> val3;
                norm.push_back(val1); norm.push_back(val2); norm.push_back(val3);

                dump << "vn " << val1 << " " << val2 << " " << val3 << std::endl;
            }
            else if (line.substr(0, 2) == "f ")
            {
                // Extact out the compont parts from face string
                //
                std::istringstream valstring(line.substr(2));
                //std::istringstream valstring2(line.substr(2));
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
                    pidx.push_back(ipos[0] - 1);
                    pidx.push_back(ipos[1] - 1);
                    pidx.push_back(ipos[2] - 1);

                    uvidx.push_back(iuv[0] - 1);
                    uvidx.push_back(iuv[1] - 1);
                    uvidx.push_back(iuv[2] - 1);

                    nidx.push_back(inorm[0] - 1);
                    nidx.push_back(inorm[1] - 1);
                    nidx.push_back(inorm[2] - 1);

                    dump << "f "
                        << ipos[0] << "/" << iuv[0] << "/" << inorm[0] << " "
                        << ipos[1] << "/" << iuv[1] << "/" << inorm[1] << " "
                        << ipos[2] << "/" << iuv[2] << "/" << inorm[2] << std::endl;

                    if (vertexCount >= 4)
                    {
                        pidx.push_back(ipos[0] - 1);
                        pidx.push_back(ipos[2] - 1);
                        pidx.push_back(ipos[3] - 1);

                        uvidx.push_back(iuv[0] - 1);
                        uvidx.push_back(iuv[2] - 1);
                        uvidx.push_back(iuv[3] - 1);

                        nidx.push_back(inorm[0] - 1);
                        nidx.push_back(inorm[2] - 1);
                        nidx.push_back(inorm[3] - 1);

                        //dump.setf(std::ios::fixed, std::ios::floatfield);
                        //dump << std::setw(6);
                        //dump << std::fixed;
                        //dump.precision(6);
                        dump << "f " 
                            << ipos[0] << "/" << iuv[0] << "/" << inorm[0] << " "
                            << ipos[2] << "/" << iuv[2] << "/" << inorm[2] << " "
                            << ipos[3] << "/" << iuv[3] << "/" << inorm[3] << std::endl;
                    }
                }
            }
        }

        dump.close();
        objfile.close();

        // Set bounds
        _minimumBounds[0] = minPos[0];
        _minimumBounds[1] = minPos[1];
        _minimumBounds[2] = minPos[2];
        _maximumBounds[0] = maxPos[0];
        _maximumBounds[1] = maxPos[1];
        _maximumBounds[2] = maxPos[2];

        // Organize data to get triangles for positions 
        for (unsigned int i = 0; i < pidx.size() ; i++)
        {
            unsigned int vertexIndex = 3 * pidx[i];
            _positionData.push_back(pos[vertexIndex]);
            _positionData.push_back(pos[vertexIndex + 1]);
            _positionData.push_back(pos[vertexIndex + 2]);
        }

        // Organize data to get triangles for texture coordinates 
        for (unsigned int i = 0; i < uvidx.size(); i++)
        {
            unsigned int vertexIndex = 2 * uvidx[i];
            _texcoordData[0].push_back(uv[vertexIndex]);
            _texcoordData[0].push_back(uv[vertexIndex + 1]);
            _texcoordData[1].push_back(uv[vertexIndex]);
            _texcoordData[1].push_back(uv[vertexIndex + 1]);

            // Fake some colors
            _colorData[0].push_back(1.0f);
            _colorData[0].push_back(1.0f);
            _colorData[0].push_back(1.0f);
        }

        // Organize data to get triangles for normals 
        for (unsigned int i = 0; i < nidx.size(); i++)
        {
            unsigned int vertexIndex = 3 * nidx[i];
            _normalData.push_back(norm[vertexIndex]);
            _normalData.push_back(norm[vertexIndex + 1]);
            _normalData.push_back(norm[vertexIndex + 2]);

            // Fake some tangent, bitangent data
            _tangentData[0].push_back(norm[vertexIndex]);
            _tangentData[0].push_back(norm[vertexIndex + 1]);
            _tangentData[0].push_back(norm[vertexIndex + 2]);

            _bitangentData[0].push_back(norm[vertexIndex]);
            _bitangentData[0].push_back(norm[vertexIndex + 1]);
            _bitangentData[0].push_back(norm[vertexIndex + 2]);
        }

        // Set up flattened indexing
        if (_positionData.size() && pidx.size())
        {
            _indexing.resize(pidx.size());
            std::iota(_indexing.begin(), _indexing.end(), 0);
        }

        //printf(">>> Posidx size: %zd, uvindex size: %zd normindex size %zd. Final indexing size: %zd\n",
        //    pidx.size(), uvidx.size(), nidx.size(), _indexing.size());
        //printf(">>> Read in geometry min: %g,%g,%g. max: %g,%g,%g\n", _minimumBounds[0], _minimumBounds[1],
        //    _minimumBounds[2], _maximumBounds[0], _maximumBounds[1], _maximumBounds[2]);
    }
}

const MaterialX::Vector3& DefaultGeometryHandler::getMinimumBounds()
{
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        const float border = (float)_inputProperties.screenOffset;
        _minimumBounds[0] = border;
        _minimumBounds[1] = border;
        _minimumBounds[2] = 0.0f;
    }
    return _minimumBounds;
}

MaterialX::Vector3& DefaultGeometryHandler::getMaximumBounds()
{
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        const float border = (float)_inputProperties.screenOffset;
        const float bufferWidth = (float)_inputProperties.screenWidth;
        const float bufferHeight = (float)_inputProperties.screenHeight;
        _maximumBounds[0] = bufferWidth - border;
        _maximumBounds[1] = bufferHeight - border;
        _maximumBounds[2] = 0.0f;
    }
    return _maximumBounds;
}


GeometryHandler::IndexBuffer& DefaultGeometryHandler::getIndexing()
{
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (_indexing.empty())
        {
            _indexing = {
                0, 1, 2, 0, 2, 3
            };
        }
    }
    return _indexing;
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getPositions(unsigned int& stride, unsigned int /*index*/)
{
    stride = 3;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        const float border = (float)_inputProperties.screenOffset;
        const float bufferWidth = (float)_inputProperties.screenWidth;
        const float bufferHeight = (float)_inputProperties.screenHeight;

        _minimumBounds[0] = border;
        _minimumBounds[1] = border;
        _minimumBounds[2] = 0.0f;
        _maximumBounds[0] = bufferWidth - border;
        _maximumBounds[1] = bufferHeight - border;
        _maximumBounds[2] = 0.0f;

        if (_positionData.empty())
        {
            _positionData = 
            {
                border, border, 0.0f,
                    border, (bufferHeight - border), 0.0f,
                    (bufferWidth - border), (bufferHeight - border), 0.0f,
                    (bufferWidth - border), border, 0.0f
            };
        }
    }
    return _positionData;
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getNormals(unsigned int& stride, unsigned int /*index*/)
{
    stride = 3;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (_normalData.empty())
        {
            _normalData = {
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f
            };
        }
    }
    return _normalData;
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getTextureCoords(unsigned int& stride, unsigned int index)
{
    stride = 2;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (index == 0)
        {
            if (_texcoordData[0].empty())
            {
                _texcoordData[0] = {
                    0.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,
                    1.0f, 0.0f
                };
            }
        }
        else
        {
            if (_texcoordData[1].empty())
            {
                _texcoordData[1] = {
                    1.0f, 0.0f,
                    0.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f
                };
            }
            return _texcoordData[1];
        }
    }
    return _texcoordData[0];
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getTangents(unsigned int& stride, unsigned int index)
{
    stride = 3;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (index == 0)
        {
            if (_tangentData[0].empty())
            {
                _tangentData[0] = {
                    .0f, 1.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    0.0f, -1.0f, 0.0f
                };
            }
        }
        else
        {
            if (_tangentData[1].empty())
            {
                _tangentData[1] = {
                    0.0f, -1.0f, 0.0f,
                    .0f, 1.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f
                };
            }
            return _tangentData[1];
        }
    }
    return _tangentData[0];
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getBitangents(unsigned int& stride, unsigned int index)
{
    stride = 3;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (index == 0)
        {
            if (_bitangentData[0].empty())
            {
                _bitangentData[0] = {
                    1.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    0.0f, -1.0f, 0.0f
                };
            }
        }
        else
        {
            if (_bitangentData[1].empty())
            {
                _bitangentData[1] = {
                    0.0f, -1.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f
                };
            }
            return _bitangentData[1];
        }        
    }
    return _bitangentData[0];
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getColors(unsigned int& stride, unsigned int index)
{
    stride = 4;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (index == 0)
        {
            if (_colorData[0].empty())
            {
                _colorData[0] = {
                    1.0f, 0.0f, 0.0f, 1.0f,
                    0.0f, 1.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, 1.0f, 0.0f, 1.0f
                };
            }
        }
        else
        {
            if (_colorData[1].empty())
            {
                _colorData[1] = {
                    1.0f, 0.0f, 1.0f, 1.0f,
                    0.0f, 1.0f, 1.0f, 1.0f,
                    1.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, 1.0f, 0.5f, 1.0f
                };
            }
            return _colorData[1];
        }
    }
    return _colorData[0];
}

DefaultGeometryHandler::FloatBuffer& DefaultGeometryHandler::getAttribute(const std::string& attributeType, 
                                                                          unsigned int& stride, 
                                                                          unsigned int index)
{
    if (attributeType.compare(0, 
        DefaultGeometryHandler::POSITION_ATTRIBUTE.size(),
        DefaultGeometryHandler::POSITION_ATTRIBUTE) == 0)
    {
        return getPositions(stride, index);
    }
    else if (attributeType.compare(0,
        DefaultGeometryHandler::NORMAL_ATTRIBUTE.size(),
        DefaultGeometryHandler::NORMAL_ATTRIBUTE) == 0)
    {
        return getNormals(stride, index);
    }
    else if (attributeType.compare(0,
        DefaultGeometryHandler::TEXCOORD_ATTRIBUTE.size(),
        DefaultGeometryHandler::TEXCOORD_ATTRIBUTE) == 0)
    {
        return getTextureCoords(stride, index);
    }
    else if (attributeType.compare(0,
        DefaultGeometryHandler::COLOR_ATTRIBUTE.size(),
        DefaultGeometryHandler::COLOR_ATTRIBUTE) == 0)
    {
        return getColors(stride, index);
    }
    else if (attributeType.compare(0,
        DefaultGeometryHandler::TANGENT_ATTRIBUTE.size(),
        DefaultGeometryHandler::TANGENT_ATTRIBUTE) == 0)
    {
        return getTangents(stride, index);
    }
    else if (attributeType.compare(0,
        DefaultGeometryHandler::BITANGENT_ATTRIBUTE.size(),
        DefaultGeometryHandler::BITANGENT_ATTRIBUTE) == 0)
    {
        return getBitangents(stride, index);
    }
    return getPositions(stride, index);
}


}