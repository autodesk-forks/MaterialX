#include <MaterialXGenShader/DefaultColorManagementSystem.h>

namespace MaterialX
{

const string DefaultColorManagementSystem::CMS_NAME = "default_cms";


string DefaultColorManagementSystem::getImplementationName(const ColorSpaceTransform& transform)
{
    if (transform.type)
    {
        return "IM_" + transform.sourceSpace + "_to_" + transform.targetSpace + "_" + transform.type->getName() + "_" + _shadergen->getLanguage();
    }
    else
    {
        return "";
    }
}

DefaultColorManagementSystemPtr DefaultColorManagementSystem::create(DocumentPtr document, ShaderGenerator& shadergen, const string& configFile)
{
    if (configFile.empty())
    {
      DefaultColorManagementSystemPtr result(new DefaultColorManagementSystem(document, shadergen));
      result->initialize();
      return result;
    }
    else
    {
        throw ExceptionShaderGenError("The " + DefaultColorManagementSystem::CMS_NAME + " color management system does not require a config file to be specified.");
    }
}

DefaultColorManagementSystem::DefaultColorManagementSystem(DocumentPtr document, ShaderGenerator& shadergen)
    : ColorManagementSystem(document, shadergen, MaterialX::EMPTY_STRING)
{
}

} // namespace MaterialX
