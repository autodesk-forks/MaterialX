#ifndef MATERIALX_HEIGHTTONORMALGLSL_H
#define MATERIALX_HEIGHTTONORMALGLSL_H

#include <MaterialXGenGlsl/Nodes/ConvolutionGlsl.h>

namespace MaterialX
{

/// Implementation of height-field to normal for GLSL
class HeightToNormalGlsl : public ConvolutionGlsl
{
  public:
    using ParentClass = ConvolutionGlsl;

    static SgImplementationPtr create();

    void emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;

  protected:
    /// Constructor
    HeightToNormalGlsl();

    /// Name of filter function to call to compute normals from input samples
    std::string _filterFunctionName;
};

} // namespace MaterialX

#endif
