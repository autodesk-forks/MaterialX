//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/ThinFilmNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Nodes/LayerNode.h>

namespace MaterialX
{

const string ThinFilmNode::THICKNESS = "thickness";
const string ThinFilmNode::IOR       = "ior";

ShaderNodeImplPtr ThinFilmNode::create()
{
    return std::make_shared<ThinFilmNode>();
}

const string ThinFilmSupport::THINFILM_THICKNESS = "thinfilm_thickness";
const string ThinFilmSupport::THINFILM_IOR       = "thinfilm_ior";

ShaderNodeImplPtr ThinFilmSupport::create()
{
    return std::make_shared<ThinFilmSupport>();
}

void ThinFilmSupport::addInputs(ShaderNode& node, GenContext&) const
{
    // Add additional thickness and ior inputs for thin-film support.
    ShaderInput* thickness = node.addInput(THINFILM_THICKNESS, Type::FLOAT);
    thickness->setValue(Value::createValue<float>(0.0f));
    ShaderInput* ior = node.addInput(THINFILM_IOR, Type::FLOAT);
    ior->setValue(Value::createValue<float>(1.5f));
}

} // namespace MaterialX
