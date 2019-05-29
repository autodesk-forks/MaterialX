#include "MaterialXNode.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MStringArray.h>
#include <maya/MDGModifier.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>

const MTypeId MaterialXNode::s_typeId(0x00042402);
const MString MaterialXNode::s_typeName("MaterialXNode");
MObject MaterialXNode::s_materialXAttr;
MObject MaterialXNode::s_targetAttr;

MaterialXNode::MaterialXNode()
{
}

MaterialXNode::~MaterialXNode()
{
}

void* MaterialXNode::creator()
{
    return new MaterialXNode();
}

MStatus MaterialXNode::initialize()
{
	MFnTypedAttribute typedAttr;
	MFnStringData stringData;

	MObject theString = stringData.create();
	s_materialXAttr = typedAttr.create("materialXDocument", "mxd", MFnData::kString, theString);

	CHECK_MSTATUS(typedAttr.setStorable(true));
	CHECK_MSTATUS(typedAttr.setReadable(true));
	CHECK_MSTATUS(typedAttr.setWritable(true));
	CHECK_MSTATUS(addAttribute(s_materialXAttr));

	s_targetAttr = typedAttr.create("target", "tar", MFnData::kString, theString);

	CHECK_MSTATUS(typedAttr.setStorable(true));
	CHECK_MSTATUS(typedAttr.setReadable(true));
	CHECK_MSTATUS(typedAttr.setWritable(true));

	CHECK_MSTATUS(addAttribute(s_materialXAttr));
	CHECK_MSTATUS(addAttribute(s_targetAttr));

	return MS::kSuccess;
}

MTypeId MaterialXNode::typeId() const
{
    return s_typeId;
}
