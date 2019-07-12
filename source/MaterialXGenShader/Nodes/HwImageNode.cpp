//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>
#include <MaterialXGenShader/Nodes/HwImageNode.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <iostream>

namespace MaterialX
{

ShaderNodeImplPtr HwImageNode::create()
{
    return std::make_shared<HwImageNode>();
}

void HwImageNode::addInputs(ShaderNode& node, GenContext&) const
{
    // Add additional scale and offset inputs
    ShaderInput* input = node.addInput("uv_scale", Type::VECTOR2);
    input->setValue(Value::createValue<Vector2>(Vector2(1.0f, 1.0f)));
    input = node.addInput("uv_offset", Type::VECTOR2);
    input->setValue(Value::createValue<Vector2>(Vector2(0.0f, 0.0f)));
}

void HwImageNode::setValues(const Node& node, ShaderNode& shaderNode, GenContext& context) const
{
    if (context.getOptions().normalizeUdimTexCoords)
    {
        ParameterPtr file = node.getParameter("file");
        if (file)
        {
            // set the uv scale and offset properly.
            const string& fileName = file->getValueString();
            if (fileName.find(UDIM_TOKEN) != string::npos)
            {
                ValuePtr udimSetValue = node.getDocument()->getGeomAttrValue("udimset");
                if (udimSetValue && udimSetValue->isA<StringVec>())
                {
                    const StringVec& udimIdentifiers = udimSetValue->asA<StringVec>();
                    vector<Vector2> udimCoordinates{ getUdimCoordinates(udimIdentifiers) };

                    Vector2 scaleUV{ 1.0f, 1.0f };
                    Vector2 offsetUV{ 0.0f, 0.0f };
                    getUdimScaleAndOffset(udimCoordinates, scaleUV, offsetUV);

                    ShaderInput* input = shaderNode.getInput("uv_scale");
                    if (input)
                    {
                        //std::cout << "Handle UDIMS scale: " << std::to_string(scaleUV[0]) 
                        //    << " , " << std::to_string(scaleUV[1])
                        //    << std::endl;
                        input->setValue(Value::createValue<Vector2>(scaleUV));
                    }
                    input = shaderNode.getInput("uv_offset");
                    if (input)
                    {
                        //std::cout << "Handle UDIMS offset: " << std::to_string(offsetUV[0])
                        //    << " , " << std::to_string(offsetUV[1])
                        //    << std::endl;
                        input->setValue(Value::createValue<Vector2>(offsetUV));
                    }
                }
            }
        }
    }
}

void HwImageNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    HwSourceCodeNode::emitFunctionCall(node, context, stage);
}

} // namespace MaterialX
