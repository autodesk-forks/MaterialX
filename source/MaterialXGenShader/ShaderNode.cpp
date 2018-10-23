#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

#include <MaterialXFormat/File.h>

#include <iostream>
#include <sstream>
#include <stack>

namespace MaterialX
{

void ShaderInput::makeConnection(ShaderOutput* src)
{
    this->connection = src;
    src->connections.insert(this);
}

void ShaderInput::breakConnection()
{
    if (this->connection)
    {
        this->connection->connections.erase(this);
        this->connection = nullptr;
    }
}

void ShaderOutput::makeConnection(ShaderInput* dst)
{
    dst->connection = this;
    this->connections.insert(dst);
}

void ShaderOutput::breakConnection(ShaderInput* dst)
{
    this->connections.erase(dst);
    dst->connection = nullptr;
}

void ShaderOutput::breakConnection()
{
    for (ShaderInput* input : this->connections)
    {
        input->connection = nullptr;
    }
    this->connections.clear();
}

namespace
{
    ShaderNodePtr createEmptyNode()
    {
        ShaderNodePtr node = std::make_shared<ShaderNode>("");
        node->addContextID(ShaderGenerator::CONTEXT_DEFAULT);
        return node;
    }
}

const ShaderNodePtr ShaderNode::NONE = createEmptyNode();

const string ShaderNode::SXCLASS_ATTRIBUTE = "sxclass";
const string ShaderNode::CONSTANT = "constant";
const string ShaderNode::IMAGE = "image";
const string ShaderNode::COMPARE = "compare";
const string ShaderNode::SWITCH = "switch";
const string ShaderNode::BSDF_R = "R";
const string ShaderNode::BSDF_T = "T";

bool ShaderNode::referencedConditionally() const
{
    if (_scopeInfo.type == ShaderNode::ScopeInfo::Type::SINGLE)
    {
        int numBranches = 0;
        uint32_t mask = _scopeInfo.conditionBitmask;
        for (; mask != 0; mask >>= 1)
        {
            if (mask & 1)
            {
                numBranches++;
            }
        }
        return numBranches > 0;
    }
    return false;
}

void ShaderNode::ScopeInfo::adjustAtConditionalInput(ShaderNode* condNode, int branch, const uint32_t fullMask)
{
    if (type == ScopeInfo::Type::GLOBAL || (type == ScopeInfo::Type::SINGLE && conditionBitmask == fullConditionMask))
    {
        type = ScopeInfo::Type::SINGLE;
        conditionalNode = condNode;
        conditionBitmask = 1 << branch;
        fullConditionMask = fullMask;
    }
    else if (type == ScopeInfo::Type::SINGLE)
    {
        type = ScopeInfo::Type::MULTIPLE;
        conditionalNode = nullptr;
    }
}

void ShaderNode::ScopeInfo::merge(const ScopeInfo &fromScope)
{
    if (type == ScopeInfo::Type::UNKNOWN || fromScope.type == ScopeInfo::Type::GLOBAL)
    {
        *this = fromScope;
    }
    else if (type == ScopeInfo::Type::GLOBAL)
    {

    }
    else if (type == ScopeInfo::Type::SINGLE && fromScope.type == ScopeInfo::Type::SINGLE && conditionalNode == fromScope.conditionalNode)
    {
        conditionBitmask |= fromScope.conditionBitmask;

        // This node is needed for all branches so it is no longer conditional
        if (conditionBitmask == fullConditionMask)
        {
            type = ScopeInfo::Type::GLOBAL;
            conditionalNode = nullptr;
        }
    }
    else
    {
        // NOTE: Right now multiple scopes is not really used, it works exactly as ScopeInfo::Type::GLOBAL
        type = ScopeInfo::Type::MULTIPLE;
        conditionalNode = nullptr;
    }
}

ShaderNode::ShaderNode(const string& name)
    : _name(name)
    , _classification(0)
    , _samplingInput(nullptr)
    , _impl(nullptr)
{
}

static bool elementCanBeSampled2D(const Element& element)
{
    const string TEXCOORD_NAME("texcoord");
    return (element.getName() == TEXCOORD_NAME);
}

static bool elementCanBeSampled3D(const Element& element)
{
    const string POSITION_NAME("position");
    return (element.getName() == POSITION_NAME);
}

//
// Given a nodedef and corresponding implementation, return the
// implementation value if any for a value.
//
// An implementation value will be returned if:
// - There is a implementation Parametner with the same name as the input Value 
// - There is a nodedef Value with the same name as the input Value 
// - There is a enumeration and type specified on the implementation Parameter 
// - There is a enumeration and type specified on the nodedef Value
//
ValuePtr getImplementationValue(const ValueElementPtr& elem, const InterfaceElementPtr impl, const NodeDef& nodeDef)
{
    const string& valueElementName = elem->getName();
    const string& valueString = elem->getValueString();
    if (valueString.empty())
    {
        return nullptr;
    }

    ParameterPtr implParam = impl->getParameter(valueElementName);
    if (!implParam)
    {
        return elem->getValue();
    }
 
    ValueElementPtr nodedefElem = nodeDef.getChildOfType<ValueElement>(valueElementName);
    if (!nodedefElem)
    {
        return elem->getValue();
    }

    const string& implType = implParam->getAttribute(ValueElement::IMPLEMENTATION_TYPE_ATTRIBUTE);
    const string& implEnums = implParam->getAttribute(ValueElement::ENUM_VALUES_ATTRIBUTE);
    if (implType.empty() || implEnums.empty())
    {
        return elem->getValue();
    }

    const string nodedefElemEnums = nodedefElem->getAttribute(ValueElement::ENUM_ATTRIBUTE);
    if (nodedefElemEnums.empty())
    {
        return elem->getValue();
    }

    // Find the list index of the Value string in list fo nodedef enums.
    // Use this index to lookup the implementation list value.
    int implIndex = -1;
    StringVec implEnumsVec = splitString(implEnums, ",");
    StringVec nodedefElemEnumsVec = splitString(nodedefElemEnums, ",");
    if (nodedefElemEnumsVec.size() == implEnumsVec.size())
    {
        auto pos = std::find(nodedefElemEnumsVec.begin(), nodedefElemEnumsVec.end(), valueString);
        if (pos != nodedefElemEnumsVec.end())
        {
            implIndex = static_cast<int>(std::distance(nodedefElemEnumsVec.begin(), pos));
        }
    }
    // There is no mapping so just choose the first implementation list string.
    if (implIndex < 0)
    {
        implIndex = 0;
    }
    return Value::createValueFromStrings(implEnumsVec[implIndex], implType);
}

ShaderNodePtr ShaderNode::create(const string& name, const NodeDef& nodeDef, ShaderGenerator& shadergen, const Node* nodeInstance)
{
    ShaderNodePtr newNode = std::make_shared<ShaderNode>(name);

    // Find the implementation for this nodedef
    InterfaceElementPtr impl = nodeDef.getImplementation(shadergen.getTarget(), shadergen.getLanguage());
    ShaderNodeImplPtr a = nullptr;
    if (impl)
    {
        newNode->_impl = shadergen.getImplementation(impl);
    }
    if (!newNode->_impl)
    {
        throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef.getNodeString() +
            "' matching language '" + shadergen.getLanguage() + "' and target '" + shadergen.getTarget() + "'");
    }

    // Check for classification based on group name
    unsigned int groupClassification = 0;
    const string TEXTURE2D_GROUPNAME("texture2d");
    const string TEXTURE3D_GROUPNAME("texture3d");
    const string PROCEDURAL2D_GROUPNAME("procedural2d");
    const string PROCEDURAL3D_GROUPNAME("procedural3d");
    const string CONVOLUTION2D_GROUPNAME("convolution2d");
    string groupName = nodeDef.getNodeGroup();
    if (!groupName.empty())
    {
        if (groupName == TEXTURE2D_GROUPNAME || groupName == PROCEDURAL2D_GROUPNAME)
        {
            groupClassification = Classification::SAMPLE2D;
        }
        else if (groupName == TEXTURE3D_GROUPNAME || groupName == PROCEDURAL3D_GROUPNAME)
        {
            groupClassification = Classification::SAMPLE3D;
        }
        else if (groupName == CONVOLUTION2D_GROUPNAME)
        {
            groupClassification = Classification::CONVOLUTION2D;
        }
    }
    newNode->_samplingInput = nullptr;

    // Create interface from nodedef
    const vector<ValueElementPtr> nodeDefInputs = nodeDef.getChildrenOfType<ValueElement>();
    for (const ValueElementPtr& elem : nodeDefInputs)
    {
        if (elem->isA<Output>())
        {
            newNode->addOutput(elem->getName(), TypeDesc::get(elem->getType()));
        }
        else
        {
            ParameterPtr implParam = impl->getParameter(elem->getName());
            const string& implType = implParam ? implParam->getAttribute(ValueElement::IMPLEMENTATION_TYPE_ATTRIBUTE) : EMPTY_STRING;

            const string& elemType = elem->getType();
            const TypeDesc* typeDesc = TypeDesc::get(elemType);
            const TypeDesc* remapTypeDesc = nullptr;
            const string implEnums = implParam ? implParam->getAttribute(ValueElement::ENUM_VALUES_ATTRIBUTE) : EMPTY_STRING;
            const string elemEnums = elem->getAttribute(ValueElement::ENUM_ATTRIBUTE);
            if (!implType.empty() && implType != elemType && !implEnums.empty() && !elemEnums.empty())
            {
                remapTypeDesc = TypeDesc::get(implType);
            }

            ShaderInput* input = nullptr;
            const string& elemValueString = elem->getValueString();
            StringVec implEnumsVec = splitString(implEnums, ",");
            int implIndex = -1;
            if (remapTypeDesc)
            {
                input = newNode->addInput(elem->getName(), remapTypeDesc);
                if (!elemValueString.empty())
                {
                    StringVec elemEnumsVec = splitString(elemEnums, ",");
                    if (elemEnumsVec.size() == implEnumsVec.size())
                    {
                        auto pos = std::find(elemEnumsVec.begin(), elemEnumsVec.end(), elemValueString);
                        if (pos != elemEnumsVec.end())
                        {
                            implIndex = static_cast<int>(std::distance(elemEnumsVec.begin(), pos));
                        }
                    }
                    if (implIndex < 0)
                    {
                        implIndex = 0;
                    }
                }

                input->value = Value::createValueFromStrings(implEnumsVec[implIndex], implType);
            }
            if (!input)
            {
                input = newNode->addInput(elem->getName(), typeDesc);
                if (!elemValueString.empty())
                {
                    input->value = elem->getValue();
                }
            }

            // Determine if this input can be sampled
            if ((groupClassification == Classification::SAMPLE2D && elementCanBeSampled2D(*elem)) ||
                (groupClassification == Classification::SAMPLE3D && elementCanBeSampled3D(*elem)))
            {
                newNode->_samplingInput = input;
            }
        }
    }

    // Add a default output if needed
    if (newNode->numOutputs() == 0)
    {
        newNode->addOutput("out", TypeDesc::get(nodeDef.getType()));
    }

    // Assign input values from the node instance
    if (nodeInstance)
    {
        const vector<ValueElementPtr> nodeInstanceInputs = nodeInstance->getChildrenOfType<ValueElement>();
        for (const ValueElementPtr& elem : nodeInstanceInputs)
        {
            const string& elemValueString = elem->getValueString();
            if (!elemValueString.empty())
            {
                ShaderInput* input = newNode->getInput(elem->getName());
                if (input && !elemValueString.empty())
                {
                    input->value = getImplementationValue(elem, impl, nodeDef);
                }
            }
        }
    }

    //
    // Set node classification, defaulting to texture node
    //
    newNode->_classification = Classification::TEXTURE;

    // First, check for specific output types
    const ShaderOutput* primaryOutput = newNode->getOutput();
    if (primaryOutput->type == Type::SURFACESHADER)
    {
        newNode->_classification = Classification::SURFACE | Classification::SHADER;
    }
    else if (primaryOutput->type == Type::LIGHTSHADER)
    {
        newNode->_classification = Classification::LIGHT | Classification::SHADER;
    }
    else if (primaryOutput->type == Type::BSDF)
    {
        newNode->_classification = Classification::BSDF | Classification::CLOSURE;

        // Add additional classifications if the BSDF is restricted to
        // only reflection or transmission
        const string& bsdfType = nodeDef.getAttribute("bsdf");
        if (bsdfType == BSDF_R)
        {
            newNode->_classification |= Classification::BSDF_R;
        }
        else if (bsdfType == BSDF_T)
        {
            newNode->_classification |= Classification::BSDF_T;
        }
    }
    else if (primaryOutput->type == Type::EDF)
    {
        newNode->_classification = Classification::EDF | Classification::CLOSURE;
    }
    else if (primaryOutput->type == Type::VDF)
    {
        newNode->_classification = Classification::VDF | Classification::CLOSURE;
    }
    // Second, check for specific nodes types
    else if (nodeDef.getNodeString() == CONSTANT)
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONSTANT;
    }
    else if (nodeDef.getNodeString() == IMAGE || nodeDef.getAttribute(SXCLASS_ATTRIBUTE) == IMAGE)
    {
        newNode->_classification = Classification::TEXTURE | Classification::FILETEXTURE;
    }
    else if (nodeDef.getNodeString() == COMPARE)
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONDITIONAL | Classification::IFELSE;
    }
    else if (nodeDef.getNodeString() == SWITCH)
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONDITIONAL | Classification::SWITCH;
    }

    // Add in group classification
    newNode->_classification |= groupClassification;

    // Let the shader generator assign in which contexts to use this node
    shadergen.addNodeContextIDs(newNode.get());

    return newNode;
}

ShaderInput* ShaderNode::getInput(const string& name)
{
    auto it = _inputMap.find(name);
    return it != _inputMap.end() ? it->second.get() : nullptr;
}

ShaderOutput* ShaderNode::getOutput(const string& name)
{
    auto it = _outputMap.find(name);
    return it != _outputMap.end() ? it->second.get() : nullptr;
}

const ShaderInput* ShaderNode::getInput(const string& name) const
{
    auto it = _inputMap.find(name);
    return it != _inputMap.end() ? it->second.get() : nullptr;
}

const ShaderOutput* ShaderNode::getOutput(const string& name) const
{
    auto it = _outputMap.find(name);
    return it != _outputMap.end() ? it->second.get() : nullptr;
}

ShaderInput* ShaderNode::addInput(const string& name, const TypeDesc* type)
{
    if (getInput(name))
    {
        throw ExceptionShaderGenError("An input named '" + name + "' already exists on node '" + _name + "'");
    }

    ShaderInputPtr input = std::make_shared<ShaderInput>();
    input->name = name;
    input->type = type;
    input->node = this;
    input->value = nullptr;
    input->connection = nullptr;
    _inputMap[name] = input;
    _inputOrder.push_back(input.get());

    return input.get();
}

ShaderOutput* ShaderNode::addOutput(const string& name, const TypeDesc* type)
{
    if (getOutput(name))
    {
        throw ExceptionShaderGenError("An output named '" + name + "' already exists on node '" + _name + "'");
    }

    ShaderOutputPtr output = std::make_shared<ShaderOutput>();
    output->name = name;
    output->type = type;
    output->node = this;
    _outputMap[name] = output;
    _outputOrder.push_back(output.get());

    return output.get();
}

void ShaderNode::renameInput(const string& name, const string& newName)
{
    if (name != newName)
    {
        auto it = _inputMap.find(name);
        if (it != _inputMap.end())
        {
            it->second->name = newName;
            _inputMap[newName] = it->second;
            _inputMap.erase(it);
        }
    }
}

void ShaderNode::renameOutput(const string& name, const string& newName)
{
    if (name != newName)
    {
        auto it = _outputMap.find(name);
        if (it != _outputMap.end())
        {
            it->second->name = newName;
            _outputMap[newName] = it->second;
            _outputMap.erase(it);
        }
    }
}

} // namespace MaterialX
