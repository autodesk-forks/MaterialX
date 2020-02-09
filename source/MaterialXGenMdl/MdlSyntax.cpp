//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/MdlSyntax.h>

#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/TypeDesc.h>

#include <sstream>

namespace MaterialX
{

namespace
{

class MdlFilenameTypeSyntax : public ScalarTypeSyntax
{
public:
    MdlFilenameTypeSyntax() :
        ScalarTypeSyntax("texture_2d", EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
    {}

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        return getName() + "(\"" + value.getValueString() + "\")";
    }
};

class MdlArrayTypeSyntax : public ScalarTypeSyntax
{
  public:
    MdlArrayTypeSyntax(const string& name) :
        ScalarTypeSyntax(name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
    {}

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        if (!isEmpty(value))
        {
            return getName() + "[](" + value.getValueString() + ")";
        }
        return EMPTY_STRING;
    }

    string getValue(const StringVec& values, bool /*uniform*/) const override
    {
        if (values.empty())
        {
            throw ExceptionShaderGenError("No values given to construct an array value");
        }

        string result = getName() + "[](" + values[0];
        for (size_t i = 1; i<values.size(); ++i)
        {
            result += ", " + values[i];
        }
        result += ")";

        return result;
    }

  protected:
    virtual bool isEmpty(const Value& value) const = 0;
};

class MdlFloatArrayTypeSyntax : public MdlArrayTypeSyntax
{
  public:
    explicit MdlFloatArrayTypeSyntax(const string& name) :
        MdlArrayTypeSyntax(name)
    {}

  protected:
    bool isEmpty(const Value& value) const override
    {
        const vector<float>& valueArray = value.asA<vector<float>>();
        return valueArray.empty();
    }
};

class MdlIntegerArrayTypeSyntax : public MdlArrayTypeSyntax
{
  public:
    explicit MdlIntegerArrayTypeSyntax(const string& name) :
        MdlArrayTypeSyntax(name)
    {}

  protected:
    bool isEmpty(const Value& value) const override
    {
        const vector<int>& valueArray = value.asA<vector<int>>();
        return valueArray.empty();
    }
};

// For the color4 type we need even more specialization since it's a struct of a struct:
//
// struct color4 {
//    color rgb;
//    float a;
// }
//
class MdlColor4TypeSyntax : public AggregateTypeSyntax
{
public:
    MdlColor4TypeSyntax() :
        AggregateTypeSyntax("color4", "mk_color4(0.0)", "mk_color4(0.0)", EMPTY_STRING, EMPTY_STRING, MdlSyntax::COLOR4_MEMBERS)
    {}

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        StringStream ss;

        // Set float format and precision for the stream
        const Value::FloatFormat fmt = Value::getFloatFormat();
        ss.setf(std::ios_base::fmtflags(
            (fmt == Value::FloatFormatFixed ? std::ios_base::fixed :
            (fmt == Value::FloatFormatScientific ? std::ios_base::scientific : 0))),
            std::ios_base::floatfield);
        ss.precision(Value::getFloatPrecision());

        const Color4 c = value.asA<Color4>();
        ss << "mk_color4(" << c[0] << ", " << c[1] << ", " << c[2] << ", " << c[3] << ")";

        return ss.str();
    }

    string getValue(const StringVec& values, bool /*uniform*/) const override
    {
        if (values.size() < 4)
        {
            throw ExceptionShaderGenError("Too few values given to construct a color4 value");
        }
        return "mk_color4(" + values[0] + ", " + values[1] + ", " + values[2] + ", " + values[3] + ")";
    }
};


} // anonymous namespace

const string MdlSyntax::CONST_QUALIFIER = "";
const string MdlSyntax::UNIFORM_QUALIFIER = "uniform";
const StringVec MdlSyntax::VECTOR2_MEMBERS = { ".x", ".y" };
const StringVec MdlSyntax::VECTOR3_MEMBERS = { ".x", ".y", ".z" };
const StringVec MdlSyntax::VECTOR4_MEMBERS = { ".x", ".y", ".z", ".w" };
const StringVec MdlSyntax::COLOR2_MEMBERS = { ".x", ".y" };
const StringVec MdlSyntax::COLOR3_MEMBERS = { "[0]", "[1]", "[2]" };
const StringVec MdlSyntax::COLOR4_MEMBERS = { ".rgb[0]", ".rgb[1]", ".rgb[2]", ".a" };

//
// MdlSyntax methods
//

MdlSyntax::MdlSyntax()
{
    // Add in all reserved words and keywords in MDL
    registerReservedWords(
    {
        // Reserved words
        "annotation", "bool", "bool2", "bool3", "bool4", "break", "bsdf", "bsdf_measurement", "case", "cast", "color", "const",
        "continue", "default", "do", "double", "double2", "double2x2", "double2x3", "double3", "double3x2", "double3x3", "double3x4",
        "double4", "double4x3", "double4x4", "double4x2", "double2x4", "edf", "else", "enum", "export", "false", "float", "float2",
        "float2x2", "float2x3", "float3", "float3x2", "float3x3", "float3x4", "float4", "float4x3", "float4x4", "float4x2", "float2x4",
        "for", "hair_bsdf", "if", "import", "in", "int", "int2", "int3", "int4", "intensity_mode", "intensity_power", "intensity_radiant_exitance",
        "let", "light_profile", "material", "material_emission", "material_geometry", "material_surface", "material_volume", "mdl", "module",
        "package", "return", "string", "struct", "switch", "texture_2d", "texture_3d", "texture_cube", "texture_ptex", "true", "typedef", "uniform",
        "using", "varying", "vdf", "while", 
        // Reserved for future use
        "auto", "catch", "char", "class", "const_cast", "delete", "dynamic_cast", "explicit", "extern", "external", "foreach", "friend", "goto",
        "graph", "half", "half2", "half2x2", "half2x3", "half3", "half3x2", "half3x3", "half3x4", "half4", "half4x3", "half4x4", "half4x2", "half2x4",
        "inline", "inout", "lambda", "long", "mutable", "namespace", "native", "new", "operator", "out", "phenomenon", "private", "protected", "public",
        "reinterpret_cast", "sampler", "shader", "short", "signed", "sizeof", "static", "static_cast", "technique", "template", "this", "throw", "try",
        "typeid", "typename", "union", "unsigned", "virtual", "void", "volatile", "wchar_t"
    });

    //
    // Register type syntax handlers for each data type.
    //

    registerTypeSyntax
    (
        Type::FLOAT,
        std::make_shared<ScalarTypeSyntax>(
            "float",
            "0.0",
            "0.0")
    );

    registerTypeSyntax
    (
        Type::FLOATARRAY,
        std::make_shared<MdlFloatArrayTypeSyntax>(
            "float")
    );

    registerTypeSyntax
    (
        Type::INTEGER,
        std::make_shared<ScalarTypeSyntax>(
            "int",
            "0",
            "0")
    );

    registerTypeSyntax
    (
        Type::INTEGERARRAY,
        std::make_shared<MdlIntegerArrayTypeSyntax>(
            "int")
    );

    registerTypeSyntax
    (
        Type::BOOLEAN,
        std::make_shared<ScalarTypeSyntax>(
            "bool",
            "false",
            "false")
    );

    registerTypeSyntax
    (
        Type::COLOR2,
        std::make_shared<AggregateTypeSyntax>(
            "float2",
            "float2(0.0)",
            "float2(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            COLOR2_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::COLOR3,
        std::make_shared<AggregateTypeSyntax>(
            "color",
            "color(0.0)",
            "color(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            COLOR3_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::COLOR4,
        std::make_shared<MdlColor4TypeSyntax>()
    );

    registerTypeSyntax
    (
        Type::VECTOR2,
        std::make_shared<AggregateTypeSyntax>(
            "float2",
            "float2(0.0)",
            "float2(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VECTOR2_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::VECTOR3,
        std::make_shared<AggregateTypeSyntax>(
            "float3",
            "float3(0.0)",
            "float3(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VECTOR3_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::VECTOR4,
        std::make_shared<AggregateTypeSyntax>(
            "float4",
            "float4(0.0)",
            "float4(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VECTOR4_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::MATRIX33,
        std::make_shared<AggregateTypeSyntax>(
            "float3x3",
            "float3x3(1.0)",
            "float3x3(1.0)")
    );

    registerTypeSyntax
    (
        Type::MATRIX44,
        std::make_shared<AggregateTypeSyntax>(
            "float4x4",
            "float4x4(1.0)",
            "float4x4(1.0)")
    );

    registerTypeSyntax
    (
        Type::STRING,
        std::make_shared<StringTypeSyntax>(
            "string",
            "\"\"",
            "\"\"")
    );

    registerTypeSyntax
    (
        Type::FILENAME,
        std::make_shared<MdlFilenameTypeSyntax>()
    );

    registerTypeSyntax
    (
        Type::BSDF,
        std::make_shared<ScalarTypeSyntax>(
            "material",
            "material()",
            "material()")
    );

    registerTypeSyntax
    (
        Type::EDF,
        std::make_shared<ScalarTypeSyntax>(
            "material",
            "material()",
            "material()")
    );

    registerTypeSyntax
    (
        Type::VDF,
        std::make_shared<ScalarTypeSyntax>(
            "material",
            "material()",
            "material()")
    );

    registerTypeSyntax
    (
        Type::SURFACESHADER,
        std::make_shared<ScalarTypeSyntax>(
            "material",
            "material()",
            "material()")
    );

    registerTypeSyntax
    (
        Type::VOLUMESHADER,
        std::make_shared<ScalarTypeSyntax>(
            "material",
            "material()",
            "material()")
    );

    registerTypeSyntax
    (
        Type::DISPLACEMENTSHADER,
        std::make_shared<ScalarTypeSyntax>(
            "material",
            "material()",
            "material()")
    );

    registerTypeSyntax
    (
        Type::LIGHTSHADER,
        std::make_shared<ScalarTypeSyntax>(
            "material",
            "material()",
            "material()")
    );
}

} // namespace MaterialX
