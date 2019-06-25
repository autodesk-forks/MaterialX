#include "MaterialXNode.h"
#include "Plugin.h"
#include "Util.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MStringArray.h>
#include <maya/MPlugArray.h>
#include <maya/MDGModifier.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>

#include <maya/MRenderUtil.h>
#include <maya/MFloatVector.h>

#define MAKE_INPUT(attr) \
	CHECK_MSTATUS(attr.setKeyable(true)); \
	CHECK_MSTATUS(attr.setStorable(true)); \
	CHECK_MSTATUS(attr.setReadable(true)); \
	CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_OUTPUT(attr) \
	CHECK_MSTATUS(attr.setKeyable(false)); \
	CHECK_MSTATUS(attr.setStorable(false)); \
	CHECK_MSTATUS(attr.setReadable(true)); \
	CHECK_MSTATUS(attr.setWritable(false));

const MTypeId MaterialXNode::MATERIALX_NODE_TYPEID(0x00042402);
const MString MaterialXNode::MATERIALX_NODE_TYPENAME("MaterialXNode");

MString MaterialXNode::DOCUMENT_ATTRIBUTE_LONG_NAME("document");
MString MaterialXNode::DOCUMENT_ATTRIBUTE_SHORT_NAME("doc");
MObject MaterialXNode::DOCUMENT_ATTRIBUTE;

MString MaterialXNode::ELEMENT_ATTRIBUTE_LONG_NAME("element");
MString MaterialXNode::ELEMENT_ATTRIBUTE_SHORT_NAME("ele");
MObject MaterialXNode::ELEMENT_ATTRIBUTE;

const MTypeId MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPEID(0x00042403);
const MString MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPENAME("MaterialXTextureNode");

const MTypeId MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPEID(0x00042404);
const MString MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPENAME("MaterialXSurfaceNode");

MaterialXNode::MaterialXNode()
{
	std::cout << "MaterialXNode::MaterialXNode" << std::endl;
}

MaterialXNode::~MaterialXNode()
{
	std::cout << "MaterialXNode::~MaterialXNode" << std::endl;
}

void* MaterialXNode::creator()
{
	std::cout.rdbuf(std::cerr.rdbuf());
	std::cout << "MaterialXNode::creator" << std::endl;
	return new MaterialXNode();
}

MStatus MaterialXNode::initialize()
{
	std::cout << "MaterialXNode::initialize" << std::endl;

	MFnTypedAttribute typedAttr;
	MFnStringData stringData;

	MObject theString = stringData.create();
	DOCUMENT_ATTRIBUTE = typedAttr.create(DOCUMENT_ATTRIBUTE_LONG_NAME, DOCUMENT_ATTRIBUTE_SHORT_NAME, MFnData::kString, theString);
	CHECK_MSTATUS(typedAttr.setStorable(true));
	CHECK_MSTATUS(typedAttr.setReadable(true));
	CHECK_MSTATUS(typedAttr.setInternal(true));
	CHECK_MSTATUS(typedAttr.setCached(false));
	CHECK_MSTATUS(typedAttr.setHidden(true));
	CHECK_MSTATUS(addAttribute(DOCUMENT_ATTRIBUTE));

	ELEMENT_ATTRIBUTE = typedAttr.create(ELEMENT_ATTRIBUTE_LONG_NAME, ELEMENT_ATTRIBUTE_SHORT_NAME, MFnData::kString, theString);
	CHECK_MSTATUS(typedAttr.setStorable(true));
	CHECK_MSTATUS(typedAttr.setReadable(true));
	CHECK_MSTATUS(typedAttr.setInternal(true));
	CHECK_MSTATUS(typedAttr.setCached(false));
	CHECK_MSTATUS(typedAttr.setHidden(true));
	CHECK_MSTATUS(addAttribute(ELEMENT_ATTRIBUTE));

	return MS::kSuccess;
}

void MaterialXNode::createOutputAttr(MDGModifier& mdgModifier)
{
    if (materialXData && !materialXData->getElementPath().empty())
    {
        const MString outputName(MaterialX::OgsXmlGenerator::OUTPUT_NAME.c_str());
        MFnNumericAttribute nAttr;
        _outAttr = nAttr.createColor(outputName, outputName);
        CHECK_MSTATUS(nAttr.setStorable(false));
        CHECK_MSTATUS(nAttr.setInternal(false));
        CHECK_MSTATUS(nAttr.setReadable(true));
        CHECK_MSTATUS(nAttr.setWritable(false));
        CHECK_MSTATUS(nAttr.setCached(true));
        CHECK_MSTATUS(nAttr.setHidden(false));

        mdgModifier.addAttribute(thisMObject(), _outAttr);
    }
}

MStatus MaterialXNode::setDependentsDirty(const MPlug &/*plugBeingDirtied*/, MPlugArray & affectedPlugs)
{
	if (!_outAttr.isNull())
	{
		MPlug outPlug(thisMObject(), _outAttr);
		affectedPlugs.append(outPlug);
	}

	return MS::kSuccess;
}

MTypeId MaterialXNode::typeId() const
{
    return MATERIALX_NODE_TYPEID;
}

MPxNode::SchedulingType MaterialXNode::schedulingType() const
{
	return MPxNode::SchedulingType::kParallel;
}

bool MaterialXNode::setInternalValue(const MPlug &plug, const MDataHandle &dataHandle)
{
	if (!materialXData) return false;

	if (plug == DOCUMENT_ATTRIBUTE)
	{
		//MString documentContent = dataHandle.asString();
		//materialXData->createDocument(documentContent.asChar());
	}
	else if (plug == ELEMENT_ATTRIBUTE)
	{
		if (materialXData->getDocument())
		{
			//MString elementPath = dataHandle.asString();
			//materialXData->element = materialXData->doc->getDescendant(elementPath.asChar());
		}
	}
	else if (plug == _outAttr)
	{
		return MPxNode::setInternalValue(plug, dataHandle);
	}
	else
	{
		std::string plugName = plug.partialName().asChar();
		if (plug.isChild())
		{
			MPlug plug2 = plug.parent();
			plugName = plug2.partialName().asChar();
		}
		if (_attributeElementPairMap.count(plugName) > 0)
		{
			MaterialX::ElementPtr element = _attributeElementPairMap[plugName];
			if (element->isA<MaterialX::ValueElement>())
			{
				MaterialX::ValueElementPtr valueElement = element->asA<MaterialX::ValueElement>();
				std::string type = valueElement->getType();
				if (type == MaterialX::TypedValue<MaterialX::Vector2>::TYPE)
				{
                    float2& value = dataHandle.asFloat2();
                    valueElement->setValue(MaterialX::Vector2(value[0], value[1]));
				}
				else if (type == MaterialX::TypedValue<MaterialX::Vector3>::TYPE)
				{
                    float3& value = dataHandle.asFloat3();
                    valueElement->setValue(MaterialX::Vector3(value[0], value[1], value[2]));
				}
				else if (type == MaterialX::TypedValue<MaterialX::Vector4>::TYPE)
				{
                    MFloatVector& value = dataHandle.asFloatVector();
                    valueElement->setValue(MaterialX::Vector4(value[0], value[1], value[2], value[3]));
				}
				else if (type == MaterialX::TypedValue<MaterialX::Color2>::TYPE)
				{
                    float2& value = dataHandle.asFloat2();
                    valueElement->setValue(MaterialX::Color2(value[0], value[1]));
				}
				else if (type == MaterialX::TypedValue<MaterialX::Color3>::TYPE)
				{
					float3& value = dataHandle.asFloat3();
					valueElement->setValue(MaterialX::Color3(value[0], value[1], value[2]));
				}
				else if (type == MaterialX::TypedValue<MaterialX::Color4>::TYPE)
				{
					MFloatVector& value = dataHandle.asFloatVector();
                    valueElement->setValue(MaterialX::Color4(value[0], value[1], value[2], value[3]));
				}
				else if (type == MaterialX::TypedValue<float>::TYPE)
				{
					float& value = dataHandle.asFloat();
					valueElement->setValue(value);
				}
			}
		}
	}
	return true;
}

void MaterialXNode::setAttributeValue(MObject &materialXObject, MObject &attr, float* values, unsigned int size, MDGModifier& mdgModifier)
{
	MPlug plug(materialXObject, attr);
	if (size == 1)
	{
		mdgModifier.newPlugValueDouble(plug, values[0]);
	}
	else
	{
		for(unsigned int i=0; i<size; i++)
		{
			MPlug indexPlug = plug.child(i);
			mdgModifier.newPlugValueDouble(indexPlug, values[i]);
		}
	}
}

void MaterialXNode::createAttributesFromDocument(MDGModifier& mdgModifier)
{
    MaterialX::DocumentPtr document;
    if (!materialXData || !(document = materialXData->getDocument()))
    {
        return;
    }

	const MaterialX::StringMap& inputMap = materialXData->getPathInputMap();
	for (auto it = inputMap.begin(); it != inputMap.end(); ++it)
	{
		MaterialX::ElementPtr element = document->getDescendant(it->first);
		if (!element) continue;
		MObject mobject = thisMObject();
		if (element->isA<MaterialX::ValueElement>())
		{
			MaterialX::ValueElementPtr valueElement = element->asA<MaterialX::ValueElement>();
			std::string name = valueElement->getName();
			std::string type = valueElement->getType();
			MFnNumericAttribute numericAttr;
			MObject attr;
			if (type == MaterialX::TypedValue<MaterialX::Vector2>::TYPE)
			{
				attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k2Double, 0.0);
				mdgModifier.addAttribute(mobject, attr);
				MaterialX::Vector2 value = valueElement->getValue()->asA<MaterialX::Vector2>();
				setAttributeValue(mobject, attr, value.data(), 2, mdgModifier);
			}
			else if (type == MaterialX::TypedValue<MaterialX::Vector3>::TYPE)
			{
				attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k3Double, 0.0);
				mdgModifier.addAttribute(mobject, attr);
				MaterialX::Vector3 value = valueElement->getValue()->asA<MaterialX::Vector3>();
				setAttributeValue(mobject, attr, value.data(), 3, mdgModifier);
			}
			else if (type == MaterialX::TypedValue<MaterialX::Vector4>::TYPE)
			{
				attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k4Double, 0.0);
				mdgModifier.addAttribute(mobject, attr);
				MaterialX::Vector4 value = valueElement->getValue()->asA<MaterialX::Vector4>();
				setAttributeValue(mobject, attr, value.data(), 4, mdgModifier);
			}
			else if (type == MaterialX::TypedValue<MaterialX::Color2>::TYPE)
			{
				attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k2Double, 0.0);
				mdgModifier.addAttribute(mobject, attr);
				MaterialX::Color2 value = valueElement->getValue()->asA<MaterialX::Color2>();
				setAttributeValue(mobject, attr, value.data(), 2, mdgModifier);
			}
			else if (type == MaterialX::TypedValue<MaterialX::Color3>::TYPE)
			{
				attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k3Double, 0.0);
				mdgModifier.addAttribute(mobject, attr);
				MaterialX::Color3 value = valueElement->getValue()->asA<MaterialX::Color3>();
				setAttributeValue(mobject, attr, value.data(), 3, mdgModifier);
			}
			else if (type == MaterialX::TypedValue<MaterialX::Color4>::TYPE)
			{
				attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k4Double, 0.0);
				mdgModifier.addAttribute(mobject, attr);
				MaterialX::Color4 value = valueElement->getValue()->asA<MaterialX::Color4>();
				setAttributeValue(mobject, attr, value.data(), 4, mdgModifier);
			}
			else if (type == MaterialX::TypedValue<float>::TYPE)
			{
				attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::kDouble, 0.0);
				mdgModifier.addAttribute(mobject, attr);
				float value = valueElement->getValue()->asA<float>();
				setAttributeValue(mobject, attr, &value, 1, mdgModifier);
			}
			numericAttr.setStorable(true);
			numericAttr.setReadable(true);
			numericAttr.setInternal(true);
			numericAttr.setCached(false);
			//			numericAttr.setHidden(true);

			MPlug plug(mobject, attr);
			_attributeElementPairMap[plug.partialName().asChar()] = element;
		}
	}
}

MTypeId MaterialXTextureNode::typeId() const
{
    return MATERIALX_TEXTURE_NODE_TYPEID;
}

void* MaterialXTextureNode::creator()
{
    std::cout.rdbuf(std::cerr.rdbuf());
    std::cout << "MaterialXTextureNode::creator" << std::endl;
    return new MaterialXTextureNode();
}

MStatus MaterialXTextureNode::initialize()
{
    std::cout << "MaterialXTextureNode::initialize" << std::endl;

    CHECK_MSTATUS(inheritAttributesFrom(MATERIALX_NODE_TYPENAME));

    return MS::kSuccess;
}

MTypeId MaterialXSurfaceNode::typeId() const
{
    return MATERIALX_SURFACE_NODE_TYPEID;
}

void* MaterialXSurfaceNode::creator()
{
    std::cout.rdbuf(std::cerr.rdbuf());
    std::cout << "MaterialXSurfaceNode::creator" << std::endl;
    return new MaterialXSurfaceNode();
}

MStatus MaterialXSurfaceNode::initialize()
{
    std::cout << "MaterialXSurfaceNode::initialize" << std::endl;

    CHECK_MSTATUS(inheritAttributesFrom(MATERIALX_NODE_TYPENAME));

    return MS::kSuccess;
}