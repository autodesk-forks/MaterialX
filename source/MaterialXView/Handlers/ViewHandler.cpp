#include <MaterialXView/Handlers/ViewHandler.h>
#include <cmath>

namespace MaterialX
{ 

float ViewHandler::PI_VALUE = 3.14159265358979323846f;

float ViewHandler::degreesToRadians(float degrees)
{
    return (degrees * PI_VALUE / 180.0f);
}

float ViewHandler::length(const Vector3& vector)
{
    return std::sqrtf(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
}


void ViewHandler::makeIdentityMatrix(Matrix4x4& matrix)
{
    matrix.data.fill(0.0f);
    matrix[0] = 1.0f;
    matrix[5] = 1.0f;
    matrix[10] = 1.0f;
    matrix[15] = 1.0f;
}

void ViewHandler::setPerspectiveProjectionMatrix(float fov,
                                                 float aspectRatio,
                                                 float nearClipPlane,
                                                 float farClipPlane)
{
    _projectionMatrix.data.fill(0.0f);

    float scaley = 1.0f / std::tan(degreesToRadians(fov / 2.0f));
    float scalex = scaley / aspectRatio;
    float clipDistance = farClipPlane - nearClipPlane;

    _projectionMatrix[0] = scalex;
    _projectionMatrix[5] = scaley;
    _projectionMatrix[10] = -(nearClipPlane + farClipPlane) / clipDistance;
    _projectionMatrix[11] = -1;
    _projectionMatrix[14] = -((2.0f * nearClipPlane * farClipPlane) / clipDistance);
}

void ViewHandler::setOrthoGraphicProjectionMatrix(float left,
                                                  float right,
                                                  float bottom,
                                                  float top,
                                                  float nearClipPlane,
                                                  float farClipPlane)
{
    _projectionMatrix.data.fill(0.0f);

    float clipDistance = farClipPlane - nearClipPlane;

    _projectionMatrix[0] = 2.0f / (right - left);
    _projectionMatrix[5] = 2.0f / (top - bottom);
    _projectionMatrix[10] = -2.0f / clipDistance;
    _projectionMatrix[12] = -(right + left) / (right - left);
    _projectionMatrix[13] = -(top + bottom) / (top - bottom);
    _projectionMatrix[14] = -(farClipPlane + nearClipPlane) / clipDistance;
    _projectionMatrix[15] = 1.0f;
}


void ViewHandler::translateMatrix(Matrix4x4& matrix, Vector3 vector)
{
    matrix[12] += vector[0];
    matrix[13] += vector[1];
    matrix[14] += vector[2];
}

void ViewHandler::multiplyMatrix(const Matrix4x4& m1, const Matrix4x4& m2, Matrix4x4& result)
{
    result[0] = m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3];
    result[1] = m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3];
    result[2] = m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3];
    result[3] = m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3];
    result[4] = m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7];
    result[5] = m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7];
    result[6] = m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7];
    result[7] = m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7];
    result[8] = m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11];
    result[9] = m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11];
    result[10] = m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11];
    result[11] = m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11];
    result[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15];
    result[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15];
    result[14] = m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15];
    result[15] = m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15];
}


}
