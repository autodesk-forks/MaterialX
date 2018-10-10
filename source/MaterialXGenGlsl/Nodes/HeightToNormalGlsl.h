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

    virtual void computeSampleOffsetStrings(const string& sampleSizeName, StringVec& offsetStrings) override;
    void emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;

  protected:
    bool acceptsInputType(const TypeDesc* type) override;

    /// Constructor
    HeightToNormalGlsl();

    /// Name of filter function to call to compute normals from input samples
    string _filterFunctionName;
};

} // namespace MaterialX

#endif
