#ifndef MATERIALX_CONVOLUTIONGLSL_H
#define MATERIALX_CONVOLUTIONGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Nodes/Compound.h>

namespace MaterialX
{

/// GLSL utility class for implementations of nodes which perform convolutions
///
/// Note that we assume the sample grid is 3 x 3 in size and is organized as follows
/// in array index order.
/// 
/// ----+-----+----
///  0  |  1  | 2
/// ----+-----+----
///  3  |  4  | 5
/// ----+-----+----
///  6  |  7  | 8
/// ----+-----+----
///
class ConvolutionGlsl : public SgImplementation
{
  protected:
    /// Constructor
    ConvolutionGlsl();

    /// Generate upstream / input sample code and cache the output variable names which 
    /// will hold the sample values after execution.
    void emitInputSamples(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, HwShader& shader);
    
    /// Number of samples. Default value is 9.
    unsigned int _sampleCount;

    /// Filter size. Default value is 1
    float _filterSize;

    /// Filter offset. Default value is 0
    float _filterOffset;

    /// List of variables names which contain input sample values
    StringVec _sampleStrings;

    /// Name of function to compute sample size in uv space. Takes uv, filter size, and filter offset
    /// as input, and return a 2 channel vector as output
    std::string _sampleSizeFunctionUV;
};

} // namespace MaterialX

#endif
