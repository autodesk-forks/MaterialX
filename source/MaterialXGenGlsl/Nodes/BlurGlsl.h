#ifndef MATERIALX_BLURGLSL_H
#define MATERIALX_BLURGLSL_H

#include <MaterialXGenGlsl/Nodes/ConvolutionGlsl.h>

namespace MaterialX
{

/// Implementation of blur for GLSL
class BlurGlsl : public ConvolutionGlsl
{
  public:
    using ParentClass = ConvolutionGlsl;

    static SgImplementationPtr create();

    void computeSampleOffsetStrings(const string& sampleSizeName, StringVec& offsetStrings) override;
    void emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;

  protected:
    /// Constructor
    BlurGlsl();

    bool acceptsInput(SgOutput& input) override;

    /// Name of filter function to call to compute normals from input samples
    std::string _filterFunctionName;

    string _filterType;
    string _inputTypeString;
};

} // namespace MaterialX

#endif
