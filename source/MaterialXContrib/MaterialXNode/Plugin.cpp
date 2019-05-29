#include "Plugin.h"
#include "CreateMaterialXNodeCmd.h"
#include "MaterialXNode.h"

#include <maya/MFnPlugin.h>
#include <maya/MDGMessage.h>
#include <maya/MDrawRegistry.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

Plugin& Plugin::instance()
{
	static Plugin s_instance;
	return s_instance;
}

void Plugin::initialize(const std::string& loadPath)
{
	MaterialX::FilePath searchPath(loadPath);
	m_librarySearchPath = searchPath / MaterialX::FilePath("../../libraries");
}

// Plugin configuration
//
MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj, "Autodesk", "1.0", "Any");
	Plugin::instance().initialize(plugin.loadPath().asChar());

	CHECK_MSTATUS(plugin.registerCommand(
		"createMaterialXNode",
		CreateMaterialXNodeCmd::creator,
		CreateMaterialXNodeCmd::newSyntax));

	const MString materialXNodeClassification("texture/2d:drawdb/shader/texture/2d/materialXNode");
	CHECK_MSTATUS(plugin.registerNode(
		MaterialXNode::s_typeName,
		MaterialXNode::s_typeId,
		MaterialXNode::creator,
		MaterialXNode::initialize,
		MPxNode::kDependNode,
		&materialXNodeClassification));

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
