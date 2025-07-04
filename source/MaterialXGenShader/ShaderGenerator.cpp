//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/ShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXFormat/File.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Value.h>

#include <sstream>

MATERIALX_NAMESPACE_BEGIN

const string ShaderGenerator::T_FILE_TRANSFORM_UV = "$fileTransformUv";
const string ShaderGenerator::LIGHTDATA_TYPEVAR_STRING = "type";

//
// ShaderGenerator methods
//

ShaderGenerator::ShaderGenerator(TypeSystemPtr typeSystem, SyntaxPtr syntax) :
    _typeSystem(typeSystem),
    _syntax(syntax)
{
}

void ShaderGenerator::emitScopeBegin(ShaderStage& stage, Syntax::Punctuation punc) const
{
    stage.beginScope(punc);
}

void ShaderGenerator::emitScopeEnd(ShaderStage& stage, bool semicolon, bool newline) const
{
    stage.endScope(semicolon, newline);
}

void ShaderGenerator::emitLineBegin(ShaderStage& stage) const
{
    stage.beginLine();
}

void ShaderGenerator::emitLineEnd(ShaderStage& stage, bool semicolon) const
{
    stage.endLine(semicolon);
}

void ShaderGenerator::emitLineBreak(ShaderStage& stage) const
{
    stage.newLine();
}

void ShaderGenerator::emitString(const string& str, ShaderStage& stage) const
{
    stage.addString(str);
}

void ShaderGenerator::emitLine(const string& str, ShaderStage& stage, bool semicolon) const
{
    stage.addLine(str, semicolon);
}

void ShaderGenerator::emitComment(const string& str, ShaderStage& stage) const
{
    stage.addComment(str);
}

void ShaderGenerator::emitBlock(const string& str, const FilePath& sourceFilename, GenContext& context, ShaderStage& stage) const
{
    stage.addBlock(str, sourceFilename, context);
}

void ShaderGenerator::emitLibraryInclude(const FilePath& filename, GenContext& context, ShaderStage& stage) const
{
    FilePath libraryPrefix = context.getOptions().libraryPrefix;
    FilePath fullFilename = libraryPrefix.isEmpty() ? filename : libraryPrefix / filename;
    FilePath resolvedFilename = context.resolveSourceFile(fullFilename, FilePath());
    stage.addInclude(fullFilename, resolvedFilename, context);
}

void ShaderGenerator::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    stage.addFunctionDefinition(node, context);
}

void ShaderGenerator::emitFunctionDefinitionParameter(const ShaderPort* shaderPort, bool isOutput, GenContext&, ShaderStage& stage) const
{
    if (isOutput)
    {
        emitString(_syntax->getOutputTypeName(shaderPort->getType()) + " " + shaderPort->getVariable(), stage);
    }
    else 
    {
        emitString(_syntax->getTypeName(shaderPort->getType()) + " " + shaderPort->getVariable(), stage);
    }
}

void ShaderGenerator::emitFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    // Emit function definitions for all nodes in the graph.
    for (ShaderNode* node : graph.getNodes())
    {
        emitFunctionDefinition(*node, context, stage);
    }
}

void ShaderGenerator::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    // Check if it's emitted already.
    if (!stage.isEmitted(node, context))
    {
        stage.addFunctionCall(node, context);
    }
}

// Wrapper for deprecated version of this method.
void ShaderGenerator::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage, bool /*checkScope*/) const
{
    emitFunctionCall(node, context, stage);
}

void ShaderGenerator::emitFunctionCalls(const ShaderGraph& graph, GenContext& context, ShaderStage& stage, uint32_t classification) const
{
    for (ShaderNode* node : graph.getNodes())
    {
        if (!classification || node->hasClassification(classification))
        {
            emitFunctionCall(*node, context, stage);
        }
    }
}

void ShaderGenerator::emitDependentFunctionCalls(const ShaderNode& node, GenContext& context, ShaderStage& stage, uint32_t classification) const
{
    for (ShaderInput* input : node.getInputs())
    {
        const ShaderNode* upstream = input->getConnectedSibling();
        if (upstream && (!classification || upstream->hasClassification(classification)))
        {
            emitFunctionCall(*upstream, context, stage);
        }
    }
}

void ShaderGenerator::emitFunctionBodyBegin(const ShaderNode&, GenContext&, ShaderStage& stage, Syntax::Punctuation punc) const
{
    emitScopeBegin(stage, punc);
}

void ShaderGenerator::emitFunctionBodyEnd(const ShaderNode&, GenContext&, ShaderStage& stage) const
{
    emitScopeEnd(stage);
    emitLineBreak(stage);
}

void ShaderGenerator::emitTypeDefinitions(GenContext&, ShaderStage& stage) const
{
    // Emit typedef statements for all data types that have an alias
    for (const auto& syntax : _syntax->getTypeSyntaxes())
    {
        if (!syntax->getTypeDefinition().empty())
        {
            stage.addLine(syntax->getTypeDefinition(), false);
        }
    }
    stage.newLine();
}

void ShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                              GenContext&, ShaderStage& stage,
                                              bool assignValue) const
{
    string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
    str += _syntax->getTypeName(variable->getType());

    bool haveArray = variable->getType().isArray() && variable->getValue();
    if (haveArray)
    {
        str += _syntax->getArrayTypeSuffix(variable->getType(), *variable->getValue());
    }

    str += " " + variable->getVariable();

    // If an array we need an array qualifier (suffix) for the variable name
    if (haveArray)
    {
        str += _syntax->getArrayVariableSuffix(variable->getType(), *variable->getValue());
    }

    if (assignValue)
    {
        const string valueStr = (variable->getValue() ?
                                 _syntax->getValue(variable->getType(), *variable->getValue(), true) :
                                 _syntax->getDefaultValue(variable->getType(), true));
        str += valueStr.empty() ? EMPTY_STRING : " = " + valueStr;
    }

    stage.addString(str);
}

void ShaderGenerator::emitVariableDeclarations(const VariableBlock& block, const string& qualifier, const string& separator,
                                               GenContext& context, ShaderStage& stage,
                                               bool assignValue) const
{
    for (size_t i = 0; i < block.size(); ++i)
    {
        emitLineBegin(stage);
        emitVariableDeclaration(block[i], qualifier, context, stage, assignValue);
        emitString(separator, stage);
        emitLineEnd(stage, false);
    }
}

void ShaderGenerator::emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const
{
    stage.addString(getUpstreamResult(input, context));
}

void ShaderGenerator::emitOutput(const ShaderOutput* output, bool includeType, bool assignValue, GenContext& context, ShaderStage& stage) const
{
    stage.addString(includeType ? _syntax->getTypeName(output->getType()) + " " + output->getVariable() : output->getVariable());

    // Look for any additional suffix to append
    string suffix;
    context.getOutputSuffix(output, suffix);
    if (!suffix.empty())
    {
        stage.addString(suffix);
    }

    if (assignValue)
    {
        const string& value = _syntax->getDefaultValue(output->getType());
        if (!value.empty())
        {
            stage.addString(" = " + value);
        }
    }
}

string ShaderGenerator::getUpstreamResult(const ShaderInput* input, GenContext& context) const
{
    if (!input->getConnection())
    {
        return input->getValue() ? _syntax->getValue(input->getType(), *input->getValue()) : _syntax->getDefaultValue(input->getType());
    }

    string variable = input->getConnection()->getVariable();

    // Look for any additional suffix to append
    string suffix;
    context.getInputSuffix(input, suffix);
    if (!suffix.empty())
    {
        variable += suffix;
    }

    return variable;
}

void ShaderGenerator::registerImplementation(const string& name, CreatorFunction<ShaderNodeImpl> creator)
{
    _implFactory.registerClass(name, creator);
}

void ShaderGenerator::registerImplementation(const StringVec& nameVec, CreatorFunction<ShaderNodeImpl> creator)
{
    for (const string& name : nameVec)
    {
        _implFactory.registerClass(name, creator);
    }
}

bool ShaderGenerator::implementationRegistered(const string& name) const
{
    return _implFactory.classRegistered(name);
}

ShaderNodeImplPtr ShaderGenerator::getImplementation(const NodeDef& nodedef, GenContext& context) const
{
    InterfaceElementPtr implElement = nodedef.getImplementation(getTarget());
    if (!implElement)
    {
        return nullptr;
    }

    const string& name = implElement->getName();

    // Check if it's created and cached already.
    ShaderNodeImplPtr impl = context.findNodeImplementation(name);
    if (impl)
    {
        return impl;
    }

    if (implElement->isA<NodeGraph>())
    {
        impl = CompoundNode::create();
    }
    else if (implElement->isA<Implementation>())
    {
        if (getColorManagementSystem() && getColorManagementSystem()->hasImplementation(name))
        {
            impl = getColorManagementSystem()->createImplementation(name);
        }
        else
        {
            // Try creating a new in the factory.
            impl = _implFactory.create(name);
        }
        if (!impl)
        {
            impl = SourceCodeNode::create();
        }
    }
    if (!impl)
    {
        return nullptr;
    }

    impl->initialize(*implElement, context);

    // Cache it.
    context.addNodeImplementation(name, impl);

    return impl;
}

void ShaderGenerator::registerTypeDefs(const DocumentPtr& doc)
{
    /// Load any struct type definitions from the document.
    for (const auto& mxTypeDef : doc->getTypeDefs())
    {
        const string& typeName = mxTypeDef->getName();
        const auto& members = mxTypeDef->getMembers();

        // If we don't have any member children then we're not going to consider ourselves a struct.
        if (members.empty())
        {
            continue;
        }

        auto structMembers = std::make_shared<StructMemberDescVec>();
        for (const auto& member : members)
        {
            const auto memberType = _typeSystem->getType(member->getType());
            const auto memberName = member->getName();
            const auto memberDefaultValue = member->getValueString();
            structMembers->emplace_back(StructMemberDesc(memberType, memberName, memberDefaultValue));
        }

        _typeSystem->registerType(typeName, TypeDesc::BASETYPE_STRUCT, TypeDesc::SEMANTIC_NONE, 1, structMembers);
    }

    // Create a type syntax for all struct types loaded above.
    for (TypeDesc typeDesc : _typeSystem->getTypes())
    {
        if (!typeDesc.isStruct())
        {
            continue;
        }

        const string& structTypeName = typeDesc.getName();
        string defaultValue = structTypeName + "( ";
        string uniformDefaultValue = EMPTY_STRING;
        string typeAlias = EMPTY_STRING;
        string typeDefinition = "struct " + structTypeName + " { ";

        auto structMembers = typeDesc.getStructMembers();
        if (structMembers)
        {
            for (const auto& structMember : *structMembers)
            {
                const string& memberType = structMember.getType().getName();
                const string& memberName = structMember.getName();
                const string& memberDefaultValue = structMember.getDefaultValueStr();

                defaultValue += memberDefaultValue + ", ";
                typeDefinition += memberType + " " + memberName + "; ";
            }
        }

        typeDefinition += " };";
        defaultValue += " )";

        StructTypeSyntaxPtr structTypeSyntax = _syntax->createStructSyntax(
            structTypeName,
            defaultValue,
            uniformDefaultValue,
            typeAlias,
            typeDefinition);

        _syntax->registerTypeSyntax(typeDesc, structTypeSyntax);
    }
}

namespace
{

void replace(const StringMap& substitutions, ShaderPort* port)
{
    string name = port->getName();
    tokenSubstitution(substitutions, name);
    port->setName(name);
    string variable = port->getVariable();
    tokenSubstitution(substitutions, variable);
    port->setVariable(variable);
}

} // anonymous namespace

void ShaderGenerator::registerShaderMetadata(const DocumentPtr& doc, GenContext& context) const
{
    ShaderMetadataRegistryPtr registry = context.getUserData<ShaderMetadataRegistry>(ShaderMetadataRegistry::USER_DATA_NAME);
    if (!registry)
    {
        registry = std::make_shared<ShaderMetadataRegistry>();
        context.pushUserData(ShaderMetadataRegistry::USER_DATA_NAME, registry);
    }

    // Add default entries.
    const ShaderMetadata DEFAULT_METADATA[] =
    {
        ShaderMetadata(ValueElement::UI_NAME_ATTRIBUTE, Type::STRING),
        ShaderMetadata(ValueElement::UI_FOLDER_ATTRIBUTE, Type::STRING),
        ShaderMetadata(ValueElement::UI_MIN_ATTRIBUTE, Type::NONE),
        ShaderMetadata(ValueElement::UI_MAX_ATTRIBUTE, Type::NONE),
        ShaderMetadata(ValueElement::UI_SOFT_MIN_ATTRIBUTE, Type::NONE),
        ShaderMetadata(ValueElement::UI_SOFT_MAX_ATTRIBUTE, Type::NONE),
        ShaderMetadata(ValueElement::UI_STEP_ATTRIBUTE, Type::NONE),
        ShaderMetadata(ValueElement::UI_ADVANCED_ATTRIBUTE, Type::BOOLEAN),
        ShaderMetadata(ValueElement::DOC_ATTRIBUTE, Type::STRING),
        ShaderMetadata(ValueElement::UNIT_ATTRIBUTE, Type::STRING),
        ShaderMetadata(ValueElement::COLOR_SPACE_ATTRIBUTE, Type::STRING)
    };
    for (const ShaderMetadata& data : DEFAULT_METADATA)
    {
        registry->addMetadata(data.name, data.type);
    }

    // Add entries from AttributeDefs in the document.
    vector<AttributeDefPtr> attributeDefs = doc->getAttributeDefs();
    for (const AttributeDefPtr& def : attributeDefs)
    {
        if (def->getExportable())
        {
            const string& attrName = def->getAttrName();
            const TypeDesc type = _typeSystem->getType(def->getType());
            if (!attrName.empty() && type != Type::NONE)
            {
                registry->addMetadata(attrName, type, def->getValue());
            }
        }
    }
}

void ShaderGenerator::replaceTokens(const StringMap& substitutions, ShaderStage& stage) const
{
    // Replace tokens in source code
    tokenSubstitution(substitutions, stage._code);

    // Replace tokens on shader interface
    for (size_t i = 0; i < stage._constants.size(); ++i)
    {
        replace(substitutions, stage._constants[i]);
    }
    for (const auto& it : stage._uniforms)
    {
        VariableBlock& uniforms = *it.second;
        for (size_t i = 0; i < uniforms.size(); ++i)
        {
            replace(substitutions, uniforms[i]);
        }
    }
    for (const auto& it : stage._inputs)
    {
        VariableBlock& inputs = *it.second;
        for (size_t i = 0; i < inputs.size(); ++i)
        {
            replace(substitutions, inputs[i]);
        }
    }
    for (const auto& it : stage._outputs)
    {
        VariableBlock& outputs = *it.second;
        for (size_t i = 0; i < outputs.size(); ++i)
        {
            replace(substitutions, outputs[i]);
        }
    }
}

ShaderStagePtr ShaderGenerator::createStage(const string& name, Shader& shader) const
{
    return shader.createStage(name, _syntax);
}

void ShaderGenerator::createVariables(ShaderGraphPtr graph, GenContext& context, Shader& shader) const
{
    ApplicationVariableHandler handler = context.getApplicationVariableHandler();
    for (ShaderNode* node : graph->getNodes())
    {
        if (handler)
        {
            handler(node, context);
        }
        node->getImplementation().createVariables(*node, context, shader);
    }
}

MATERIALX_NAMESPACE_END
