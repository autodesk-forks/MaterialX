#include <MaterialXGenShader/ColorManagementSystem.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

ColorManagementSystem::ColorManagementSystem(ShaderGenerator& shadergen, const string& configFile, const string& language)
    : _shadergen(shadergen)
    , _configFile(configFile)
    , _language(language)
{
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

void  ColorManagementSystem::setConfigFile(const string& configFile)
{
    _configFile = configFile;
    _implFactory.unregisterClasses();
    _cachedImpls.clear();
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
    ImplementationPtr impl = getImplementation(implName);
    // Try creating a new shader implementation in the factory
    ShaderNodeImplPtr shaderImpl = _implFactory.create(implName);
    if (!shaderImpl)
    {
        // Fall back to the default implementation
        shaderImpl = SourceCodeNode::create();
    }
    if (impl)
    {
        shaderImpl->initialize(impl, _shadergen);
    }
    ShaderNodePtr shaderNode = ShaderNode::createColorTransformNode(getShaderNodeName(transform), shaderImpl, transform.type, _shadergen);
    _cachedImpls[transform] = shaderNode;

    return shaderNode;
}

string ColorManagementSystem::getShaderNodeName(const ColorSpaceTransform& transform)
{
    if (transform.type)
    {
        return transform.sourceSpace + "_to_" + transform.targetSpace + "_" + transform.type->getName();
    }
    else
    {
        return "";
    }
}

} // namespace MaterialX
