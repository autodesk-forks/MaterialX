#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXFormat/Util.h>

#include <iostream>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

void initContext(mx::GenContext& context, mx::FileSearchPath searchPath, mx::DocumentPtr stdLib, mx::UnitConverterRegistryPtr unitRegistry) {
    // Initialize search paths.
    for (const mx::FilePath& path : searchPath)
    {
        context.registerSourceCodeSearchPath(path / "libraries");
    }

    context.getOptions().targetColorSpaceOverride = "lin_rec709";
    context.getOptions().fileTextureVerticalFlip = false;
    context.getOptions().hwMaxActiveLightSources = 1;
    context.getOptions().hwSpecularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_FIS;
    context.getOptions().hwDirectionalAlbedoMethod = mx::DIRECTIONAL_ALBEDO_CURVE_FIT;
 
    // Initialize color management.
    mx::DefaultColorManagementSystemPtr cms = mx::DefaultColorManagementSystem::create(context.getShaderGenerator().getTarget());
    cms->loadLibrary(stdLib);
    context.getShaderGenerator().setColorManagementSystem(cms);

    // Initialize unit management.
    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(context.getShaderGenerator().getTarget());
    unitSystem->loadLibrary(stdLib);
    unitSystem->setUnitConverterRegistry(unitRegistry);
    context.getShaderGenerator().setUnitSystem(unitSystem);
    context.getOptions().targetDistanceUnit = "meter";
}

mx::DocumentPtr loadStandardLibraries(mx::GenContext& context)
{
    mx::DocumentPtr stdLib;
    mx::LinearUnitConverterPtr _distanceUnitConverter;
    mx::StringVec _distanceUnitOptions;
    mx::UnitConverterRegistryPtr unitRegistry;
    mx::FilePathVec libraryFolders = { "libraries" };
    mx::FileSearchPath searchPath;
    searchPath.append("/");

    // Initialize the standard library.
    try
    {
        stdLib = mx::createDocument();
        mx::StringSet _xincludeFiles = mx::loadLibraries(libraryFolders, searchPath, stdLib);
        if (_xincludeFiles.empty())
        {
            std::cerr << "Could not find standard data libraries on the given search path: " << searchPath.asString() << std::endl;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Failed to load standard data libraries: " << e.what() << std::endl;
        return nullptr;
    }

    // Initialize unit management.
    mx::UnitTypeDefPtr distanceTypeDef = stdLib->getUnitTypeDef("distance");
    _distanceUnitConverter = mx::LinearUnitConverter::create(distanceTypeDef);
    unitRegistry->addUnitConverter(distanceTypeDef, _distanceUnitConverter);
    mx::UnitTypeDefPtr angleTypeDef = stdLib->getUnitTypeDef("angle");
    mx::LinearUnitConverterPtr angleConverter = mx::LinearUnitConverter::create(angleTypeDef);
    unitRegistry->addUnitConverter(angleTypeDef, angleConverter);

    // Create the list of supported distance units.
    auto unitScales = _distanceUnitConverter->getUnitScale();
    _distanceUnitOptions.resize(unitScales.size());
    for (auto unitScale : unitScales)
    {
        int location = _distanceUnitConverter->getUnitAsInteger(unitScale.first);
        _distanceUnitOptions[location] = unitScale.first;
    }

    initContext(context,searchPath, stdLib, unitRegistry);

    return stdLib;
}

EMSCRIPTEN_BINDINGS(GenContext)
{
    ems::class_<mx::GenContext>("GenContext")
        .constructor<mx::ShaderGeneratorPtr>()
        .smart_ptr<std::shared_ptr<mx::GenContext>>("GenContextPtr")
        ;

    ems::function("loadStandardLibraries", &loadStandardLibraries);
}
