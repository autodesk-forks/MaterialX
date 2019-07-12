//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/ImageNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

namespace MaterialX
{

ShaderNodeImplPtr ImageNode::create()
{
    return std::make_shared<ImageNode>();
}

void ImageNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    SourceCodeNode::emitFunctionCall(node, context, stage);
}

} // namespace MaterialX
