#include <MaterialXGenShader/ColorManagementSystem.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

ColorSpaceTransform::ColorSpaceTransform(const string& ss, const string& ts, const TypeDesc& t)
    : sourceSpace(ss)
    , targetSpace(ts)
    , type(&t)
{
    if (type != Type::COLOR3 && type != Type::COLOR4)
    {
        throw ExceptionShaderGenError("Color space transform can only be a color3 or color4, '" + type->getName() + "' requested.");
    }
}


ColorManagementSystem::ColorManagementSystem(ShaderGenerator& shadergen, const string& configFile)
    : _configFile(configFile)
    , _shadergen(shadergen)
{
}

bool ColorManagementSystem::registerImplementation(const ColorSpaceTransform& transform, CreatorFunction<ShaderNodeImpl> creator)
{
    if (transform.type)
    {
        string implName = getImplementationName(transform);
        _implFactory.registerClass(implName, creator);
        _registeredImplNames.push_back(implName);
        return true;
    }
    return false;
}

void  ColorManagementSystem::setConfigFile(const string& configFile)
{
    _configFile = configFile;
    _implFactory.unregisterClasses(_registeredImplNames);
    _registeredImplNames.clear();
    _cachedImpls.clear();
}

void ColorManagementSystem::loadLibrary(DocumentPtr document)
{
    _document = document;
    _implFactory.unregisterClasses(_registeredImplNames);
    _registeredImplNames.clear();
    _cachedImpls.clear();
}

ShaderNodePtr ColorManagementSystem::createNode(const ColorSpaceTransform& transform, const string& prefix)
{
    string implName = getImplementationName(transform);
    ImplementationPtr impl = _document->getImplementation(implName);

    // Check if the shader implementation has been created already
    ShaderNodeImplPtr shaderImpl;
    auto it = _cachedImpls.find(transform);
    if (it != _cachedImpls.end())
    {
        shaderImpl = it->second;
    }
    // If not, try creating a new shader implementation in the factory
    else
    {
        shaderImpl = _implFactory.create(implName);
    }
    // Fall back to the default implementation
    if (!shaderImpl)
    {
        shaderImpl = SourceCodeNode::create();
    }
    if (impl)
    {
        shaderImpl->initialize(impl, _shadergen);
    }
    _cachedImpls[transform] = shaderImpl;
    ShaderNodePtr shaderNode = ShaderNode::createColorTransformNode(getShaderNodeName(transform, prefix), shaderImpl, transform.type, _shadergen);

    return shaderNode;
}

string ColorManagementSystem::getShaderNodeName(const ColorSpaceTransform& transform, const string& prefix)
{
    return prefix + "_" + transform.sourceSpace + "_to_" + transform.targetSpace + "_" + transform.type->getName();
}

} // namespace MaterialX
