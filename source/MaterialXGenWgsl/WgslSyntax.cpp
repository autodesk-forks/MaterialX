//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/WgslSyntax.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Since WGSL doesn't support strings we use integers instead.
class WgslStringTypeSyntax : public StringTypeSyntax
{
  public:
    WgslStringTypeSyntax(const Syntax* parent) :
        StringTypeSyntax(parent, "i32", "0", "0") { }

    string getValue(const Value& /*value*/, bool /*uniform*/) const override
    {
        return "0";
    }
};

class WgslArrayTypeSyntax : public ScalarTypeSyntax
{
  public:
    WgslArrayTypeSyntax(const Syntax* parent, const string& name) :
        ScalarTypeSyntax(parent, name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
    {
    }

    string getValue(const Value& value, bool uniform) const override
    {
        if (uniform)
        {
            return value.getValueString();
        }

        size_t arraySize = getSize(value);
        if (arraySize > 0)
        {
            return "array<" + getName() + ", " + std::to_string(arraySize) + ">(" + value.getValueString() + ")";
        }
        return EMPTY_STRING;
    }

  protected:
    virtual size_t getSize(const Value& value) const = 0;
};

class WgslFloatArrayTypeSyntax : public WgslArrayTypeSyntax
{
  public:
    explicit WgslFloatArrayTypeSyntax(const Syntax* parent, const string& name) :
        WgslArrayTypeSyntax(parent, name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<float> valueArray = value.asA<vector<float>>();
        return valueArray.size();
    }
};

class WgslIntegerArrayTypeSyntax : public WgslArrayTypeSyntax
{
  public:
    explicit WgslIntegerArrayTypeSyntax(const Syntax* parent, const string& name) :
        WgslArrayTypeSyntax(parent, name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<int> valueArray = value.asA<vector<int>>();
        return valueArray.size();
    }
};

class WgslAggregateTypeSyntax : public AggregateTypeSyntax
{
  public:
    WgslAggregateTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                            const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING,
                            const StringVec& members = EMPTY_MEMBERS) :
        AggregateTypeSyntax(parent, name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, members)
    {
    }

    string getValue(const Value& value, bool uniform) const override
    {
        const string valueString = value.getValueString();
        if (uniform)
        {
            return valueString;
        }

        return valueString.empty() ? valueString : getName() + "(" + valueString + ")";
    }
};

} // anonymous namespace

const string WgslSyntax::INPUT_QUALIFIER = "";
const string WgslSyntax::OUTPUT_QUALIFIER = ""; // WGSL doesn't use qualifiers like GLSL
const string WgslSyntax::UNIFORM_QUALIFIER = ""; // WGSL uses binding/group instead
const string WgslSyntax::CONSTANT_QUALIFIER = "const";
const string WgslSyntax::SOURCE_FILE_EXTENSION = ".wgsl";
const StringVec WgslSyntax::VEC2_MEMBERS = { ".x", ".y" };
const StringVec WgslSyntax::VEC3_MEMBERS = { ".x", ".y", ".z" };
const StringVec WgslSyntax::VEC4_MEMBERS = { ".x", ".y", ".z", ".w" };

//
// WgslSyntax methods
//

WgslSyntax::WgslSyntax(TypeSystemPtr typeSystem) :
    Syntax(typeSystem)
{
    // Add in all reserved words and keywords in WGSL
    registerReservedWords({ 
        // Keywords (https://www.w3.org/TR/WGSL/#keyword-summary)
        "alias", "break", "case", "const", "const_assert", "continue", "continuing",
        "default", "diagnostic", "discard", "else", "enable", "false", "fn",
        "for", "if", "let", "loop", "override", "requires", "return",
        "struct", "switch", "true", "var", "while",
        // Reserved Words (https://www.w3.org/TR/WGSL/#reserved-words)
        "NULL", "Self", "abstract", "active", "alignas", "alignof", "as", "asm",
        "asm_fragment", "async", "attribute", "auto", "await", "become", "cast",
        "catch", "class", "co_await", "co_return", "co_yield", "coherent",
        "column_major", "common", "compile", "compile_fragment", "concept",
        "const_cast", "consteval", "constexpr", "constinit", "crate", "debugger",
        "decltype", "delete", "demote", "demote_to_helper", "do", "dynamic_cast",
        "enum", "explicit", "export", "extends", "extern", "external", "fallthrough",
        "filter", "final", "finally", "friend", "from", "fxgroup", "get", "goto",
        "groupshared", "highp", "impl", "implements", "import", "inline", "instanceof",
        "interface", "layout", "lowp", "macro", "macro_rules", "match", "mediump",
        "meta", "mod", "module", "move", "mut", "mutable", "namespace", "new",
        "nil", "noexcept", "noinline", "nointerpolation", "non_coherent", "noncoherent",
        "noperspective", "null", "nullptr", "of", "operator", "package", "packoffset",
        "partition", "pass", "patch", "pixelfragment", "precise", "precision",
        "premerge", "priv", "protected", "pub", "public", "readonly", "ref",
        "regardless", "register", "reinterpret_cast", "require", "resource", "restrict",
        "self", "set", "shared", "sizeof", "smooth", "snorm", "static", "static_assert",
        "static_cast", "std", "subroutine", "super", "target", "template", "this",
        "thread_local", "throw", "trait", "try", "type", "typedef", "typeid",
        "typename", "typeof", "union", "unless", "unorm", "unsafe", "unsized",
        "use", "using", "varying", "virtual", "volatile", "wgsl", "where", "with",
        "writeonly", "yield"
    });

    //
    // Register syntax handlers for each data type.
    //

    registerTypeSyntax(
        Type::FLOAT,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "f32",
            "0.0",
            "0.0"));

    registerTypeSyntax(
        Type::FLOATARRAY,
        std::make_shared<WgslFloatArrayTypeSyntax>(
            this,
            "f32"));

    registerTypeSyntax(
        Type::INTEGER,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "i32",
            "0",
            "0"));

    registerTypeSyntax(
        Type::INTEGERARRAY,
        std::make_shared<WgslIntegerArrayTypeSyntax>(
            this,
            "i32"));

    registerTypeSyntax(
        Type::BOOLEAN,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "bool",
            "false",
            "false"));

    registerTypeSyntax(
        Type::COLOR3,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "vec3<f32>",
            "vec3<f32>(0.0)",
            "0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "vec4<f32>",
            "vec4<f32>(0.0)",
            "0.0, 0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "vec2<f32>",
            "vec2<f32>(0.0)",
            "0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC2_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR3,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "vec3<f32>",
            "vec3<f32>(0.0)",
            "0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "vec4<f32>",
            "vec4<f32>(0.0)",
            "0.0, 0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "mat3x3<f32>",
            "mat3x3<f32>(1.0,0.0,0.0,  0.0,1.0,0.0, 0.0,0.0,1.0)",
            "1.0,0.0,0.0,  0.0,1.0,0.0, 0.0,0.0,1.0"));

    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "mat4x4<f32>",
            "mat4x4<f32>(1.0,0.0,0.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,0.0,0.0,1.0)",
            "1.0,0.0,0.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,0.0,0.0,1.0"));

    registerTypeSyntax(
        Type::STRING,
        std::make_shared<WgslStringTypeSyntax>(this));

    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "texture_2d<f32>",
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "BSDF",
            "BSDF(vec3<f32>(0.0),vec3<f32>(1.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct BSDF { response: vec3<f32>, throughput: vec3<f32> }"));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "EDF",
            "EDF(0.0)",
            "0.0, 0.0, 0.0",
            "vec3<f32>",
            "alias EDF = vec3<f32>;"));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "BSDF",
            "BSDF(vec3<f32>(0.0),vec3<f32>(1.0))",
            EMPTY_STRING));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "surfaceshader",
            "surfaceshader(vec3<f32>(0.0),vec3<f32>(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct surfaceshader { color: vec3<f32>, transparency: vec3<f32> }"));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "volumeshader",
            "volumeshader(vec3<f32>(0.0),vec3<f32>(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct volumeshader { color: vec3<f32>, transparency: vec3<f32> }"));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "displacementshader",
            "displacementshader(vec3<f32>(0.0),1.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct displacementshader { offset: vec3<f32>, scale: f32 }"));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "lightshader",
            "lightshader(vec3<f32>(0.0),vec3<f32>(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct lightshader { intensity: vec3<f32>, direction: vec3<f32> }"));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "material",
            "material(vec3<f32>(0.0),vec3<f32>(0.0))",
            EMPTY_STRING,
            "surfaceshader",
            "alias material = surfaceshader;"));
}

bool WgslSyntax::typeSupported(const TypeDesc* type) const
{
    return *type != Type::STRING;
}

void WgslSyntax::makeValidName(string& name) const
{
    Syntax::makeValidName(name);
    if (std::isdigit(name[0]))
        name = "v_" + name;
}

bool WgslSyntax::remapEnumeration(const string& value, TypeDesc type, const string& enumNames, std::pair<TypeDesc, ValuePtr>& result) const
{
    // Early out if not an enum input.
    if (enumNames.empty())
    {
        return false;
    }

    // Don't convert already supported types
    if (type != Type::STRING)
    {
        return false;
    }

    // Early out if no valid value provided
    if (value.empty())
    {
        return false;
    }

    // For WGSL we always convert to integer,
    // with the integer value being an index into the enumeration.
    result.first = Type::INTEGER;
    result.second = nullptr;

    // Try remapping to an enum value.
    StringVec valueElemEnumsVec = splitString(enumNames, ",");
    for (size_t i = 0; i < valueElemEnumsVec.size(); i++)
    {
        valueElemEnumsVec[i] = trimSpaces(valueElemEnumsVec[i]);
    }
    auto pos = std::find(valueElemEnumsVec.begin(), valueElemEnumsVec.end(), value);
    if (pos == valueElemEnumsVec.end())
    {
        throw ExceptionShaderGenError("Given value '" + value + "' is not a valid enum value for input.");
    }
    const int index = static_cast<int>(std::distance(valueElemEnumsVec.begin(), pos));
    result.second = Value::createValue<int>(index);

    return true;
}

StructTypeSyntaxPtr WgslSyntax::createStructSyntax(const string& structTypeName, const string& defaultValue,
                                                    const string& uniformDefaultValue, const string& typeAlias,
                                                    const string& typeDefinition) const
{
    return std::make_shared<WgslStructTypeSyntax>(
        this,
        structTypeName,
        defaultValue,
        uniformDefaultValue,
        typeAlias,
        typeDefinition);
}

string WgslStructTypeSyntax::getValue(const Value& value, bool /* uniform */) const
{
    const AggregateValue& aggValue = static_cast<const AggregateValue&>(value);

    string result = aggValue.getTypeString() + "(";

    string separator = "";
    for (const auto& memberValue : aggValue.getMembers())
    {
        result += separator;
        separator = ",";

        const string& memberTypeName = memberValue->getTypeString();
        const TypeDesc memberTypeDesc = _parent->getType(memberTypeName);

        // Recursively use the syntax to generate the output, so we can supported nested structs.
        result += _parent->getValue(memberTypeDesc, *memberValue, true);
    }

    result += ")";

    return result;
}

MATERIALX_NAMESPACE_END

