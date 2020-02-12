//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXGenMdl/MdlSyntax.h>
#include <MaterialXGenMdl/Nodes/CompoundNodeMdl.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/Nodes/ConvertNode.h>
#include <MaterialXGenShader/Nodes/CombineNode.h>
#include <MaterialXGenShader/Nodes/SwitchNode.h>
#include <MaterialXGenShader/Nodes/IfNode.h>
#include <MaterialXGenShader/Nodes/BlurNode.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

namespace
{
    std::unordered_map<string, string> GEOMPROP_DEFINITIONS =
    {
        {"Pobject", "base::transform_point(base::coordinate_internal, base::coordinate_object, state::position())"},
        {"Pworld", "base::transform_point(base::coordinate_internal, base::coordinate_world, state::position())"},
        {"Nobject", "base::transform_normal(base::coordinate_internal, base::coordinate_object, state::normal())"},
        {"Nworld", "base::transform_normal(base::coordinate_internal, base::coordinate_world, state::normal())"},
        {"Tobject", "base::transform_vector(base::coordinate_internal, base::coordinate_object, state::texture_tangent_u(0))"},
        {"Tworld", "base::transform_vector(base::coordinate_internal, base::coordinate_world, state::texture_tangent_u(0))"},
        {"Bobject", "base::transform_vector(base::coordinate_internal, base::coordinate_object, state::texture_tangent_v(0))"},
        {"Bworld", "base::transform_vector(base::coordinate_internal, base::coordinate_world, state::texture_tangent_v(0))"},
        {"UV0", "float2(state::texture_coordinate(0).x, state::texture_coordinate(0).y)"},
        {"Vworld", "state::direction()"}
    };

    const string MDL_VERSION = "1.6";

    const vector<string> DEFAULT_IMPORTS = {
        "import ::df::*",
        "import ::base::*",
        "import ::math::*",
        "import ::state::*",
        "import ::anno::*",
        "import ::tex::*",
        "import ::nvidia::core_definitions::*",
        "import ::mx::stdlib::*",
        "import ::mx::swizzle::*",
        "import ::mx::supplib::*",
        "import ::mx::pbrlib::*"
    };
}

const string MdlShaderGenerator::LANGUAGE = "genmdl";
const string MdlShaderGenerator::TARGET = "mdl";

//
// MdlShaderGenerator methods
//

MdlShaderGenerator::MdlShaderGenerator() :
    ShaderGenerator(MdlSyntax::create())
{
    // Register build-in implementations

    // <!-- <if*> -->
    static const string SEPARATOR = "_";
    static const string INT_SEPARATOR = "I_";
    static const string BOOL_SEPARATOR = "B_";
    static const StringVec IMPL_PREFIXES = { "IM_ifgreater_", "IM_ifgreatereq_", "IM_ifequal_" };
    static const vector<CreatorFunction<ShaderNodeImpl>> IMPL_CREATE_FUNCTIONS =
    { IfGreaterNode::create,  IfGreaterEqNode::create, IfEqualNode::create };
    static const vector<bool> IMPL_HAS_INTVERSION = { true, true, true };
    static const vector<bool> IMPL_HAS_BOOLVERSION = { false, false, true };
    static const StringVec IMPL_TYPES = { "float", "color2", "color3", "color4", "vector2", "vector3", "vector4" };
    for (size_t i = 0; i < IMPL_PREFIXES.size(); i++)
    {
        const string& implPrefix = IMPL_PREFIXES[i];
        for (const string& implType : IMPL_TYPES)
        {
            const string implRoot = implPrefix + implType;
            registerImplementation(implRoot + SEPARATOR + MdlShaderGenerator::LANGUAGE, IMPL_CREATE_FUNCTIONS[i]);
            if (IMPL_HAS_INTVERSION[i])
            {
                registerImplementation(implRoot + INT_SEPARATOR + MdlShaderGenerator::LANGUAGE, IMPL_CREATE_FUNCTIONS[i]);
            }
            if (IMPL_HAS_BOOLVERSION[i])
            {
                registerImplementation(implRoot + BOOL_SEPARATOR + MdlShaderGenerator::LANGUAGE, IMPL_CREATE_FUNCTIONS[i]);
            }
        }
    }

    // <!-- <switch> -->
    // <!-- 'which' type : float -->
    registerImplementation("IM_switch_float_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color2_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color3_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color4_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector2_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector3_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector4_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    // <!-- 'which' type : integer -->
    registerImplementation("IM_switch_floatI_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color2I_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color3I_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color4I_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector2I_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector3I_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector4I_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    // <!-- 'which' type : boolean -->
    registerImplementation("IM_switch_floatB_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color2B_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color3B_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color4B_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector2B_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector3B_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector4B_" + MdlShaderGenerator::LANGUAGE, SwitchNode::create);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle_float_color2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_color3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_color4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : color2 -->
    registerImplementation("IM_swizzle_color2_float_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle_color3_float_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle_color4_float_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle_vector2_float_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle_vector3_float_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle_vector4_float_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector2_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector3_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector4_" + MdlShaderGenerator::LANGUAGE, SwizzleNode::create);

    // <!-- <convert> -->
    registerImplementation("IM_convert_float_color2_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_color3_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_color4_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_vector2_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_vector3_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_vector4_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector2_color2_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector2_vector3_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector2_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector3_color3_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector4_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector4_vector3_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector4_color4_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color2_vector2_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color3_vector3_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color4_vector4_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color3_color4_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color4_color3_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_boolean_float_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_integer_float_" + MdlShaderGenerator::LANGUAGE, ConvertNode::create);

    // <!-- <combine> -->
    registerImplementation("IM_combine2_color2_" + MdlShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine2_vector2_" + MdlShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine2_color4CF_" + MdlShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine2_vector4VF_" + MdlShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine2_color4CC_" + MdlShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine2_vector4VV_" + MdlShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine3_color3_" + MdlShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine3_vector3_" + MdlShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine4_color4_" + MdlShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine4_vector4_" + MdlShaderGenerator::LANGUAGE, CombineNode::create);

    // <!-- <blur> -->
    registerImplementation("IM_blur_float_" + MdlShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_color2_" + MdlShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_color3_" + MdlShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_color4_" + MdlShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_vector2_" + MdlShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_vector3_" + MdlShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_vector4_" + MdlShaderGenerator::LANGUAGE, BlurNode::create);
}

ShaderPtr MdlShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);

    ShaderGraph& graph = shader->getGraph();
    ShaderStage& stage = shader->getStage(Stage::PIXEL);

    // Emit version
    emitLine("mdl " + MDL_VERSION, stage);
    emitLineBreak(stage);

    emitLine("using mx = ::materialx", stage);

    // Emit module imports
    for (const string& module : DEFAULT_IMPORTS)
    {
        emitLine(module, stage);
    }
    if (graph.hasClassification(ShaderNode::Classification::CONVOLUTION2D))
    {
        // TODO: create a package with the convolution functions
        // emitLine("import materialx::convolution::*", stage);
    }

    // Add global constants and type definitions
    emitTypeDefinitions(context, stage);

    // TODO: Fix this!
    //
    // Set the include file to use for uv transformations,
    // depending on the vertical flip flag.
    /*
    if (context.getOptions().fileTextureVerticalFlip)
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "stdlib/" + OslShaderGenerator::LANGUAGE + "/lib/mx_transform_uv_vflip.osl";
    }
    else
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "stdlib/" + OslShaderGenerator::LANGUAGE + "/lib/mx_transform_uv.osl";
    }
    */

    // Emit function definitions for all nodes
    emitFunctionDefinitions(graph, context, stage);

    // Emit shader type, determined from the first
    // output if there are multiple outputs.
    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket(0);
    emitString("export material ", stage);

    // Begin shader signature. Note that makeIdentifier() will sanitize the name.
    string functionName = shader->getName();
    _syntax->makeIdentifier(functionName, graph.getIdentifierMap());
    setFunctionName(functionName, stage);
    emitLine(functionName, stage, false);
    emitScopeBegin(stage, Syntax::PARENTHESES);

    // Emit shader inputs
    emitShaderInputs(stage.getInputBlock(MDL::INPUTS), stage);

    // End shader signature
    emitScopeEnd(stage);

    // Begin shader body
    emitLine("= let", stage, false);
    emitScopeBegin(stage);

    // Emit constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (constants.size())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), SEMICOLON, context, stage);
        emitLineBreak(stage);
    }

    // Emit function calls for all nodes
    emitFunctionCalls(graph, context, stage);

    // Get final result
    const string result = getUpstreamResult(outputSocket, context);

    if (graph.hasClassification(ShaderNode::Classification::TEXTURE))
    {
        emitLine("color finalOutput__ = color(" + result + ")", stage);

        // End shader body
        emitScopeEnd(stage);

        static const string textureMaterial =
            "in material\n"
            "(\n"
            "    surface: material_surface(\n"
            "        emission : material_emission(\n"
            "            emission : df::diffuse_edf(),\n"
            "            intensity : finalOutput__,\n"
            "            mode : intensity_radiant_exitance\n"
            "        )\n"
            "    )\n"
            ");";
        emitBlock(textureMaterial, context, stage);
    }
    else if (graph.hasClassification(ShaderNode::Classification::SHADER))
    {
        emitLine(_syntax->getTypeSyntax(outputSocket->getType()).getName() +  " finalOutput__ = " + result, stage);

        // End shader body
        emitScopeEnd(stage);

        static const string shaderMaterial = "in material(finalOutput__);";
        emitBlock(shaderMaterial, context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Output type '" + outputSocket->getType()->getName() + "' is not yet supported by shader generator");
    }

    // Perform token substitution
    replaceTokens(_tokenSubstitutions, stage);

    return shader;
}

ShaderPtr MdlShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    // Create the root shader graph
    ShaderGraphPtr graph = ShaderGraph::create(nullptr, name, element, context);
    ShaderPtr shader = std::make_shared<Shader>(name, graph);

    // Create our stage.
    ShaderStagePtr stage = createStage(Stage::PIXEL, *shader);
    VariableBlockPtr inputs = stage->createInputBlock(MDL::INPUTS);
    VariableBlockPtr outputs = stage->createOutputBlock(MDL::OUTPUTS);

    // Create shader variables for all nodes that need this.
    for (ShaderNode* node : graph->getNodes())
    {
        node->getImplementation().createVariables(*node, context, *shader);
    }

    // Create inputs for the published graph interface.
    for (ShaderGraphInputSocket* inputSocket : graph->getInputSockets())
    {
        // Only for inputs that are connected/used internally,
        // and are editable by users.
        if (inputSocket->getConnections().size() && graph->isEditable(*inputSocket))
        {
            inputs->add(inputSocket->getSelf());
        }
    }

    // Create outputs from the graph interface.
    for (ShaderGraphOutputSocket* outputSocket : graph->getOutputSockets())
    {
        outputs->add(outputSocket->getSelf());
    }

    return shader;
}

void MdlShaderGenerator::emitShaderInputs(const VariableBlock& inputs, ShaderStage& stage) const
{
    const string uniformPrefix = _syntax->getUniformQualifier() + " ";
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        const ShaderPort* input = inputs[i];

        const string& qualifier = input->isUniform() ? uniformPrefix : EMPTY_STRING;
        const string& type = _syntax->getTypeName(input->getType());
        const string value = (input->getValue() ?
            _syntax->getValue(input->getType(), *input->getValue(), true) :
            _syntax->getDefaultValue(input->getType(), true));

        emitLineBegin(stage);

        const string& geomprop = input->getGeomProp();
        if (!geomprop.empty())
        {
            auto it = GEOMPROP_DEFINITIONS.find(geomprop);
            const string& v = it != GEOMPROP_DEFINITIONS.end() ? it->second : value;
            emitString(type + " " + input->getVariable() + " = " + v, stage);
        }
        else
        {
            emitString(qualifier + type + " " + input->getVariable() + " = " + value, stage);
        }

        if (i < inputs.size() - 1)
        {
            emitString(",", stage);
        }

        emitLineEnd(stage, false);
    }
}

ShaderNodeImplPtr MdlShaderGenerator::createCompoundImplementation(const NodeGraph&) const
{
    // The standard compound implementation
    // is the compound implementation to us by default
    return CompoundNodeMdl::create();
}

namespace MDL
{
    // Identifiers for MDL variable blocks
    const string INPUTS   = "i";
    const string OUTPUTS  = "o";
}

bool MdlShaderGenerator::remapEnumeration(const ValueElement& input, const string& value, std::pair<const TypeDesc*, ValuePtr>& result) const
{
    // Early out if not an enum input.
    const string& enumNames = input.getAttribute(ValueElement::ENUM_ATTRIBUTE);
    if (enumNames.empty())
    {
        return false;
    }

    // Don't convert or filenames and arrays.
    const TypeDesc* type = TypeDesc::get(input.getType());
    if (type == Type::FILENAME || type->isArray())
    {
        return false;
    }

    // Try remapping to an enum value.
    if (!value.empty())
    {
        MdlSyntaxPtr mdlSyntax = std::dynamic_pointer_cast<MdlSyntax>(_syntax);
        result.first = mdlSyntax->getEnumeratedType(value);
        if (!result.first || (result.first->getSemantic() != TypeDesc::Semantic::SEMANTIC_ENUM))
        {
            return false;
        }

        StringVec valueElemEnumsVec = splitString(enumNames, ",");
        auto pos = std::find(valueElemEnumsVec.begin(), valueElemEnumsVec.end(), value);
        if (pos == valueElemEnumsVec.end())
        {
            throw ExceptionShaderGenError("Given value '" + value + "' is not a valid enum value for input '" + input.getNamePath() + "'");
        }
        const int index = static_cast<int>(std::distance(valueElemEnumsVec.begin(), pos));
        result.second = Value::createValue<string>(valueElemEnumsVec[index]);

        return true;
    }

    return false;
}

} // namespace MaterialX
