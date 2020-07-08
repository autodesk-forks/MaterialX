//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_THINFILMNODE_H
#define MATERIALX_THINFILMNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>

namespace MaterialX
{

/// Thin-Film node.
/// No real implementation in GLSL. Its inputs will just be copied
/// to the corresponding microfacet BSDF to enabled thin-film there.
class ThinFilmNode : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    /// String constants
    static const string THICKNESS;
    static const string IOR;
};

/// Base class for microfacet BSDF nodes that support layering with thin-film.
/// Thin-film thickness and ior are added as extra inputs to BSDF nodes that
/// derive from this class.
class ThinFilmSupport : public HwSourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;

    /// String constants
    static const string THINFILM_THICKNESS;
    static const string THINFILM_IOR;
};

} // namespace MaterialX

#endif
