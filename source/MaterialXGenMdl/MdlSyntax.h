//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_MDLSYNTAX_H
#define MATERIALX_MDLSYNTAX_H

/// @file
/// OSL syntax class

#include <MaterialXGenShader/Syntax.h>

namespace MaterialX
{

/// @class MdlSyntax
/// Syntax class for MDL (Material Definition Language)
class MdlSyntax : public Syntax
{
public:
    MdlSyntax();

    static SyntaxPtr create() { return std::make_shared<MdlSyntax>(); }

    const string& getConstantQualifier() const override { return CONST_QUALIFIER; };
    const string& getUniformQualifier() const override { return UNIFORM_QUALIFIER; };

    static const string CONST_QUALIFIER;
    static const string UNIFORM_QUALIFIER;
    static const StringVec VECTOR2_MEMBERS;
    static const StringVec VECTOR3_MEMBERS;
    static const StringVec VECTOR4_MEMBERS;
    static const StringVec COLOR2_MEMBERS;
    static const StringVec COLOR3_MEMBERS;
    static const StringVec COLOR4_MEMBERS;
};

} // namespace MaterialX

#endif
