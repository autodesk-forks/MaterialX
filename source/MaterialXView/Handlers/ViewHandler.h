#ifndef MATERIALX_VIEWHANDLER_H
#define MATERIALX_VIEWHANDLER_H

#include <MaterialXCore/Types.h>
#include <memory>

namespace MaterialX
{

/// Shared pointer to a ViewHandler
using ViewHandlerPtr = std::shared_ptr<class ViewHandler>;

/// @class @ViewHandler
/// Utility view handler for creating and providing 
/// View data for shader binding.
///
class ViewHandler
{
  public:
    /// Static instance creator
    static ViewHandlerPtr creator() { return std::make_shared<ViewHandler>(); }

    /// Default constructor
    ViewHandler() {};
    
    /// Default destructor
    virtual ~ViewHandler() {};

    /// @name View matrices and vectors
    /// @{

    /// Set a matrix to a perspective projection
    /// @param fov Field of view
    /// @param aspectRatio Aspect ration (viewport width /  viewport height)
    /// @param nearClipPlane Near clip plane
    /// @param farClipPlane Far clip plane
    void setPerspectiveProjectionMatrix(float fov,
                                        float aspectRatio,
                                        float nearClipPlane,
                                        float farClipPlane);

    /// Set a matrix to an orthographic projection
    /// @param nearClipPlane Near clip plane
    /// @param farClipPlane Far clip plane
    void setOrthoGraphicProjectionMatrix(float left,
                                         float right,
                                         float bottom,
                                         float top,
                                         float nearClipPlane,
                                         float farClipPlane);

    /// Set the world matrix
    void setWorldMatrix(Matrix4x4& matrix)
    {
        _worldMatrix = matrix;
    }

    /// Get the projection matrix
    Matrix4x4& projectionMatrix()
    {
        return _projectionMatrix;
    }

    /// Get the view matrix
    Matrix4x4& viewMatrix()
    {
        return _viewMatrix;
    }

    /// Get the view position
    Vector3& viewPosition()
    {
        return _viewPosition;
    }

    /// Get the view direction
    Vector3& viewDirection()
    {
        return _viewDirection;
    }

    /// Get the world matrix
    Matrix4x4& worldMatrix()
    {
        return _worldMatrix;
    }

    /// @}
    /// @name General utilities
    /// @{

    /// Modify matrix by a given translation amount
    /// @param matrix Matrix to modify.
    /// @param vector Translation amount
    void translateMatrix(Matrix4x4& matrix, Vector3 vector);

    /// Multiply two matricies and return the result
    /// @param matrix1 First matrix 
    /// @param matrix2 Second matrix 
    /// @param result Resulting matrix
    void multiplyMatrix(const Matrix4x4& matrix1, const Matrix4x4& matrix2, Matrix4x4& result);

    /// Convert from degress to radians
    /// @param degrees Degree value
    /// @return value converted to radians
    float degreesToRadians(float degrees);

    /// Get lenth of a vector
    float length(const Vector3& vector);

    /// Set 4x4 matrix to be identity
    /// @param matrix Matrix to modify.
    void makeIdentityMatrix(Matrix4x4& matrix);

    /// PI
    static float PI_VALUE;

    /// @}

  protected:
    /// World matrix
    Matrix4x4 _worldMatrix;
    /// View matrix
    Matrix4x4 _viewMatrix;
    /// View position
    Vector3 _viewPosition;
    /// View direction
    Vector3 _viewDirection;
    /// Projection matrix
    Matrix4x4 _projectionMatrix;
};

} // namespace MaterialX

#endif
