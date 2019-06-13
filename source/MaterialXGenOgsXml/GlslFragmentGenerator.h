//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GLSLFRAGMENTGENERATOR_H
#define MATERIALX_GLSLFRAGMENTGENERATOR_H

/// @file
/// GLSL fragment generator

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

using GlslFragmentGeneratorPtr = shared_ptr<class GlslFragmentGenerator>;

/// GLSL shader generator specialized for usage in OGS fragment wrappers.
class GlslFragmentGenerator : public GlslShaderGenerator
{
public:
    GlslFragmentGenerator();

    static ShaderGeneratorPtr create();

    const string& getTarget() const override { return TARGET; }

    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    static const string TARGET;

protected:
    static void toVec3(const TypeDesc* type, string& variable);
};

}

#endif
