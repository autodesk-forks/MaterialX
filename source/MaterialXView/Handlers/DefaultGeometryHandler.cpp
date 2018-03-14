#include <MaterialXView/Handlers/DefaultGeometryHandler.h>

namespace MaterialX
{ 
DefaultGeometryHandler::DefaultGeometryHandler() :
    GeometryHandler(),
    _indexing(nullptr),
    _positionData(nullptr),
    _normalData(nullptr),
    _texcoordData(nullptr),
    _tangentData(nullptr),
    _bitangentData(nullptr),
    _colorData(nullptr)
{
}

DefaultGeometryHandler::~DefaultGeometryHandler()
{
    clearData();
}

void DefaultGeometryHandler::clearData()
{
    delete[] _indexing;
    delete[] _positionData;
    delete[] _normalData;
    delete[] _texcoordData;
    delete[] _tangentData;
    delete[] _bitangentData;
    delete[] _colorData;
}

void DefaultGeometryHandler::setIdentifier(const std::string identifier)
{
    if (identifier != _identifier)
    {
        _identifier = identifier;
        clearData();
    }
}

unsigned int* DefaultGeometryHandler::getIndexing(size_t &bufferSize, unsigned int& indexCount)
{
    bufferSize = 0;
    indexCount = 0;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (!_indexing)
        {
            _indexing = new unsigned int[6]{
                0, 1, 2, 0, 2, 3
            };
        }
        bufferSize = 6 * sizeof(unsigned int);
        indexCount = 6;
    }
    return _indexing;
}

float* DefaultGeometryHandler::getPositions(size_t &bufferSize, 
                                            unsigned screenWidth, unsigned int screenHeight)
{
    bufferSize = 0;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (!_positionData)
        {
            const float border = 20.0f;
            const float bufferWidth = (float)screenWidth;
            const float bufferHeight = (float)screenHeight;
            printf("position data: buffw = %g, bufh = %g\n", bufferWidth, bufferHeight);
            _positionData = new float[12]
            {
                border, border, 0.0f,
                    border, (bufferHeight - border), 0.0f,
                    (bufferWidth - border), (bufferHeight - border), 0.0f,
                    (bufferWidth - border), border, 0.0f
            };
        }
        bufferSize = 12 * sizeof(float);
    }
    return _positionData;
}

float* DefaultGeometryHandler::getNormals(size_t &bufferSize)
{
    bufferSize = 0;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (!_normalData)
        {
            _normalData = new float[12]{
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f
            };
        }
        bufferSize = 12 * sizeof(float);
    }
    return _normalData;
}

float* DefaultGeometryHandler::getTextureCoords(const std::string& setIdentifier, size_t &bufferSize)
{
    bufferSize = 0;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (!_texcoordData)
        {
            if (setIdentifier.empty())
            {
                _texcoordData = new float[8]{
                    0.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,
                    1.0f, 0.0f
                };
            }
            else
            {
                _texcoordData = new float[8]{
                    1.0f, 0.0f,
                    0.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f
                };
            }
        }
        bufferSize = 8 * sizeof(float);
    }
    return _texcoordData;
}

float* DefaultGeometryHandler::getTangents(const std::string& setIdentifier, size_t &bufferSize)
{
    bufferSize = 0;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (!_tangentData)
        {
            if (setIdentifier.empty())
            {
                _tangentData = new float[12]{
                    .0f, 1.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    0.0f, -1.0f, 0.0f
                };
            }
            else
            {
                _tangentData = new float[12]{
                    0.0f, -1.0f, 0.0f,
                    .0f, 1.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f
                };
            }
        }
        bufferSize = 12 * sizeof(float);
    }
    return _tangentData;
}

float* DefaultGeometryHandler::getBitangents(const std::string& setIdentifier, size_t &bufferSize)
{
    bufferSize = 0;
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (!_bitangentData)
        {
            if (setIdentifier.empty())
            {
                _bitangentData = new float[12]{
                    1.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    0.0f, -1.0f, 0.0f
                };
            }
            else
            {
                _bitangentData = new float[12]{
                    0.0f, -1.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f
                };
            }
        }
        bufferSize = 12 * sizeof(float);
    }
    return _bitangentData;
}

float* DefaultGeometryHandler::getColors(const std::string& setIdentifier, size_t &bufferSize)
{
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (!_colorData)
        {
            if (setIdentifier.empty())
            {
                _colorData = new float[16]{
                    1.0f, 0.0f, 0.0f, 1.0f,
                    0.0f, 1.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, 1.0f, 0.0f, 1.0f
                };
            }
            else
            {
                _colorData = new float[16]{
                    1.0f, 0.0f, 1.0f, 1.0f,
                    0.0f, 1.0f, 1.0f, 1.0f,
                    1.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, 1.0f, 0.5f, 1.0f
                };
            }
        }
        bufferSize = 16 * sizeof(float);
    }
    return _colorData;
}

}