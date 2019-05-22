#include <maya/MFnPlugin.h>
#include <maya/MDGMessage.h>
#include <maya/MDrawRegistry.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

#include "CreateMaterialXNodeCmd.h"
#include "MaterialXNode.h"

// Plugin configuration
//
MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj, "Autodesk", "1.0", "Any");

	CHECK_MSTATUS(
		plugin.registerCommand("createMaterialXNode",
		CreateMaterialXNodeCmd::creator,
		CreateMaterialXNodeCmd::newSyntax));

	CHECK_MSTATUS(
		plugin.registerNode(
		MaterialXNode::s_typeName,
		MaterialXNode::s_typeId,
		MaterialXNode::creator,
		MaterialXNode::initialize));

	return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);
	MStatus status;

	CHECK_MSTATUS(plugin.deregisterNode(MaterialXNode::s_typeId));

	CHECK_MSTATUS(plugin.deregisterCommand("createMaterialXNode"));

	return MS::kSuccess;
}
