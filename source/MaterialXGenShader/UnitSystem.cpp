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

UnitTransform::UnitTransform(const string& ss, const string& ts, const TypeDesc* t, const string& unittype) :
    sourceUnit(ss),
    targetUnit(ts),
    type(t),
    unitType(unittype)
{
    if (type != Type::FLOAT && type != Type::VECTOR2 && type != Type::VECTOR3 && type != Type::VECTOR4)
    {
        throw ExceptionShaderGenError("Unit space transform can only be a float or vectors");
    }
}

const string UnitSystem::UNITSYTEM_NAME = "default_unit_system";
const string UnitSystem::LENGTH_UNIT_TARGET_NAME = "u_lengthUnitTarget";

UnitSystem::UnitSystem(const string& language)
{
    _language = createValidName(language);
}

void UnitSystem::loadLibrary(DocumentPtr document)
{
    _document = document;
}

void UnitSystem::setUnitConverterRegistry(UnitConverterRegistryPtr registry)
{
    _unitRegistry = registry;
}

UnitConverterRegistryPtr UnitSystem::getUnitConverterRegistry() const
{
    return _unitRegistry;
}

UnitSystemPtr UnitSystem::create(const string& language)
{
    UnitSystemPtr result(new UnitSystem(language));
    return result;
}

string UnitSystem::getImplementationName(const UnitTransform& transform, const string& unitname) const
{
    return "IM_" + unitname + "_unit_" + transform.type->getName() + "_" + _language;
}

bool UnitSystem::supportsTransform(const UnitTransform& transform) const
{
    const string implName = getImplementationName(transform, LengthUnitConverter::LENGTH_UNIT);
    ImplementationPtr impl = _document->getImplementation(implName);
    return impl != nullptr;
}

ShaderNodePtr UnitSystem::createNode(ShaderGraph* parent, const UnitTransform& transform, const string& name,
    GenContext& context) const
{
    const string implName = getImplementationName(transform, LengthUnitConverter::LENGTH_UNIT);
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
        throw ExceptionShaderGenError("Invalid type specified to unitTransform: '" + transform.type->getName() + "'");
    }

    
    // Length Unit Conversion
    UnitTypeDefPtr lengthTypeDef = _document->getUnitTypeDef(LengthUnitConverter::LENGTH_UNIT);
    if (_unitRegistry && _unitRegistry->getUnitConverter(lengthTypeDef))
    {
        LengthUnitConverterPtr lengthConverter = std::dynamic_pointer_cast<LengthUnitConverter>(_unitRegistry->getUnitConverter(lengthTypeDef));

        // Add the conversion code
        {
            int value = lengthConverter->getUnitAsInteger(transform.sourceUnit);
            if (value < 0)
            {
                throw ExceptionTypeError("Unrecognized source unit: " + transform.sourceUnit);
            }

            ShaderInput* convertFrom = shaderNode->addInput("unit_from", Type::INTEGER);
            convertFrom->setValue(Value::createValue(value));
        }

        {
            int value = lengthConverter->getUnitAsInteger(transform.targetUnit);
            if (value < 0)
            {
                throw ExceptionTypeError("Unrecognized target unit: " + transform.targetUnit);
            }

            ShaderInput* convertTo = shaderNode->addInput("unit_to", Type::INTEGER);

            // Create a graph input to connect to the "unit_to" if it does not already exist.
            ShaderGraphInputSocket* globalInput = parent->getInputSocket(LENGTH_UNIT_TARGET_NAME);
            if (!globalInput)
            {
                globalInput = parent->addInputSocket(LENGTH_UNIT_TARGET_NAME, Type::INTEGER);
            }
            globalInput->setValue(Value::createValue(value));
            convertTo->makeConnection(globalInput);
        }

        shaderNode->addOutput("out", transform.type);

        return shaderNode;
    }
    else
    {
        throw ExceptionTypeError("Unit registry unavaliable or undefined Unit convertor for: " + LengthUnitConverter::LENGTH_UNIT);
    }

}

} // namespace MaterialX
