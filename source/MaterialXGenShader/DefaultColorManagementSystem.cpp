#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

const string DefaultColorManagementSystem::CMS_NAME = "default_cms";


string DefaultColorManagementSystem::getImplementationName(const ColorSpaceTransform& transform)
{
    string language = _shadergen.getLanguage();
    std::replace(language.begin(), language.end(), '-', '_');
    return "IM_" + transform.sourceSpace + "_to_" + transform.targetSpace + "_" + transform.type->getName() + "_" + language;
}

DefaultColorManagementSystemPtr DefaultColorManagementSystem::create(ShaderGenerator& shadergen)
{
    DefaultColorManagementSystemPtr result(new DefaultColorManagementSystem(shadergen));
    return result;
}

DefaultColorManagementSystem::DefaultColorManagementSystem(ShaderGenerator& shadergen)
    : ColorManagementSystem(shadergen, MaterialX::EMPTY_STRING)
{
}

} // namespace MaterialX
