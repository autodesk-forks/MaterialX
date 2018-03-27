#include <MaterialXView/Handlers/ViewHandler.h>
#include <cmath>

namespace MaterialX
{ 

float ViewHandler::PI_VALUE = 3.14159265358979323846f;

float ViewHandler::degreesToRadians(float degrees)
{
    return (degrees * PI_VALUE / 180.0f);
}

void ViewHandler::makeIdentityMatrix(Matrix4x4& matrix)
{
    matrix.data.fill(0.0f);
    matrix[0] = 1.0f;
    matrix[5] = 1.0f;
    matrix[10] = 1.0f;
    matrix[15] = 1.0f;
}

void ViewHandler::createProjectionMatrix(Matrix4x4& matrix,
                                        float fov,
                                        float aspectRatio,
                                        float nearClipPlane,
                                        float farClipPlane)
{
    matrix.data.fill(0.0f);

    float scaley = 1.0f / std::tan(degreesToRadians(fov / 2.0f));
    float scalex = scaley / aspectRatio;
    float clipDistance = farClipPlane - nearClipPlane;

    matrix[0] = scalex;
    matrix[5] = scaley;
    matrix[10] = -(nearClipPlane + farClipPlane) / clipDistance;
    matrix[11] = -1;
    matrix[14] = -((2.0f * nearClipPlane * farClipPlane) / clipDistance);
}

void ViewHandler::createOrthoGraphicMatrix(Matrix4x4& matrix,
                                            float left,
                                            float right,
                                            float bottom,
                                            float top,
                                            float nearClipPlane,
                                            float farClipPlane)
{
    matrix.data.fill(0.0f);

    float clipDistance = farClipPlane - nearClipPlane;

    matrix[0] = 2.0f / (right - left);
    matrix[5] = 2.0f / (top - bottom);
    matrix[10] = -2.0f / clipDistance;
    matrix[12] = -(right + left) / (right - left);
    matrix[13] = -(top + bottom) / (top - bottom);
    matrix[14] = -(farClipPlane + nearClipPlane) / clipDistance;
    matrix[15] = 1.0f;
}


void ViewHandler::translateMatrix(Matrix4x4& matrix, Vector3 vector)
{
    matrix[12] += vector[0];
    matrix[13] += vector[1];
    matrix[14] += vector[2];
}

}
