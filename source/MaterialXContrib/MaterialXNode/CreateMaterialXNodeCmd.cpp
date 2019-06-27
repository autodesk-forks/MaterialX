#include "CreateMaterialXNodeCmd.h"
#include "MaterialXNode.h"
#include "MaterialXUtil.h"
#include "Plugin.h"

#include <MaterialXFormat/XmlIo.h>

#include <maya/MArgParser.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStringArray.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>

#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

#include <algorithm>
#include <sstream>

namespace mx = MaterialX;

namespace
{
const char* const kDocumentFlag     = "d";
const char* const kDocumentFlagLong = "document";

const char* const kElementFlag      = "e";
const char* const kElementFlagLong  = "element";

const char* const kTextureFlag      = "t";
const char* const kTextureFlagLong  = "texture";

const char* const kOgsXmlFlag       = "x";
const char* const kOgsXmlFlagLong   = "ogsxml";

void registerFragment(const MaterialXData& materialData, const std::string& ogsXmlFileName)
{
    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
    MHWRender::MFragmentManager* fragmentManager = theRenderer ? theRenderer->getFragmentManager() : nullptr;
    if (!fragmentManager)
    {
        throw mx::Exception("Failed to find fragment manager");
    }

    // Name of fragment created or reused
    MString fragmentNameM;

    // Register fragments with the manager if needed
    const std::string& fragmentString = materialData.getFragmentSource();
    const std::string& fragmentName = materialData.getFragmentName();
    if (!fragmentName.empty() && !fragmentString.empty())
    {
        MString previousOutputDirectory(fragmentManager->getEffectOutputDirectory());
        MString previousIntermdiateDirectory(fragmentManager->getIntermediateGraphOutputDirectory());
        mx::FilePath dumpPath(Plugin::instance().getShaderDebugPath());
        bool setDumpPath = !dumpPath.isEmpty();
        if (setDumpPath)
        {
            fragmentManager->setEffectOutputDirectory(dumpPath.asString().c_str());
            fragmentManager->setIntermediateGraphOutputDirectory(dumpPath.asString().c_str());
        }

        const bool fragmentExists = fragmentManager->hasFragment(fragmentName.c_str());
        if (!fragmentExists)
        {
            // If explicit XML file specified use it.
            mx::FilePath ogsXmlPath;
            if (!ogsXmlFileName.empty())
            {
                ogsXmlPath = Plugin::instance().getResourceSearchPath().find(ogsXmlFileName);
                if (!ogsXmlPath.isEmpty())
                {
                    fragmentNameM = fragmentManager->addShadeFragmentFromFile(ogsXmlPath.asString().c_str(), false);
                }
            }

            // Otherwise use the generated XML
            else
            {
                fragmentNameM = fragmentManager->addShadeFragmentFromBuffer(fragmentString.c_str(), false);
            }
        }
        else
        {
            fragmentNameM.set(fragmentName.c_str());
        }

        if (setDumpPath)
        {
            fragmentManager->setEffectOutputDirectory(previousOutputDirectory);
            fragmentManager->setIntermediateGraphOutputDirectory(previousIntermdiateDirectory);
        }
    }

    // TODO: On failure a fallback shader should be provided.
    if (fragmentNameM.length() == 0)
    {
        throw mx::Exception("Failed to add shader fragment: (" + fragmentName + ")");
    }
}

}

MString CreateMaterialXNodeCmd::NAME("createMaterialXNode");

CreateMaterialXNodeCmd::CreateMaterialXNodeCmd()
{
}

CreateMaterialXNodeCmd::~CreateMaterialXNodeCmd()
{
}

MStatus CreateMaterialXNodeCmd::doIt( const MArgList &args )
{
    // Parse the shader node
    //
    MArgParser parser(syntax(), args);

    MStatus status;
    MArgDatabase argData(syntax(), args, &status);
    if (!status)
        return status;

    MString elementPath;
    try
    {
        MString materialXDocument;
        if (parser.isFlagSet(kDocumentFlag))
        {
            argData.getFlagArgument(kDocumentFlag, 0, materialXDocument);
        }

        if (materialXDocument.length() == 0)
        {
            throw mx::Exception("MaterialX document file path is empty.");
        }

	    if (parser.isFlagSet(kElementFlag))
	    {
		    argData.getFlagArgument(kElementFlag, 0, elementPath);
	    }

        MString ogsXmlFileName;
        if (parser.isFlagSet(kOgsXmlFlag))
        {
            argData.getFlagArgument(kOgsXmlFlag, 0, ogsXmlFileName);
        }

        std::unique_ptr<MaterialXData> materialXData{
            new MaterialXData(materialXDocument.asChar(),
                              elementPath.asChar(),
                              Plugin::instance().getLibrarySearchPath())
        };

        elementPath.set(materialXData->getElementPath().c_str());
        if (elementPath.length() == 0)
        {
            throw mx::Exception("The element specified is not renderable.");
        }

        // Create the MaterialX node
        MObject node = _dgModifier.createNode(
            parser.isFlagSet(kTextureFlag) ? MaterialXTextureNode::MATERIALX_TEXTURE_NODE_TYPEID
            : MaterialXSurfaceNode::MATERIALX_SURFACE_NODE_TYPEID
        );

        // Generate a valid Maya node name from the path string
        std::string nodeName = mx::createValidName(elementPath.asChar());
        _dgModifier.renameNode(node, nodeName.c_str());

        materialXData->generateXml();
        ::registerFragment(*materialXData, ogsXmlFileName.asChar());

        MFnDependencyNode depNode(node);
        auto materialXNode = dynamic_cast<MaterialXNode*>(depNode.userNode());
        if (!materialXNode)
        {
            throw mx::Exception("Unexpected DG node type.");
        }

        std::string documentString = mx::writeToXmlString(materialXData->getDocument());

        materialXNode->setMaterialXData(std::move(materialXData));
        materialXNode->createOutputAttr(_dgModifier);

        MPlug materialXPlug(node, MaterialXNode::DOCUMENT_ATTRIBUTE);
        _dgModifier.newPlugValueString(materialXPlug, documentString.c_str());

        MPlug elementPlug(node, MaterialXNode::ELEMENT_ATTRIBUTE);
        _dgModifier.newPlugValueString(elementPlug, elementPath);

        _dgModifier.doIt();
    }
    catch (mx::Exception& e)
    {
        MString message("Failed to create MaterialX node: ");
        message += MString(e.what());
        MGlobal::displayError(message);
        return MS::kFailure;
    }

    MString message("Created MaterialX node: ");
    message += elementPath;
    MGlobal::displayInfo(message);
    return MS::kSuccess;
 }

MSyntax CreateMaterialXNodeCmd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag(kDocumentFlag, kDocumentFlagLong, MSyntax::kString);
	syntax.addFlag(kElementFlag, kElementFlagLong, MSyntax::kString);
    syntax.addFlag(kOgsXmlFlag, kOgsXmlFlagLong, MSyntax::kString);
    syntax.addFlag(kTextureFlag, kTextureFlagLong, MSyntax::kNoArg);
	return syntax;
}

void* CreateMaterialXNodeCmd::creator()
{
	return new CreateMaterialXNodeCmd();
}

// Sets the value of the specified MaterialXNode attribute
void CreateMaterialXNodeCmd::setAttributeValue(MObject &materialXObject, MObject &attr, const float* values, unsigned int size)
{
	MPlug plug(materialXObject, attr);
	if (size == 1)
	{
		_dgModifier.newPlugValueDouble(plug, *values);
	}
	else
	{
		for (unsigned int i=0; i<size; i++)
		{
			MPlug indexPlug = plug.child(i);
			_dgModifier.newPlugValueDouble(indexPlug, values[i]);
		}
	}
}

void  CreateMaterialXNodeCmd::setAttributeValue(MObject &materialXObject, MObject &attr, const std::string& stringValue)
{
	MPlug plug(materialXObject, attr);
    _dgModifier.newPlugValueString(plug, stringValue.c_str());
}

void CreateMaterialXNodeCmd::createAttribute(MObject &materialXObject, const std::string& /*name*/, const mx::UIPropertyItem& propertyItem)
{
	MFnNumericAttribute numericAttr;
	MFnTypedAttribute typedAttr;
	MObject attr;

	std::string label = propertyItem.label;
	mx::ValuePtr value = propertyItem.value;
	if (value->getTypeString() == mx::TypedValue<mx::Color2>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::k2Double, 0.0);
		_dgModifier.addAttribute(materialXObject, attr);
		mx::Color2 color2 = value->asA<mx::Color2>();
		setAttributeValue(materialXObject, attr, color2.data(), 2);
	}
	else if (value->getTypeString() == mx::TypedValue<mx::Color3>::TYPE)
	{
		attr = numericAttr.createColor(label.c_str(), label.c_str());
		_dgModifier.addAttribute(materialXObject, attr);
		mx::Color3 color3 = value->asA<mx::Color3>();
		setAttributeValue(materialXObject, attr, color3.data(), 3);
	}
	else if (value->getTypeString() == mx::TypedValue<mx::Color4>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::k4Double, 0.0);
		_dgModifier.addAttribute(materialXObject, attr);
		mx::Color4 color4 = value->asA<mx::Color4>();
		setAttributeValue(materialXObject, attr, color4.data(), 4);
	}
	else if (value->getTypeString() == mx::TypedValue<mx::Vector2>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::k2Double, 0.0);
		_dgModifier.addAttribute(materialXObject, attr);
		mx::Vector2 vector2 = value->asA<mx::Vector2>();
		setAttributeValue(materialXObject, attr, vector2.data(), 2);
	}
	else if (value->getTypeString() == mx::TypedValue<mx::Vector3>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::k3Double, 0.0);
		_dgModifier.addAttribute(materialXObject, attr);
		mx::Vector3 vector3 = value->asA<mx::Vector3>();
		setAttributeValue(materialXObject, attr, vector3.data(), 3);
	}
	else if (value->getTypeString() == mx::TypedValue<mx::Vector4>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::k4Double, 0.0);
		_dgModifier.addAttribute(materialXObject, attr);
		mx::Vector4 vector4 = value->asA<mx::Vector4>();
		setAttributeValue(materialXObject, attr, vector4.data(), 4);
	}
	else if (value->getTypeString() == mx::TypedValue<float>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::kDouble, 0.0);
		_dgModifier.addAttribute(materialXObject, attr);
		float floatValue = value->asA<float>();
		setAttributeValue(materialXObject, attr, &floatValue, 1);
	}
	else if (value->getTypeString() == mx::TypedValue<std::string>::TYPE)
	{
		attr = typedAttr.create(label.c_str(), label.c_str(), MFnData::kString, MObject::kNullObj);
		_dgModifier.addAttribute(materialXObject, attr);
		std::string stringValue = value->asA<std::string>();
		setAttributeValue(materialXObject, attr, stringValue);
	}
}

void CreateMaterialXNodeCmd::createAttributes(MObject &materialXObject, const mx::UIPropertyGroup& groups, const mx::UIPropertyGroup& unnamedGroups)
{
	for (auto groupIt = groups.begin(); groupIt != groups.end(); ++groupIt)
	{
		createAttribute(materialXObject, groupIt->first, groupIt->second);
	}
	for (auto unnamedGroupIt = unnamedGroups.begin(); unnamedGroupIt != unnamedGroups.end(); ++unnamedGroupIt)
	{
		createAttribute(materialXObject, unnamedGroupIt->first, unnamedGroupIt->second);
	}
}
