#include "Plugin.h"
#include "CreateMaterialXNodeCmd.h"
#include "MaterialXNode.h"
#include "MaterialXTextureOverride.h"

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
	_librarySearchPath = searchPath / MaterialX::FilePath("../../libraries");
	_ogsXmlFragmentPath = searchPath / MaterialX::FilePath("../resources");
}

///////////////////////////////////////////////////////////////
static const MString sRegistrantId("testFileTexturePlugin");

// Plugin configuration
//
MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj, "Autodesk", "1.0", "Any");
	Plugin::instance().initialize(plugin.loadPath().asChar());

	CHECK_MSTATUS(plugin.registerCommand(
        CreateMaterialXNodeCmd::NAME,
		CreateMaterialXNodeCmd::creator,
		CreateMaterialXNodeCmd::newSyntax));

	const MString materialXNodeClassification("texture/2d:drawdb/shader/texture/2d/materialXNode");
	CHECK_MSTATUS(plugin.registerNode(
		MaterialXNode::MATERIALX_NODE_TYPENAME,
		MaterialXNode::MATERIALX_NODE_TYPEID,
		MaterialXNode::creator,
		MaterialXNode::initialize,
		MPxNode::kDependNode,
		&materialXNodeClassification));

	CHECK_MSTATUS(MHWRender::MDrawRegistry::registerShadingNodeOverrideCreator(
		"drawdb/shader/texture/2d/materialXNode",
		MaterialXTextureOverride::REGISTRANT_ID,
		MaterialXTextureOverride::creator));

    ///////////////////////////////////////////////////////////////////////
    const MString UserClassify("texture/2d:drawdb/shader/texture/2d/testFileTexture");

    CHECK_MSTATUS(plugin.registerNode(
        "testFileTexture",
        TestFileNode::id,
        TestFileNode::creator,
        TestFileNode::initialize,
        MPxNode::kDependNode,
        &UserClassify));

    CHECK_MSTATUS(MHWRender::MDrawRegistry::registerShadingNodeOverrideCreator(
            "drawdb/shader/texture/2d/testFileTexture",
            sRegistrantId,
            TestFileNodeOverride::creator));

    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);
	MStatus status;

	CHECK_MSTATUS(plugin.deregisterNode(MaterialXNode::MATERIALX_NODE_TYPEID));

	CHECK_MSTATUS(plugin.deregisterCommand(CreateMaterialXNodeCmd::NAME));

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
		"drawdb/shader/texture/2d/materialXNode",
		MaterialXTextureOverride::REGISTRANT_ID));

    ///////////////////////////////////////////
    CHECK_MSTATUS(plugin.deregisterNode(TestFileNode::id));
    CHECK_MSTATUS(
        MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
            "drawdb/shader/texture/2d/testFileTexture",
            sRegistrantId));


	return MS::kSuccess;
}
