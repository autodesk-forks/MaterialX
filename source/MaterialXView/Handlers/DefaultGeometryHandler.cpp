#include <MaterialXView/Handlers/DefaultGeometryHandler.h>

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
        if (_positionData.empty())
        {
            const float border = (float)_inputProperties.screenOffset;
            const float bufferWidth = (float)_inputProperties.screenWidth;
            const float bufferHeight = (float)_inputProperties.screenHeight;

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