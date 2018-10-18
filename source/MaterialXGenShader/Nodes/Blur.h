#ifndef MATERIALX_BLUR_H
#define MATERIALX_BLUR_H

#include <MaterialXGenShader/Nodes/Convolution.h>

namespace MaterialX
{

/// Implementation of blur convolution
class Blur : public Convolution
{
  public:
    using ParentClass = Convolution;

    static ShaderImplementationPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

  protected:
    /// Constructor
    Blur();

    /// Box filter option on blur
    static string BOX_FILTER;
    /// Box filter weights variable name
    static string BOX_WEIGHTS_VARIABLE;

    /// Gaussian filter option on blur
    static string GAUSSIAN_FILTER;
    /// Gaussian filter weights variable name
    static string GAUSSIAN_WEIGHTS_VARIABLE;

    /// Return if given type is an acceptible input
    bool acceptsInputType(const TypeDesc* type) override;
    
    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, StringVec& offsetStrings) override;

    /// Name of weight array variable
    std::string _weightArrayVariable;

    /// Type of filter 
    string _filterType;

    /// Width of filter
    unsigned int _filterWidth;

    /// Language dependent input type string
    string _inputTypeString;
};

} // namespace MaterialX

#endif
