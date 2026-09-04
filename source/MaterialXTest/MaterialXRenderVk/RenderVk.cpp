//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXRenderVk/VkContext.h>
#include <MaterialXRenderVk/VkRenderer.h>
#include <MaterialXRenderVk/VkProgram.h>

#include <MaterialXGenGlsl/VkShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/UnitSystem.h>
#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlIo.h>

namespace mx = MaterialX;

//
// Phase 2 reflection test: generate a lit standard_surface shader with
// VkShaderGenerator, compile it to SPIR-V, and verify the introspected
// uniform/attribute lists match what glslang reflection reported.
//
TEST_CASE("Render: Vulkan Reflection", "[rendervk]")
{
    // Skip on headless machines with no Vulkan device.
    if (!mx::VkContext::isDeviceAvailable())
    {
        std::cerr << "No Vulkan device available. Skip Vulkan reflection test." << std::endl;
        return;
    }

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr stdlib = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, stdlib);

    mx::DocumentPtr doc = mx::createDocument();
    doc->setDataLibrary(stdlib);

    mx::FilePath materialFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx");
    REQUIRE(!materialFile.isEmpty());
    mx::readFromXmlFile(doc, materialFile, searchPath);

    // Load a light rig so light shaders are present and u_lightData is emitted.
    mx::FilePath lightRig = searchPath.find(
        "resources/Materials/TestSuite/lights/light_rig_test_1.mtlx");
    if (!lightRig.isEmpty())
    {
        mx::DocumentPtr lightDoc = mx::createDocument();
        mx::readFromXmlFile(lightDoc, lightRig, searchPath);
        doc->importLibrary(lightDoc);
    }

    auto generator = mx::VkShaderGenerator::create();
    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(searchPath);
    generator->registerShaderMetadata(stdlib, context);

    auto cms = mx::DefaultColorManagementSystem::create(generator->getTarget());
    generator->setColorManagementSystem(cms);
    cms->loadLibrary(stdlib);

    auto unitSystem = mx::UnitSystem::create(generator->getTarget());
    generator->setUnitSystem(unitSystem);
    unitSystem->loadLibrary(stdlib);
    unitSystem->setUnitConverterRegistry(mx::UnitConverterRegistry::create());

    // Bind light shaders.
    int lightId = 0;
    for (mx::NodePtr node : doc->getNodes())
    {
        if (node->getType() == mx::LIGHT_SHADER_TYPE_STRING)
        {
            mx::NodeDefPtr nodeDef = node->getNodeDef();
            if (nodeDef)
            {
                mx::HwShaderGenerator::bindLightShader(*nodeDef, lightId, context);
                lightId++;
            }
        }
    }
    context.getOptions().hwMaxActiveLightSources = (lightId > 0) ? lightId : 3;

    std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(doc);
    REQUIRE(!elements.empty());

    mx::ShaderPtr shader = generator->generate(elements[0]->getName(), elements[0], context);

    // Create a renderer and compile the shader.
    mx::VkRendererPtr renderer = mx::VkRenderer::create(512, 512, mx::Image::BaseType::UINT8);
    renderer->initialize();
    renderer->createProgram(shader);

    mx::VkProgramPtr program = renderer->getProgram();
    REQUIRE(program != nullptr);

    const mx::VkProgram::InputMap& uniforms = program->getUniformsList();
    const mx::VkProgram::InputMap& attributes = program->getAttributesList();

    // Check key uniforms are present (Q1/Q3 from Phase 0).
    CHECK(uniforms.find("u_worldMatrix") != uniforms.end());
    CHECK(uniforms.find("u_viewPosition") != uniforms.end());
    CHECK(uniforms.find("u_envRadiance") != uniforms.end());
    CHECK(uniforms.find("u_numActiveLightSources") != uniforms.end());

    // Check the light-data array members are reflected under composed names (Q1).
    CHECK(uniforms.find("u_lightData[0].direction") != uniforms.end());
    CHECK(uniforms.find("u_lightData[0].color") != uniforms.end());
    CHECK(uniforms.find("u_lightData[0].type") != uniforms.end());

    // Check vertex attributes (Q3 pipe inputs).
    CHECK(attributes.find("i_position") != attributes.end());
    auto posIt = attributes.find("i_position");
    if (posIt != attributes.end())
    {
        CHECK(posIt->second->location == 0);
    }

    // Check that u_worldMatrix is a mat4 in a UBO with a sane offset (Q2).
    auto worldIt = uniforms.find("u_worldMatrix");
    if (worldIt != uniforms.end())
    {
        CHECK(worldIt->second->glType == 0x8b5c); // GL_FLOAT_MAT4
        CHECK(worldIt->second->offset == 0);
        CHECK(worldIt->second->blockIndex >= 0);
    }

    // Check that samplers are flagged (Q3).
    auto envRadIt = uniforms.find("u_envRadiance");
    if (envRadIt != uniforms.end())
    {
        CHECK(envRadIt->second->isSampler);
        CHECK(envRadIt->second->blockIndex == -1);
        CHECK(envRadIt->second->binding >= 0);
    }

    // printUniforms should produce sane output.
    std::stringstream ss;
    program->printUniforms(ss);
    CHECK(!ss.str().empty());

    // Trap #2 regression: a Matrix33 bound via bindUniform must be written row-expanded
    // (3 columns of vec4 = 48 bytes) into the std140 shadow, NOT tightly packed (36 bytes).
    // We bind a known Matrix33 to u_worldMatrix (a mat4 UBO member) — the type dispatch
    // keys off the *value's* type string, so a Matrix33 value triggers the mat3 path.
    // This verifies the row-expansion write doesn't corrupt adjacent memory.
    mx::Matrix33 testMatrix(1.0f, 2.0f, 3.0f,
                           4.0f, 5.0f, 6.0f,
                           7.0f, 8.0f, 9.0f);
    if (worldIt != uniforms.end() && worldIt->second->blockIndex >= 0)
    {
        size_t blockIndex = static_cast<size_t>(worldIt->second->blockIndex);
        const auto& blocks = program->getUniformBlocks();
        REQUIRE(blockIndex < blocks.size());

        // Snapshot the shadow before binding.
        std::vector<uint8_t> before = blocks[blockIndex].shadow;

        program->bindUniform("u_worldMatrix", mx::Value::createValue<mx::Matrix33>(testMatrix), false);

        const auto& after = program->getUniformBlocks()[blockIndex].shadow;
        size_t offset = static_cast<size_t>(worldIt->second->offset);

        // The mat3 write should place 3 floats at offset, offset+16, offset+32
        // (row-expanded to vec4 stride), leaving the 4th component of each row untouched.
        REQUIRE(offset + 48 <= after.size());
        float row0[3], row1[3], row2[3];
        std::memcpy(row0, &after[offset], 12);
        std::memcpy(row1, &after[offset + 16], 12);
        std::memcpy(row2, &after[offset + 32], 12);
        CHECK(row0[0] == 1.0f); CHECK(row0[1] == 2.0f); CHECK(row0[2] == 3.0f);
        CHECK(row1[0] == 4.0f); CHECK(row1[1] == 5.0f); CHECK(row1[2] == 6.0f);
        CHECK(row2[0] == 7.0f); CHECK(row2[1] == 8.0f); CHECK(row2[2] == 9.0f);

        // The padding bytes (4th component of each row) must be untouched.
        float pad0, pad1, pad2;
        std::memcpy(&pad0, &after[offset + 12], 4);
        std::memcpy(&pad1, &after[offset + 28], 4);
        std::memcpy(&pad2, &after[offset + 44], 4);
        float beforePad0, beforePad1, beforePad2;
        std::memcpy(&beforePad0, &before[offset + 12], 4);
        std::memcpy(&beforePad1, &before[offset + 28], 4);
        std::memcpy(&beforePad2, &before[offset + 44], 4);
        CHECK(pad0 == beforePad0);
        CHECK(pad1 == beforePad1);
        CHECK(pad2 == beforePad2);
    }
}
