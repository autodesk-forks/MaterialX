//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <MaterialXGenMdl/Nodes/BlurNodeMdl.h>

namespace MaterialX
{

BlurNodeMdl::BlurNodeMdl() :
    BlurNode()
{
}

ShaderNodeImplPtr BlurNodeMdl::create()
{
    return std::shared_ptr<BlurNodeMdl>(new BlurNodeMdl());
}

void BlurNodeMdl::outputSampleArray(const ShaderGenerator& shadergen, ShaderStage& stage,
                                 const string& inputTypeString,
                                 const string& sampleName, const StringVec& sampleStrings) const
{
    const string arrayDeclaration = inputTypeString + "[]";
    shadergen.emitLine(arrayDeclaration + " " + sampleName + " = " + arrayDeclaration + "(", stage, false);
    size_t sampleCount = sampleStrings.size();
    for (size_t i = 0; i < sampleCount; i++)
    {
        shadergen.emitLineBegin(stage);
        shadergen.emitString("    " + sampleStrings[i], stage);
        if (i + 1 < sampleCount)
        {
            shadergen.emitLine(",", stage, false);
        }
    }
    shadergen.emitLine(")", stage);
}

} // namespace MaterialX
