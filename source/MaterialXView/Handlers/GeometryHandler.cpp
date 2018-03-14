#include <MaterialXView/Handlers/GeometryHandler.h>

namespace MaterialX
{ 
const std::string GeometryHandler::SCREEN_ALIGNED_QUAD("screen_quad");

GeometryHandler::GeometryHandler() :
    _indexing(nullptr),
    _positionData(nullptr),
    _normalData(nullptr),
    _texcoordData(nullptr),
    _tangentData(nullptr),
    _bitangentData(nullptr),
    _colorData(nullptr),
    _identifier(SCREEN_ALIGNED_QUAD)
{
}

GeometryHandler::~GeometryHandler() 
{
    clearData();
}

void GeometryHandler::clearData()
{
    delete[] _indexing;
    delete[] _positionData;
    delete[] _normalData;
    delete[] _texcoordData;
    delete[] _tangentData;
    delete[] _bitangentData;
    delete[] _colorData;
}

void GeometryHandler::setIdentifier(const std::string identifier)
{
    if (identifier != _identifier)
    {
        _identifier = identifier;
        clearData();
    }
}

unsigned int* GeometryHandler::getIndexing(size_t &bufferSize)
{
    // Change this to 0,0, 1,1 and scale the obj->world matrix
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (!_indexing)
        {
            _indexing = new unsigned int[6]{
                0, 1, 2, 0, 2, 3
            };
        }
    }
    bufferSize = 6*sizeof(unsigned int);
    return _indexing;
}

float* GeometryHandler::getPositions(size_t &bufferSize)
{
    if (!_positionData)
    {
        // Change this to 0,0, 1,1 and scale the obj->world matrix
        if (_identifier == SCREEN_ALIGNED_QUAD)
        {
            const float border = 20.0f;
            const float bufferWidth = 512.0f;
            const float bufferHeight = 512.0f;
            _positionData = new float[12]
            {
                border, border, 0.0f,
                    border, (bufferHeight - border), 0.0f,
                    (bufferWidth - border), (bufferHeight - border), 0.0f,
                    (bufferWidth - border), border, 0.0f
            };
        }
    }
    bufferSize = 12 * sizeof(float);
    return _positionData;
}

float* GeometryHandler::getNormals(size_t &bufferSize)
{
    if (!_normalData)
    {
        if (_identifier == SCREEN_ALIGNED_QUAD)
        {
            _normalData = new float[12]{
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f
            };
        }
    }
    bufferSize = 12 * sizeof(float);
    return _normalData;
}

float* GeometryHandler::getTextureCoords(const std::string& setIdentifier, size_t &bufferSize)
{
    if (!_texcoordData)
    {
        if (_identifier == SCREEN_ALIGNED_QUAD)
        {
            if (setIdentifier == "0")
            {
                _texcoordData = new float[8]{
                    0.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,
                    1.0f, 0.0f
                };
            }
        }
    }
    bufferSize = 8 * sizeof(float);
    return _texcoordData;
}

float* GeometryHandler::getTangents(const std::string& setIdentifier, size_t &bufferSize)
{
    if (!_tangentData)
    {
        if (_identifier == SCREEN_ALIGNED_QUAD)
        {
            if (setIdentifier == "0")
            {
                _tangentData = new float[12]{
                    .0f, 1.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    0.0f, -1.0f, 0.0f
                };
            }
        }
    }
    bufferSize = 12 * sizeof(float);
    return _tangentData;
}

float* GeometryHandler::getBitangents(const std::string& setIdentifier, size_t &bufferSize)
{
    if (!_bitangentData)
    {
        if (_identifier == SCREEN_ALIGNED_QUAD)
        {
            if (setIdentifier == "0")
            {
                _bitangentData = new float[12]{
                    1.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    0.0f, -1.0f, 0.0f
                };
            }
        }
    }
    bufferSize = 12 * sizeof(float);
    return _bitangentData;
}

float* GeometryHandler::getColors(const std::string& setIdentifier, size_t &bufferSize)
{
    if (!_colorData)
    {
        if (_identifier == SCREEN_ALIGNED_QUAD)
        {
            if (setIdentifier == "0")
            {
                _colorData = new float[16]{
                    1.0f, 0.0f, 0.0f, 1.0f,
                    0.0f, 1.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, 1.0f, 0.0f, 1.0f
                };
            }
        }
    }
    bufferSize = 16 * sizeof(float);
    return _colorData;
}

}