//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//


#include <MaterialXTest/RenderUtil.h>

namespace mx = MaterialX;

namespace RenderUtil
{
void createLightRig(mx::DocumentPtr doc, mx::LightHandler& lightHandler, mx::GenContext& context,
                    const mx::FilePath& envIrradiancePath, const mx::FilePath& envRadiancePath)
{
    // Scan for lights
    std::vector<mx::NodePtr> lights;
    lightHandler.findLights(doc, lights);
    lightHandler.registerLights(doc, lights, context);

    // Set the list of lights on the with the generator
    lightHandler.setLightSources(lights);
    // Set up IBL inputs
    lightHandler.setLightEnvIrradiancePath(envIrradiancePath);
    lightHandler.setLightEnvRadiancePath(envRadiancePath);
}

// Create a list of generation options based on unit test options
// These options will override the original generation context options.
void getGenerationOptions(const ShaderValidTestOptions& testOptions,
    const mx::GenOptions& originalOptions,
    std::vector<mx::GenOptions>& optionsList)
{
    optionsList.clear();
    if (testOptions.shaderInterfaces & 1)
    {
        mx::GenOptions reducedOption = originalOptions;
        reducedOption.shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;
        optionsList.push_back(reducedOption);
    }
    // Alway fallback to complete if no options specified.
    if ((testOptions.shaderInterfaces & 2) || optionsList.empty())
    {
        mx::GenOptions completeOption = originalOptions;
        completeOption.shaderInterfaceType = mx::SHADER_INTERFACE_COMPLETE;
        optionsList.push_back(completeOption);
    }
}


bool getTestOptions(const std::string& optionFile, ShaderValidTestOptions& options)
{
    const std::string SHADER_VALID_TEST_OPTIONS_STRING("ShaderValidTestOptions");
    const std::string OVERRIDE_FILES_STRING("overrideFiles");
    const std::string LIGHT_FILES_STRING("lightFiles");
    const std::string SHADER_INTERFACES_STRING("shaderInterfaces");
    const std::string VALIDATE_ELEMENT_TO_RENDER_STRING("validateElementToRender");
    const std::string COMPILE_CODE_STRING("compileCode");
    const std::string RENDER_IMAGES_STRING("renderImages");
    const std::string SAVE_IMAGES_STRING("saveImages");
    const std::string DUMP_GLSL_UNIFORMS_AND_ATTRIBUTES_STRING("dumpGlslUniformsAndAttributes");
    const std::string RUN_OSL_TESTS_STRING("runOSLTests");
    const std::string RUN_GLSL_TESTS_STRING("runGLSLTests");
    const std::string RUN_OGSFX_TESTS_STRING("runOGSFXTests");
    const std::string CHECK_IMPL_COUNT_STRING("checkImplCount");
    const std::string DUMP_GENERATED_CODE_STRING("dumpGeneratedCode");
    const std::string GLSL_NONSHADER_GEOMETRY_STRING("glslNonShaderGeometry");
    const std::string GLSL_SHADER_GEOMETRY_STRING("glslShaderGeometry");
    const std::string RADIANCE_IBL_PATH_STRING("radianceIBLPath");
    const std::string IRRADIANCE_IBL_PATH_STRING("irradianceIBLPath");
    const std::string SPHERE_OBJ("sphere.obj");
    const std::string SHADERBALL_OBJ("shaderball.obj");

    options.overrideFiles.clear();
    options.dumpGeneratedCode = false;
    options.glslNonShaderGeometry = SPHERE_OBJ;
    options.glslShaderGeometry = SHADERBALL_OBJ;

    MaterialX::DocumentPtr doc = MaterialX::createDocument();
    try {
        MaterialX::readFromXmlFile(doc, optionFile);

        MaterialX::NodeDefPtr optionDefs = doc->getNodeDef(SHADER_VALID_TEST_OPTIONS_STRING);
        if (optionDefs)
        {
            for (MaterialX::ParameterPtr p : optionDefs->getParameters())
            {
                const std::string& name = p->getName();
                MaterialX::ValuePtr val = p->getValue();
                if (val)
                {
                    if (name == OVERRIDE_FILES_STRING)
                    {
                        options.overrideFiles = MaterialX::splitString(p->getValueString(), ",");
                    }
                    if (name == LIGHT_FILES_STRING)
                    {
                        options.lightFiles = MaterialX::splitString(p->getValueString(), ",");
                    }
                    else if (name == SHADER_INTERFACES_STRING)
                    {
                        options.shaderInterfaces = val->asA<int>();
                    }
                    else if (name == VALIDATE_ELEMENT_TO_RENDER_STRING)
                    {
                        options.validateElementToRender = val->asA<bool>();
                    }
                    else if (name == COMPILE_CODE_STRING)
                    {
                        options.compileCode = val->asA<bool>();
                    }
                    else if (name == RENDER_IMAGES_STRING)
                    {
                        options.renderImages = val->asA<bool>();
                    }
                    else if (name == SAVE_IMAGES_STRING)
                    {
                        options.saveImages = val->asA<bool>();
                    }
                    else if (name == DUMP_GLSL_UNIFORMS_AND_ATTRIBUTES_STRING)
                    {
                        options.dumpGlslUniformsAndAttributes = val->asA<bool>();
                    }
                    else if (name == RUN_OSL_TESTS_STRING)
                    {
                        options.runOSLTests = val->asA<bool>();
                    }
                    else if (name == RUN_GLSL_TESTS_STRING)
                    {
                        options.runGLSLTests = val->asA<bool>();
                    }
                    else if (name == RUN_OGSFX_TESTS_STRING)
                    {
                        options.runOGSFXTests = val->asA<bool>();
                    }
                    else if (name == CHECK_IMPL_COUNT_STRING)
                    {
                        options.checkImplCount = val->asA<bool>();
                    }
                    else if (name == DUMP_GENERATED_CODE_STRING)
                    {
                        options.dumpGeneratedCode = val->asA<bool>();
                    }
                    else if (name == GLSL_NONSHADER_GEOMETRY_STRING)
                    {
                        options.glslNonShaderGeometry = p->getValueString();
                    }
                    else if (name == GLSL_SHADER_GEOMETRY_STRING)
                    {
                        options.glslShaderGeometry = p->getValueString();
                    }
                    else if (name == RADIANCE_IBL_PATH_STRING)
                    {
                        options.radianceIBLPath = p->getValueString();
                    }
                    else if (name == IRRADIANCE_IBL_PATH_STRING)
                    {
                        options.irradianceIBLPath = p->getValueString();
                    }
                }
            }
        }

        // Disable render and save of images if not compiled code will be generated
        if (!options.compileCode)
        {
            options.renderImages = false;
            options.saveImages = false;
        }
        // Disable saving images, if no images are to be produced
        if (!options.renderImages)
        {
            options.saveImages = false;
        }

        // If there is a filter on the files to run turn off profile checking
        if (!options.overrideFiles.empty())
        {
            options.checkImplCount = false;
        }

        // If implementation count check is required, then OSL and GLSL/OGSFX
        // code generation must be executed to be able to check implementation usage.
        if (options.checkImplCount)
        {
            options.runGLSLTests = true;
            options.runOSLTests = true;
            options.runOGSFXTests = true;
        }
        return true;
    }
    catch (mx::Exception& e)
    {
        std::cout << e.what();
    }
    return false;
}

void printRunLog(const ShaderValidProfileTimes &profileTimes,
    const ShaderValidTestOptions& options,
    mx::StringSet& usedImpls,
    std::ostream& profilingLog,
    mx::DocumentPtr dependLib,
    mx::GenContext& context,
    const std::string& language)
{
    profileTimes.print(profilingLog);

    profilingLog << "---------------------------------------" << std::endl;
    options.print(profilingLog);

    if (options.checkImplCount)
    {
        profilingLog << "---------------------------------------" << std::endl;

        // Get implementation count from libraries. 
        std::set<mx::ImplementationPtr> libraryImpls;
        const std::vector<mx::ElementPtr>& children = dependLib->getChildren();
        for (auto child : children)
        {
            mx::ImplementationPtr impl = child->asA<mx::Implementation>();
            if (!impl)
            {
                continue;
            }

            // Only check implementations for languages we're interested in and
            // are testing.
            // 
            if (impl->getLanguage() == language)
            {
                libraryImpls.insert(impl);
            }
        }

        size_t skipCount = 0;
        profilingLog << "-- Possibly missed implementations ----" << std::endl;
        mx::StringVec whiteList =
        {
            "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
            "volumeshader", "IM_constant_", "IM_dot_", "IM_geomattrvalue"
        };
        unsigned int implementationUseCount = 0;
        for (auto libraryImpl : libraryImpls)
        {
            const std::string& implName = libraryImpl->getName();

            // Skip white-list items
            bool inWhiteList = false;
            for (auto w : whiteList)
            {
                if (implName.find(w) != std::string::npos)
                {
                    skipCount++;
                    inWhiteList = true;
                    break;
                }
            }
            if (inWhiteList)
            {
                implementationUseCount++;
                continue;
            }

            if (usedImpls.count(implName))
            {
                implementationUseCount++;
                continue;
            }

            if (context.findNodeImplementation(implName))
            {
                implementationUseCount++;
                continue;
            }
            profilingLog << "\t" << implName << std::endl;
        }
        size_t libraryCount = libraryImpls.size();
        profilingLog << "Tested: " << implementationUseCount << " out of: " << libraryCount << " library implementations." << std::endl;
        // Enable when implementations and testing are complete
        // CHECK(implementationUseCount == libraryCount);
    }
}

} // namespace RenderUtil

