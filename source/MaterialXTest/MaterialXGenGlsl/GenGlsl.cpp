//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXTest/MaterialXGenGlsl/GenGlsl.h>

// Including this file here just to ensure it compiles
// Its not used anywhere in the MaterialX repo - but is
// added to allow backwards compatibility with OpenUSD
// codebase
#include <MaterialXGenGlsl/Nodes/SurfaceNodeGlsl.h>

#include <MaterialXGenGlsl/EsslShaderGenerator.h>
#include <MaterialXGenGlsl/EsslSyntax.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/GlslSyntax.h>
#include <MaterialXGenGlsl/GlslResourceBindingContext.h>
#include <MaterialXGenGlsl/VkShaderGenerator.h>
#include <MaterialXGenGlsl/WgslShaderGenerator.h>
#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenShader/ShaderGraphHash.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXFormat/Util.h>

#include <iomanip>
#include <sstream>

namespace mx = MaterialX;

TEST_CASE("GenShader: GLSL Syntax Check", "[genglsl]")
{
    mx::TypeSystemPtr ts = mx::TypeSystem::create();
    mx::SyntaxPtr syntax = mx::GlslSyntax::create(ts);

    REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
    REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "vec3");
    REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "vec3");

    REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
    REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "out BSDF");

    // Set fixed precision with one digit
    mx::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

    std::string value;
    value = syntax->getDefaultValue(mx::Type::FLOAT);
    REQUIRE(value == "0.0");
    value = syntax->getDefaultValue(mx::Type::COLOR3);
    REQUIRE(value == "vec3(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR3, true);
    REQUIRE(value == "vec3(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4);
    REQUIRE(value == "vec4(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4, true);
    REQUIRE(value == "vec4(0.0)");
    value = syntax->getDefaultValue(mx::Type::FLOATARRAY, true);
    REQUIRE(value.empty());
    value = syntax->getDefaultValue(mx::Type::INTEGERARRAY, true);
    REQUIRE(value.empty());

    mx::ValuePtr floatValue = mx::Value::createValue<float>(42.0f);
    value = syntax->getValue(mx::Type::FLOAT, *floatValue);
    REQUIRE(value == "42.0");
    value = syntax->getValue(mx::Type::FLOAT, *floatValue, true);
    REQUIRE(value == "42.0");

    mx::ValuePtr color3Value = mx::Value::createValue<mx::Color3>(mx::Color3(1.0f, 2.0f, 3.0f));
    value = syntax->getValue(mx::Type::COLOR3, *color3Value);
    REQUIRE(value == "vec3(1.0, 2.0, 3.0)");
    value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
    REQUIRE(value == "vec3(1.0, 2.0, 3.0)");

    mx::ValuePtr color4Value = mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
    value = syntax->getValue(mx::Type::COLOR4, *color4Value);
    REQUIRE(value == "vec4(1.0, 2.0, 3.0, 4.0)");
    value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
    REQUIRE(value == "vec4(1.0, 2.0, 3.0, 4.0)");

    std::vector<float> floatArray = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f };
    mx::ValuePtr floatArrayValue = mx::Value::createValue<std::vector<float>>(floatArray);
    value = syntax->getValue(mx::Type::FLOATARRAY, *floatArrayValue);
    REQUIRE(value == "float[7](0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7)");

    std::vector<int> intArray = { 1, 2, 3, 4, 5, 6, 7 };
    mx::ValuePtr intArrayValue = mx::Value::createValue<std::vector<int>>(intArray);
    value = syntax->getValue(mx::Type::INTEGERARRAY, *intArrayValue);
    REQUIRE(value == "int[7](1, 2, 3, 4, 5, 6, 7)");
}

TEST_CASE("GenShader: GLSL Implementation Check", "[genglsl]")
{
    mx::GenContext context(mx::GlslShaderGenerator::create());

    mx::StringSet generatorSkipNodeTypes;
    mx::StringSet generatorSkipNodeDefs;
    GenShaderUtil::checkImplementations(context, generatorSkipNodeTypes, generatorSkipNodeDefs);
}

TEST_CASE("GenShader: GLSL Unique Names", "[genglsl]")
{
    mx::GenContext context(mx::GlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(mx::getDefaultDataSearchPath());
    GenShaderUtil::testUniqueNames(context, mx::Stage::PIXEL);
}

TEST_CASE("GenShader: GLSL Light Shaders", "[genglsl]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    loadLibraries({ "libraries" }, searchPath, doc);

    mx::NodeDefPtr pointLightShader = doc->getNodeDef("ND_point_light");
    mx::NodeDefPtr spotLightShader = doc->getNodeDef("ND_spot_light");
    REQUIRE(pointLightShader != nullptr);
    REQUIRE(spotLightShader != nullptr);

    mx::GenContext context(mx::GlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);
    context.getShaderGenerator().registerTypeDefs(doc);

    mx::HwShaderGenerator::bindLightShader(*pointLightShader, 42, context);
    REQUIRE_THROWS(mx::HwShaderGenerator::bindLightShader(*spotLightShader, 42, context));
    mx::HwShaderGenerator::unbindLightShader(42, context);
    REQUIRE_NOTHROW(mx::HwShaderGenerator::bindLightShader(*spotLightShader, 42, context));
    REQUIRE_NOTHROW(mx::HwShaderGenerator::bindLightShader(*pointLightShader, 66, context));
    mx::HwShaderGenerator::unbindLightShaders(context);
    REQUIRE_NOTHROW(mx::HwShaderGenerator::bindLightShader(*spotLightShader, 66, context));
}

#ifdef MATERIALX_BUILD_BENCHMARK_TESTS
TEST_CASE("GenShader: GLSL Performance Test", "[genglsl]")
{
    mx::GenContext context(mx::GlslShaderGenerator::create());
    BENCHMARK("Load documents, validate and generate shader") 
    {
        return GenShaderUtil::shaderGenPerformanceTest(context);
    };
}
#endif

enum class GlslType
{
    Essl,
    Glsl,
    GlslLayout,
    GlslVulkan,
    GlslWgsl
};

static void generateGlslCode(GlslType type)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(searchPath.find("resources/Materials/TestSuite"));
    testRootPaths.push_back(searchPath.find("resources/Materials/Examples"));

    // Create the requested shader generator.
    mx::ShaderGeneratorPtr generator;
    if (type == GlslType::Essl)
    {
        generator = mx::EsslShaderGenerator::create();
    }
    else if (type == GlslType::GlslVulkan)
    {
        generator = mx::VkShaderGenerator::create();
    }
    else if (type == GlslType::GlslWgsl)
    {
        generator = mx::WgslShaderGenerator::create();
    }
    else
    {
        generator = mx::GlslShaderGenerator::create();
    }

    const std::unordered_map<GlslType, std::string> TYPE_NAME_MAP =
    {
        { GlslType::Essl, "essl" },
        { GlslType::Glsl, "glsl" },
        { GlslType::GlslLayout, "glsl_layout" },
        { GlslType::GlslVulkan, "glsl_vulkan" },
        { GlslType::GlslWgsl  , "glsl_wgsl" }
    };
    const mx::FilePath logPath("genglsl_" + TYPE_NAME_MAP.at(type) + "_generate_test.txt");
    GlslShaderGeneratorTester tester(generator, testRootPaths, searchPath, logPath, false);

    // Handle resource binding layouts if requested.
    if (type == GlslType::GlslLayout)
    {
        mx::GlslResourceBindingContextPtr glslresourceBinding(mx::GlslResourceBindingContext::create());
        glslresourceBinding->enableSeparateBindingLocations(true);
        tester.addUserData(mx::HW::USER_DATA_BINDING_CONTEXT, glslresourceBinding);
    }

    const mx::GenOptions genOptions;
    mx::FilePath optionsFilePath = searchPath.find("resources/Materials/TestSuite/_options.mtlx");
    tester.validate(genOptions, optionsFilePath);
}

TEST_CASE("GenShader: ESSL Shader Generation", "[genglsl]")
{
    generateGlslCode(GlslType::Essl);
}

TEST_CASE("GenShader: GLSL Shader Generation", "[genglsl]")
{
    generateGlslCode(GlslType::Glsl);
}

TEST_CASE("GenShader: GLSL Shader Generation with Layout", "[genglsl]")
{
    generateGlslCode(GlslType::GlslLayout);
}

TEST_CASE("GenShader: Vulkan GLSL Shader Generation", "[genglsl]")
{
    generateGlslCode(GlslType::GlslVulkan);
}

TEST_CASE("GenShader: Wgsl GLSL Shader Generation", "[genglsl]")
{
    generateGlslCode(GlslType::GlslWgsl);
}

TEST_CASE("GenShader: GLSL Structural Hash", "[genglsl]")
{
    mx::DocumentPtr nodeLibrary = mx::createDocument();
    const mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    loadLibraries({ "libraries" }, searchPath, nodeLibrary);

    mx::GenContext context(mx::GlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);

    mx::DefaultColorManagementSystemPtr colorManagementSystem =
        mx::DefaultColorManagementSystem::create(context.getShaderGenerator().getTarget());
    REQUIRE(colorManagementSystem);
    context.getShaderGenerator().setColorManagementSystem(colorManagementSystem);
    colorManagementSystem->loadLibrary(nodeLibrary);

    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(context.getShaderGenerator().getTarget());
    REQUIRE(unitSystem);
    context.getShaderGenerator().setUnitSystem(unitSystem);
    unitSystem->loadLibrary(nodeLibrary);
    unitSystem->setUnitConverterRegistry(mx::UnitConverterRegistry::create());
    mx::UnitTypeDefPtr distanceTypeDef = nodeLibrary->getUnitTypeDef("distance");
    unitSystem->getUnitConverterRegistry()->addUnitConverter(distanceTypeDef, mx::LinearUnitConverter::create(distanceTypeDef));
    mx::UnitTypeDefPtr angleTypeDef = nodeLibrary->getUnitTypeDef("angle");
    unitSystem->getUnitConverterRegistry()->addUnitConverter(angleTypeDef, mx::LinearUnitConverter::create(angleTypeDef));
    context.getOptions().targetDistanceUnit = "meter";

    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(searchPath.find("resources/Materials/Examples/StandardSurface"));

    std::vector<mx::DocumentPtr> loadedDocuments;
    mx::StringVec documentsPaths;
    mx::StringVec errorLog;

    for (const auto& testRoot : testRootPaths)
    {
        mx::loadDocuments(testRoot, searchPath, {}, {}, loadedDocuments, documentsPaths,
                          nullptr, &errorLog);
    }

    REQUIRE(loadedDocuments.size() > 0);

    std::ostringstream hashLog;
    hashLog << std::hex << std::setfill('0');
    hashLog << "\n=== Structural Hash Results ===\n";

    for (size_t docIdx = 0; docIdx < loadedDocuments.size(); ++docIdx)
    {
        mx::DocumentPtr doc = loadedDocuments[docIdx];
        doc->setDataLibrary(nodeLibrary);

        std::string message;
        bool docValid = doc->validate(&message);
        if (!docValid)
        {
            continue;
        }

        context.getShaderGenerator().registerTypeDefs(doc);

        std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
        for (const mx::TypedElementPtr& element : elements)
        {
            mx::ShaderPtr shader;
            try
            {
                shader = context.getShaderGenerator().generate(element->getName(), element, context);
            }
            catch (const std::exception&)
            {
                continue;
            }

            REQUIRE(shader != nullptr);

            const mx::GenOptions& options = context.getOptions();
            size_t hash1 = mx::computeStructuralHash(shader->getGraph(), options);
            REQUIRE(hash1 != 0);

            // The options-free overload must produce a different hash
            // (unless all topology-affecting options happen to hash to zero,
            // which is astronomically unlikely).
            size_t hashNoOpts = mx::computeStructuralHash(shader->getGraph());
            REQUIRE(hashNoOpts != 0);

            // Determinism check: generate the same shader again and verify the hash matches.
            mx::ShaderPtr shader2;
            try
            {
                shader2 = context.getShaderGenerator().generate(element->getName(), element, context);
            }
            catch (const std::exception&)
            {
                continue;
            }

            REQUIRE(shader2 != nullptr);
            size_t hash2 = mx::computeStructuralHash(shader2->getGraph(), options);
            REQUIRE(hash1 == hash2);

            hashLog << "  " << documentsPaths[docIdx] << " | "
                    << element->getName() << " | 0x"
                    << std::setw(sizeof(size_t) * 2) << hash1 << "\n";
        }
    }

    hashLog << "=== End Structural Hash Results ===\n";

    // Output to Catch2 INFO so it appears with -s flag
    INFO(hashLog.str());
    SUCCEED();
}

TEST_CASE("GenShader: GLSL Structural Hash GenOptions Sensitivity", "[genglsl]")
{
    mx::DocumentPtr nodeLibrary = mx::createDocument();
    const mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    loadLibraries({ "libraries" }, searchPath, nodeLibrary);

    mx::GenContext context(mx::GlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);

    mx::DefaultColorManagementSystemPtr colorManagementSystem =
        mx::DefaultColorManagementSystem::create(context.getShaderGenerator().getTarget());
    REQUIRE(colorManagementSystem);
    context.getShaderGenerator().setColorManagementSystem(colorManagementSystem);
    colorManagementSystem->loadLibrary(nodeLibrary);

    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(context.getShaderGenerator().getTarget());
    REQUIRE(unitSystem);
    context.getShaderGenerator().setUnitSystem(unitSystem);
    unitSystem->loadLibrary(nodeLibrary);
    unitSystem->setUnitConverterRegistry(mx::UnitConverterRegistry::create());
    mx::UnitTypeDefPtr distanceTypeDef = nodeLibrary->getUnitTypeDef("distance");
    unitSystem->getUnitConverterRegistry()->addUnitConverter(distanceTypeDef, mx::LinearUnitConverter::create(distanceTypeDef));
    mx::UnitTypeDefPtr angleTypeDef = nodeLibrary->getUnitTypeDef("angle");
    unitSystem->getUnitConverterRegistry()->addUnitConverter(angleTypeDef, mx::LinearUnitConverter::create(angleTypeDef));
    context.getOptions().targetDistanceUnit = "meter";

    mx::FilePath materialPath = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx");
    REQUIRE(!materialPath.isEmpty());

    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, materialPath, searchPath);
    doc->setDataLibrary(nodeLibrary);

    std::string message;
    REQUIRE(doc->validate(&message));
    context.getShaderGenerator().registerTypeDefs(doc);

    std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
    REQUIRE(!elements.empty());
    mx::TypedElementPtr element = elements[0];

    // Generate baseline shader with default options.
    mx::GenOptions baselineOptions = context.getOptions();
    mx::ShaderPtr baselineShader = context.getShaderGenerator().generate(
        element->getName(), element, context);
    REQUIRE(baselineShader != nullptr);

    size_t baselineHash = mx::computeStructuralHash(
        baselineShader->getGraph(), baselineOptions);
    REQUIRE(baselineHash != 0);

    // Helper: generate with modified options and verify the options-aware
    // hash differs from the baseline.
    auto verifyOptionChangesHash = [&](const std::string& label,
                                       std::function<void(mx::GenOptions&)> mutate)
    {
        INFO("Option: " << label);

        mx::GenOptions modifiedOptions = baselineOptions;
        mutate(modifiedOptions);
        context.getOptions() = modifiedOptions;

        mx::ShaderPtr shader;
        try
        {
            shader = context.getShaderGenerator().generate(
                element->getName(), element, context);
        }
        catch (const std::exception&)
        {
            context.getOptions() = baselineOptions;
            return;
        }
        REQUIRE(shader != nullptr);

        size_t modifiedHash = mx::computeStructuralHash(
            shader->getGraph(), modifiedOptions);
        REQUIRE(modifiedHash != 0);
        REQUIRE(modifiedHash != baselineHash);

        context.getOptions() = baselineOptions;
    };

    SECTION("shaderInterfaceType changes hash")
    {
        verifyOptionChangesHash("shaderInterfaceType", [](mx::GenOptions& opts) {
            opts.shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;
        });
    }

    SECTION("emitColorTransforms changes hash")
    {
        verifyOptionChangesHash("emitColorTransforms", [](mx::GenOptions& opts) {
            opts.emitColorTransforms = !opts.emitColorTransforms;
        });
    }

    SECTION("elideConstantNodes changes hash")
    {
        verifyOptionChangesHash("elideConstantNodes", [](mx::GenOptions& opts) {
            opts.elideConstantNodes = !opts.elideConstantNodes;
        });
    }

    SECTION("targetColorSpaceOverride changes hash")
    {
        verifyOptionChangesHash("targetColorSpaceOverride", [](mx::GenOptions& opts) {
            opts.targetColorSpaceOverride = "acescg";
        });
    }

    SECTION("targetDistanceUnit changes hash")
    {
        verifyOptionChangesHash("targetDistanceUnit", [](mx::GenOptions& opts) {
            opts.targetDistanceUnit = "centimeter";
        });
    }

    SECTION("non-topology options do NOT change hash")
    {
        mx::GenOptions modifiedOptions = baselineOptions;
        modifiedOptions.hwTransparency = !modifiedOptions.hwTransparency;
        modifiedOptions.hwShadowMap = !modifiedOptions.hwShadowMap;
        modifiedOptions.hwAmbientOcclusion = !modifiedOptions.hwAmbientOcclusion;
        modifiedOptions.hwSrgbEncodeOutput = !modifiedOptions.hwSrgbEncodeOutput;
        modifiedOptions.hwMaxActiveLightSources = 99;
        modifiedOptions.fileTextureVerticalFlip = !modifiedOptions.fileTextureVerticalFlip;
        modifiedOptions.hwImplicitBitangents = !modifiedOptions.hwImplicitBitangents;

        size_t sameGraphHash = mx::computeStructuralHash(
            baselineShader->getGraph(), modifiedOptions);
        REQUIRE(sameGraphHash == baselineHash);
    }
}

TEST_CASE("GenShader: GLSL Generic Input Naming", "[genglsl]")
{
    mx::DocumentPtr nodeLibrary = mx::createDocument();
    const mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    loadLibraries({ "libraries" }, searchPath, nodeLibrary);

    mx::GenContext context(mx::GlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);

    mx::DefaultColorManagementSystemPtr colorManagementSystem =
        mx::DefaultColorManagementSystem::create(context.getShaderGenerator().getTarget());
    REQUIRE(colorManagementSystem);
    context.getShaderGenerator().setColorManagementSystem(colorManagementSystem);
    colorManagementSystem->loadLibrary(nodeLibrary);

    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(context.getShaderGenerator().getTarget());
    REQUIRE(unitSystem);
    context.getShaderGenerator().setUnitSystem(unitSystem);
    unitSystem->loadLibrary(nodeLibrary);
    unitSystem->setUnitConverterRegistry(mx::UnitConverterRegistry::create());
    mx::UnitTypeDefPtr distanceTypeDef = nodeLibrary->getUnitTypeDef("distance");
    unitSystem->getUnitConverterRegistry()->addUnitConverter(distanceTypeDef, mx::LinearUnitConverter::create(distanceTypeDef));
    mx::UnitTypeDefPtr angleTypeDef = nodeLibrary->getUnitTypeDef("angle");
    unitSystem->getUnitConverterRegistry()->addUnitConverter(angleTypeDef, mx::LinearUnitConverter::create(angleTypeDef));
    context.getOptions().targetDistanceUnit = "meter";

    // Generate shaders for three structurally identical standard_surface materials
    // and verify they produce the same pixel shader source (generic uniform names).
    const std::vector<std::string> materialFiles = {
        "resources/Materials/Examples/StandardSurface/standard_surface_carpaint.mtlx",
        "resources/Materials/Examples/StandardSurface/standard_surface_chrome.mtlx",
        "resources/Materials/Examples/StandardSurface/standard_surface_glass.mtlx"
    };

    std::vector<std::string> pixelSources;
    for (const auto& file : materialFiles)
    {
        mx::FilePath filePath = searchPath.find(file);
        REQUIRE(!filePath.isEmpty());

        mx::DocumentPtr doc = mx::createDocument();
        mx::readFromXmlFile(doc, filePath, searchPath);
        doc->setDataLibrary(nodeLibrary);

        std::string message;
        REQUIRE(doc->validate(&message));

        context.getShaderGenerator().registerTypeDefs(doc);

        std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
        REQUIRE(!elements.empty());

        mx::ShaderPtr shader = context.getShaderGenerator().generate(
            elements[0]->getName(), elements[0], context);
        REQUIRE(shader != nullptr);

        const mx::ShaderStage& pixelStage = shader->getStage(mx::Stage::PIXEL);
        pixelSources.push_back(pixelStage.getSourceCode());
    }

    // All three should produce identical shader source (only default values differ
    // in the uniform declarations, but the variable names must be the same).
    // Extract just the uniform lines for comparison.
    auto extractUniformNames = [](const std::string& source) -> std::vector<std::string>
    {
        std::vector<std::string> names;
        std::istringstream stream(source);
        std::string line;
        while (std::getline(stream, line))
        {
            if (line.find("uniform ") != std::string::npos)
            {
                // Trim trailing whitespace from the region before '=' or ';'
                auto eqPos = line.find('=');
                auto semiPos = line.find(';');
                auto endPos = (eqPos != std::string::npos) ? eqPos : semiPos;
                if (endPos == std::string::npos) continue;

                // Walk backwards past spaces to find the end of the name
                size_t nameEnd = endPos;
                while (nameEnd > 0 && line[nameEnd - 1] == ' ') --nameEnd;
                // Walk backwards past the name to find the start
                size_t nameStart = nameEnd;
                while (nameStart > 0 && line[nameStart - 1] != ' ') --nameStart;

                if (nameStart < nameEnd)
                {
                    names.push_back(line.substr(nameStart, nameEnd - nameStart));
                }
            }
        }
        return names;
    };

    auto carpaintUniforms = extractUniformNames(pixelSources[0]);
    auto chromeUniforms = extractUniformNames(pixelSources[1]);
    auto glassUniforms = extractUniformNames(pixelSources[2]);

    REQUIRE(carpaintUniforms.size() == chromeUniforms.size());
    REQUIRE(carpaintUniforms.size() == glassUniforms.size());

    for (size_t i = 0; i < carpaintUniforms.size(); ++i)
    {
        CHECK(carpaintUniforms[i] == chromeUniforms[i]);
        CHECK(carpaintUniforms[i] == glassUniforms[i]);
    }

    // Verify that the surfaceshader node inputs use generic names
    bool foundSurfaceshaderBase = false;
    for (const auto& name : carpaintUniforms)
    {
        CHECK(name.find("SR_carpaint") == std::string::npos);
        CHECK(name.find("SR_chrome") == std::string::npos);
        CHECK(name.find("SR_glass") == std::string::npos);
        if (name == "surfaceshader_base")
        {
            foundSurfaceshaderBase = true;
        }
    }
    CHECK(foundSurfaceshaderBase);

    // Verify that local variable names in the shader body are also generic.
    // The pixel source should contain "surfaceshader_out" and "material_out"
    // instead of instance-specific names like "SR_carpaint_out" or "Car_Paint_out".
    for (size_t i = 0; i < pixelSources.size(); ++i)
    {
        CHECK(pixelSources[i].find("surfaceshader_out") != std::string::npos);
        CHECK(pixelSources[i].find("material_out") != std::string::npos);
    }
    CHECK(pixelSources[0].find("SR_carpaint_out") == std::string::npos);
    CHECK(pixelSources[0].find("Car_Paint_out") == std::string::npos);
    CHECK(pixelSources[1].find("SR_chrome_out") == std::string::npos);
    CHECK(pixelSources[1].find("Chrome_out") == std::string::npos);
    CHECK(pixelSources[2].find("SR_glass_out") == std::string::npos);
    CHECK(pixelSources[2].find("Glass_out") == std::string::npos);

    // Log the uniform names for inspection
    std::ostringstream log;
    log << "\n=== Generic Uniform Names (Car_Paint) ===\n";
    for (const auto& name : carpaintUniforms)
    {
        log << "  " << name << "\n";
    }
    INFO(log.str());
    SUCCEED();
}
