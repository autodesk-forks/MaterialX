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

    /// PI
    static float PI_VALUE;

    /// Convert from degress to radians
    /// @param degrees Degree value
    /// @return value converted to radians
    float degreesToRadians(float degrees);

    /// Set 4x4 matrix to be identity
    /// @param matrix Matrix to modify.
    void makeIdentityMatrix(Matrix4x4& matrix);

    /// Set a matrix to a perspective projection
    /// @param matrix Matrix to modify.
    /// @param fov Field of view
    /// @param aspectRatio Aspect ration (viewport width /  viewport height)
    /// @param nearClipPlane Near clip plane
    /// @param farClipPlane Far clip plane
    void createProjectionMatrix(Matrix4x4& matrix,
                                float fov,
                                float aspectRatio,
                                float nearClipPlane,
                                float farClipPlane);

    /// Set a matrix to an orthographic projection
    /// @param matrix Matrix to modify.
    /// @param nearClipPlane Near clip plane
    /// @param farClipPlane Far clip plane
    void createOrthoGraphicMatrix(Matrix4x4& matrix,
                                  float left,
                                  float right,
                                  float bottom,
                                  float top,
                                  float nearClipPlane,
                                  float farClipPlane);

    /// Modify matrix by a given translation amount
    /// @param matrix Matrix to modify.
    /// @param vector Translation amount
    void translateMatrix(Matrix4x4& matrix, Vector3 vector);

};

} // namespace MaterialX

#endif
