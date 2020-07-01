//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/ThinFilmNodeGlsl.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Nodes/LayerNode.h>

namespace MaterialX
{

const string ThinFilmNodeGlsl::THICKNESS = "thickness";
const string ThinFilmNodeGlsl::IOR       = "ior";

ShaderNodeImplPtr ThinFilmNodeGlsl::create()
{
    return std::make_shared<ThinFilmNodeGlsl>();
}

const string ThinFilmSupportGlsl::THINFILM_THICKNESS = "thinfilm_thickness";
const string ThinFilmSupportGlsl::THINFILM_IOR       = "thinfilm_ior";

ShaderNodeImplPtr ThinFilmSupportGlsl::create()
{
    return std::make_shared<ThinFilmSupportGlsl>();
}

void ThinFilmSupportGlsl::addInputs(ShaderNode& node, GenContext&) const
{
    // Add additional thickness and ior inputs for thin-film support.
    ShaderInput* thickness = node.addInput(THINFILM_THICKNESS, Type::FLOAT);
    thickness->setValue(Value::createValue<float>(0.0f));
    ShaderInput* ior = node.addInput(THINFILM_IOR, Type::FLOAT);
    ior->setValue(Value::createValue<float>(1.5f));
}

} // namespace MaterialX
