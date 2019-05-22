#include "MaterialXNode.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MStringArray.h>
#include <maya/MDGModifier.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>

const MTypeId MaterialXNode::s_typeId(0x00042402);
const MString MaterialXNode::s_typeName("MaterialXNode");

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
    return MS::kSuccess;
}

MTypeId MaterialXNode::typeId() const
{
    return s_typeId;
}
