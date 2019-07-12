//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_HWIMAGECODENODE_H
#define MATERIALX_HWIMAGECODENODE_H

#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>

namespace MaterialX
{

/// Extending the HwSourceCodeNodewith requirements for image nodes.
class HwImageNode : public HwSourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
