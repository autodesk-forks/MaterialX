#include "MaterialXNode.h"
#include "Plugin.h"
#include "Util.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MStringArray.h>
#include <maya/MDGModifier.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>
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
	if (materialXData)
	{
		MFnNumericAttribute nAttr;

		MString outputName = materialXData->glslFragmentWrapper->getFragmentName().c_str();
		_outAttr = nAttr.createColor(outputName, outputName);
		CHECK_MSTATUS(nAttr.setStorable(false));
		CHECK_MSTATUS(nAttr.setInternal(false));
		CHECK_MSTATUS(nAttr.setReadable(true));
		CHECK_MSTATUS(nAttr.setWritable(false));
		CHECK_MSTATUS(nAttr.setCached(true));
		CHECK_MSTATUS(nAttr.setHidden(false));

		mdgModifier.addAttribute(thisMObject(), _outAttr);
//		CHECK_MSTATUS(addAttribute(_outAttr));
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
		if (materialXData->doc)
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
					double2& value = dataHandle.asDouble2();
					valueElement->setValue(MaterialX::Vector2(static_cast<float>(value[0]), static_cast<float>(value[1])));
				}
				else if (type == MaterialX::TypedValue<MaterialX::Vector3>::TYPE)
				{
					double3& value = dataHandle.asDouble3();
					valueElement->setValue(MaterialX::Vector3(static_cast<float>(value[0]), static_cast<float>(value[1]), static_cast<float>(value[2])));
				}
				else if (type == MaterialX::TypedValue<MaterialX::Vector4>::TYPE)
				{
					double4& value = dataHandle.asDouble4();
					valueElement->setValue(MaterialX::Vector4(static_cast<float>(value[0]), static_cast<float>(value[1]), static_cast<float>(value[2]), static_cast<float>(value[3])));
				}
				else if (type == MaterialX::TypedValue<MaterialX::Color2>::TYPE)
				{
					double2& value = dataHandle.asDouble2();
					valueElement->setValue(MaterialX::Color2(static_cast<float>(value[0]), static_cast<float>(value[1])));
				}
				else if (type == MaterialX::TypedValue<MaterialX::Color3>::TYPE)
				{
					double3& value = dataHandle.asDouble3();
					valueElement->setValue(MaterialX::Color3(static_cast<float>(value[0]), static_cast<float>(value[1]), static_cast<float>(value[2])));
				}
				else if (type == MaterialX::TypedValue<MaterialX::Color4>::TYPE)
				{
					double4& value = dataHandle.asDouble4();
					valueElement->setValue(MaterialX::Color4(static_cast<float>(value[0]), static_cast<float>(value[1]), static_cast<float>(value[2]), static_cast<float>(value[3])));
				}
				else if (type == MaterialX::TypedValue<float>::TYPE)
				{
					double& value = dataHandle.asDouble();
					valueElement->setValue(static_cast<float>(value));
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
	if (!materialXData || !materialXData->doc) return;

	const MaterialX::StringMap& inputMap = materialXData->glslFragmentWrapper->getPathInputMap();
	for (auto it = inputMap.begin(); it != inputMap.end(); ++it)
	{
		MaterialX::ElementPtr element = materialXData->doc->getDescendant(it->first);
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

////////////////////////////////////////////////////////////////////////////
// Node Implementation
////////////////////////////////////////////////////////////////////////////
const MTypeId TestFileNode::id(0x00991057);

void* TestFileNode::creator()
{
    return new TestFileNode();
}

TestFileNode::TestFileNode()
    : fWidth(0)
    , fHeight(0)
{
}

TestFileNode::~TestFileNode()
{
}

MStatus TestFileNode::setDependentsDirty(
    const MPlug& plug,
    MPlugArray& plugArray)
{
    if (plug == aFileName)
    {
        fImage.release();
        fWidth = fHeight = 0;
    }
    return MPxNode::setDependentsDirty(plug, plugArray);
}

MStatus TestFileNode::compute(const MPlug& plug, MDataBlock& block)
{
    // outColor or individial R, G, B channel, or alpha
    if ((plug != aOutColor) &&
        (plug.parent() != aOutColor) &&
        (plug != aOutAlpha))
    {
        return MS::kUnknownParameter;
    }

    MFloatVector resultColor(0.0f, 0.0f, 0.0f);
    float resultAlpha = 1.0f;

    // Read from file if we need to
    if (!fImage.pixels())
    {
        MString& fileName = block.inputValue(aFileName).asString();
        MString exactName(fileName);

        // This class is derived from MPxNode, therefore it is not a DAG node and does not have a path.
        // Instead you we just get the node's name using the name() method inherited from MPxNode as the context.
        if (MRenderUtil::exactFileTextureName(fileName, false, "", name(), exactName))
        {
            unsigned int width = 0;
            unsigned int height = 0;
            if (fImage.readFromFile(exactName) &&
                fImage.getSize(width, height))
            {
                fWidth = width;
                fHeight = height;
            }
        }
    }

    // Compute outputs from image data
    unsigned char* data = fImage.pixels();
    if (data && fWidth > 0 && fHeight > 0)
    {
        float2& uv = block.inputValue(aUVCoord).asFloat2();
        float u = uv[0]; if (u<0.0f) u = 0.0f; if (u>1.0f) u = 1.0f;
        float v = uv[1]; if (v<0.0f) v = 0.0f; if (v>1.0f) v = 1.0f;

        static const size_t pixelSize = 4;
        size_t rowOffset = (size_t)(v*(fHeight - 1)) * fWidth;
        size_t colOffset = (size_t)(u*(fWidth - 1));
        const unsigned char* pixel = data +
            ((rowOffset + colOffset) * pixelSize);

        resultColor[0] = ((float)pixel[0]) / 255.0f;
        resultColor[1] = ((float)pixel[1]) / 255.0f;
        resultColor[2] = ((float)pixel[2]) / 255.0f;
        resultAlpha = ((float)pixel[3]) / 255.0f;
    }

    // Set ouput color attribute
    MDataHandle outColorHandle = block.outputValue(aOutColor);
    MFloatVector& outColor = outColorHandle.asFloatVector();
    outColor = resultColor;
    outColorHandle.setClean();

    // Set ouput alpha attribute
    MDataHandle outAlphaHandle = block.outputValue(aOutAlpha);
    float& outAlpha = outAlphaHandle.asFloat();
    outAlpha = resultAlpha;
    outAlphaHandle.setClean();

    return MS::kSuccess;
}

// Attributes
MObject TestFileNode::aFileName;
MObject TestFileNode::aCMConfigPath;
MObject TestFileNode::aCMWorkingSpace;
MObject TestFileNode::aColorSpace;
MObject TestFileNode::aCMEnabled;
MObject TestFileNode::aCMConfigEnabled;
MObject TestFileNode::aUVCoord;
MObject TestFileNode::aOutColor;
MObject TestFileNode::aOutAlpha;

MStatus TestFileNode::initialize()
{
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;

    // Input attributes
    MFnStringData stringData;
    MObject theString = stringData.create();
    aFileName = tAttr.create("fileName", "f", MFnData::kString, theString);
    MAKE_INPUT(tAttr);
    MFnAttribute attr(aFileName);
    CHECK_MSTATUS(attr.setUsedAsFilename(true));

    MFnStringData stringDataCMConfigPath;
    MObject theStringCMConfigPath = stringDataCMConfigPath.create();
    aCMConfigPath = tAttr.create("colorManagementConfigFilePath", "cmcfp", MFnData::kString, theStringCMConfigPath);

    MFnStringData stringDataCMWorkingSpace;
    MObject theStringCMWorkingSpace = stringDataCMWorkingSpace.create();
    aCMWorkingSpace = tAttr.create("workingSpace", "ws", MFnData::kString, theStringCMWorkingSpace);
    MAKE_INPUT(tAttr);

    MFnStringData stringDataColorSpace;
    MObject theStringColorSpace = stringDataColorSpace.create();
    aColorSpace = tAttr.create("colorSpace", "cs", MFnData::kString, theStringColorSpace);
    MAKE_INPUT(tAttr);

    aCMEnabled = nAttr.create("colorManagementEnabled", "cme", MFnNumericData::kBoolean);
    MAKE_INPUT(nAttr);
    aCMConfigEnabled = nAttr.create("colorManagementConfigFileEnabled", "cmce", MFnNumericData::kBoolean);
    MAKE_INPUT(nAttr);


    MObject child1 = nAttr.create("uCoord", "u", MFnNumericData::kFloat);
    MObject child2 = nAttr.create("vCoord", "v", MFnNumericData::kFloat);
    aUVCoord = nAttr.create("uvCoord", "uv", child1, child2);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setHidden(true));

    // Output attributes
    aOutColor = nAttr.createColor("outColor", "oc");
    MAKE_OUTPUT(nAttr);

    aOutAlpha = nAttr.create("outAlpha", "oa", MFnNumericData::kFloat);
    MAKE_OUTPUT(nAttr);

    // Add attributes to the node database.
    CHECK_MSTATUS(addAttribute(aFileName));
    CHECK_MSTATUS(addAttribute(aUVCoord));
    CHECK_MSTATUS(addAttribute(aOutColor));
    CHECK_MSTATUS(addAttribute(aOutAlpha));
    CHECK_MSTATUS(addAttribute(aCMEnabled));
    CHECK_MSTATUS(addAttribute(aCMConfigEnabled));
    CHECK_MSTATUS(addAttribute(aCMConfigPath));
    CHECK_MSTATUS(addAttribute(aCMWorkingSpace));
    CHECK_MSTATUS(addAttribute(aColorSpace));

    // All input affect the output color and alpha
    CHECK_MSTATUS(attributeAffects(aFileName, aOutColor));
    CHECK_MSTATUS(attributeAffects(aFileName, aOutAlpha));
    CHECK_MSTATUS(attributeAffects(aUVCoord, aOutColor));
    CHECK_MSTATUS(attributeAffects(aUVCoord, aOutAlpha));
    CHECK_MSTATUS(attributeAffects(aCMEnabled, aOutColor));
    CHECK_MSTATUS(attributeAffects(aCMConfigEnabled, aOutColor));
    CHECK_MSTATUS(attributeAffects(aCMConfigPath, aOutColor));
    CHECK_MSTATUS(attributeAffects(aCMWorkingSpace, aOutColor));
    CHECK_MSTATUS(attributeAffects(aColorSpace, aOutColor));

    return MS::kSuccess;
}

//////////////////////////////////////////////////////////////////////////

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