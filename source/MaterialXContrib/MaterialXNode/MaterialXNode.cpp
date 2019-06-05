#include "MaterialXNode.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MStringArray.h>
#include <maya/MDGModifier.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>
#include <maya/MRenderUtil.h>
#include <maya/MFloatVector.h>


const MTypeId MaterialXNode::MATERIALX_NODE_TYPEID(0x00042402);
const MString MaterialXNode::MATERIALX_NODE_TYPENAME("MaterialXNode");

MString MaterialXNode::DOCUMENT_ATTRIBUTE_LONG_NAME("document");
MString MaterialXNode::DOCUMENT_ATTRIBUTE_SHORT_NAME("doc");
MObject MaterialXNode::DOCUMENT_ATTRIBUTE;

MString MaterialXNode::ELEMENT_ATTRIBUTE_LONG_NAME("element");
MString MaterialXNode::ELEMENT_ATTRIBUTE_SHORT_NAME("ele");
MObject MaterialXNode::ELEMENT_ATTRIBUTE;

MString MaterialXNode::OUT_COLOR_ATTRIBUTE_LONG_NAME("outColor");
MString MaterialXNode::OUT_COLOR_ATTRIBUTE_SHORT_NAME("oc");
MObject MaterialXNode::OUT_COLOR_ATTRIBUTE;

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
	MFnNumericAttribute nAttr;
	MFnStringData stringData;

	MObject theString = stringData.create();
	DOCUMENT_ATTRIBUTE = typedAttr.create(DOCUMENT_ATTRIBUTE_LONG_NAME, DOCUMENT_ATTRIBUTE_SHORT_NAME, MFnData::kString, theString);
	CHECK_MSTATUS(typedAttr.setStorable(true));
	CHECK_MSTATUS(typedAttr.setReadable(true));
    CHECK_MSTATUS(typedAttr.setHidden(true));
	CHECK_MSTATUS(addAttribute(DOCUMENT_ATTRIBUTE));

	ELEMENT_ATTRIBUTE = typedAttr.create(ELEMENT_ATTRIBUTE_LONG_NAME, ELEMENT_ATTRIBUTE_SHORT_NAME, MFnData::kString, theString);
	CHECK_MSTATUS(typedAttr.setStorable(true));
    CHECK_MSTATUS(typedAttr.setHidden(true));
    CHECK_MSTATUS(typedAttr.setReadable(true));
	CHECK_MSTATUS(addAttribute(ELEMENT_ATTRIBUTE));

	OUT_COLOR_ATTRIBUTE = nAttr.createColor(OUT_COLOR_ATTRIBUTE_LONG_NAME, OUT_COLOR_ATTRIBUTE_SHORT_NAME);
	CHECK_MSTATUS(typedAttr.setStorable(false));
	CHECK_MSTATUS(typedAttr.setHidden(false));
	CHECK_MSTATUS(typedAttr.setReadable(true));
	CHECK_MSTATUS(typedAttr.setWritable(false));
	CHECK_MSTATUS(addAttribute(OUT_COLOR_ATTRIBUTE));

	CHECK_MSTATUS(attributeAffects(ELEMENT_ATTRIBUTE, OUT_COLOR_ATTRIBUTE));
	CHECK_MSTATUS(attributeAffects(DOCUMENT_ATTRIBUTE, OUT_COLOR_ATTRIBUTE));

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

