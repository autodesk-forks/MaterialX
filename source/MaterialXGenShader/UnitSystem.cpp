//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/UnitSystem.h>

#include <MaterialXCore/UnitConverter.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{
//
// Unit transform methods
//

UnitTransform::UnitTransform(const string& ss, const string& ts, const TypeDesc* t) :
    sourceUnit(ss),
    targetUnit(ts),
    type(t)
{
    if (type != Type::FLOAT && type != Type::VECTOR2 && type != Type::VECTOR3 && type != Type::VECTOR4)
    {
        throw ExceptionShaderGenError("Unit space transform can only be a float or vectors");
    }
}


UnitSystem::UnitSystem()
{
}

void UnitSystem::loadLibrary(DocumentPtr document)
{
    _document = document;
}

bool UnitSystem::supportsTransform(const UnitTransform& transform) const
{
    const string implName = getImplementationName(transform);
    ImplementationPtr impl = _document->getImplementation(implName);
    return impl != nullptr;
}

ShaderNodePtr UnitSystem::createNode(const ShaderGraph* parent, const UnitTransform& transform, const string& name,
    GenContext& context) const
{
    const string implName = getImplementationName(transform);
    ImplementationPtr impl = _document->getImplementation(implName);
    if (!impl)
    {
        throw ExceptionShaderGenError("No implementation found for transform: ('" + transform.sourceUnit + "', '" + transform.targetUnit + "').");
    }

    // Check if it's created and cached already,
    // otherwise create and cache it.
    ShaderNodeImplPtr nodeImpl = context.findNodeImplementation(implName);
    if (!nodeImpl)
    {
        nodeImpl = SourceCodeNode::create();
        nodeImpl->initialize(*impl, context);
        context.addNodeImplementation(implName, nodeImpl);
    }

    // Create the node.
    ShaderNodePtr shaderNode = ShaderNode::create(parent, name, nodeImpl, ShaderNode::Classification::TEXTURE);

    // Create ports on the node.
    ShaderInput* input = shaderNode->addInput("in", transform.type);
    if (transform.type == Type::FLOAT)
    {
        input->setValue(Value::createValue(1.0));
    }
    else if (transform.type == Type::VECTOR2)
    {
        input->setValue(Value::createValue(Vector2(1.0f, 1.0)));
    }
    else if (transform.type == Type::VECTOR3)
    {
        input->setValue(Value::createValue(Vector3(1.0f, 1.0, 1.0)));
    }
    else if (transform.type == Type::VECTOR4)
    {
        input->setValue(Value::createValue(Vector4(1.0f, 1.0, 1.0, 1.0)));
    }
    else
    {
        throw ExceptionShaderGenError("Invalid type specified to createColorTransform: '" + transform.type->getName() + "'");
    }

    
    // Length Unit Conversion
    UnitConverterRegistryPtr unitRegistry = UnitConverterRegistry::create();
    UnitTypeDefPtr lengthTypeDef = _document->getUnitTypeDef(LengthUnitConverter::LENGTH_UNIT);
    if (!unitRegistry->getUnitConverter(lengthTypeDef))
    {
        throw ExceptionTypeError("Undefined Unit convertor for: " + LengthUnitConverter::LENGTH_UNIT);
    }

    LengthUnitConverterPtr lengthConverter = std::dynamic_pointer_cast<LengthUnitConverter>(unitRegistry->getUnitConverter(lengthTypeDef));

    // Add the conversion code
    const std::unordered_map<std::string, float>& unitScale = lengthConverter->getUnitScale();
    {
        const auto it = unitScale.find(transform.sourceUnit);
        if (it == unitScale.end())
        {
            throw ExceptionTypeError("Unrecognized source unit: " + transform.sourceUnit);
        }

        ShaderInput* convertFrom = shaderNode->addInput("unit_from", Type::INTEGER);
        convertFrom->setValue(Value::createValue((int)std::distance(unitScale.begin(), it)));
    }

    {
        const auto it = unitScale.find(transform.targetUnit);
        if (it == unitScale.end())
        {
            throw ExceptionTypeError("Unrecognized target unit: " + transform.targetUnit);
        }

        ShaderInput* convertTo = shaderNode->addInput("unit_to", Type::INTEGER);
        convertTo->setValue(Value::createValue((int)std::distance(unitScale.begin(), it)));
    }

    shaderNode->addOutput("out", transform.type);

    return shaderNode;
}

} // namespace MaterialX
