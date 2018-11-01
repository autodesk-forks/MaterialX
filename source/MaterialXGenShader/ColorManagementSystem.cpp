#include <MaterialXGenShader/ColorManagementSystem.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

const string DefaultColorManagementSystem::CMS_NAME = "default_cms";
const string OCIOColorManagementSystem::CMS_NAME = "ocio";


ColorManagementSystem::ColorManagementSystem(DocumentPtr document, ShaderGenerator& shadergen, const string& configFile)
    : _document(document)
    , _shadergen(shadergen)
    , _configFile(configFile)
{
}

string ColorManagementSystem::getShaderNodeName(const ColorSpaceTransform& transform)
{
    if (transform.type)
    {
        return transform.sourceSpace + "_to_" + transform.targetSpace + "_" + transform.type->getName() + "_sx-glsl";
    }
    else
    {
        return "";
    }
}

bool ColorManagementSystem::registerImplementation(const ColorSpaceTransform& transform, CreatorFunction<ShaderNodeImpl> creator)
{
    if (transform.type)
    {
        string implName = getImplementationName(transform);
        _implFactory.registerClass(implName, creator);
        return true;
    }
    return false;
}

ShaderNodePtr ColorManagementSystem::createNode(const ColorSpaceTransform& transform)
{
    // Check if it's created already
    auto it = _cachedImpls.find(transform);
    if (it != _cachedImpls.end())
    {
        return it->second;
    }

    if (transform.type == nullptr)
    {
        return nullptr;
    }

    string implName = getImplementationName(transform);
    ImplementationPtr impl = _document->getImplementation(implName);
    // Try creating a new shader implementation in the factory
    ShaderNodeImplPtr shaderImpl = _implFactory.create(implName);
    if (!shaderImpl)
    {
        // Fall back to the default implementation
        shaderImpl = SourceCodeNode::create();
    }
    shaderImpl->initialize(impl, _shadergen);
    ShaderNodePtr shaderNode = ShaderNode::create(getShaderNodeName(transform), shaderImpl, *transform.type, _shadergen);
    _cachedImpls[transform] = shaderNode;

    return shaderNode;
}

void DefaultColorManagementSystem::initialize()
{
    // TODO: Look into caching the ShaderNodeImpls for the DefaultColorManagementSystem
    _implFactory.unregisterClasses();
    _cachedImpls.clear();
}

string DefaultColorManagementSystem::getImplementationName(const ColorSpaceTransform& transform)
{
    if (transform.type)
    {
        return "IM_" + transform.sourceSpace + "_to_" + transform.targetSpace + "_" + transform.type->getName() + "_sx-glsl";
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

OCIOColorManagementSystemPtr OCIOColorManagementSystem::create(DocumentPtr document, ShaderGenerator& shadergen, const string& configFile)
{
    if (configFile.empty())
    {
        throw ExceptionShaderGenError("The " + OCIOColorManagementSystem::CMS_NAME + " color management system requires a config file to be specified.");
    }
    else
    {
        OCIOColorManagementSystemPtr result(new OCIOColorManagementSystem(document, shadergen, configFile));
        result->initialize();
        return result;
    }
}

OCIOColorManagementSystem::OCIOColorManagementSystem(DocumentPtr document, ShaderGenerator& shadergen, const string& configFile)
    : ColorManagementSystem(document, shadergen, configFile)
{
    // TODO: Create implementation programatically here: <implementation name="IM_colorSpace_color3" nodedef="ND_colorSpace_color3" language="glsl"/>
    // TODO: Register OCIO implementation here: registerImplementation("IM_colorSpace_color3", OCIOImpl::create());
}

string OCIOColorManagementSystem::getImplementationName(const ColorSpaceTransform& transform)
{
    if (transform.type)
    {
        return "IM_" + transform.sourceSpace + "_to_" + transform.targetSpace + "_" + transform.type->getName() + "_sx-glsl";
    }
    else
    {
        return "";
    }
}

} // namespace MaterialX
