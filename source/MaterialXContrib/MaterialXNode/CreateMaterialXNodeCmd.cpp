#include "CreateMaterialXNodeCmd.h"
#include "MaterialXNode.h"
#include "Utils.h"
#include "Plugin.h"

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenShader/Util.h>

#include <maya/MArgParser.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStringArray.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>

#include <algorithm>
#include <sstream>

#define kDocumentFlag     "d"
#define kDocumentFlagLong "document"

#define kTargetFlag       "t"
#define kTargetFlagLong   "target"

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
	MArgParser parser( syntax(), args );

	MStatus status;
	MArgDatabase argData( syntax(), args, &status );
	if ( !status )
		return status;

	MString materialXDocument;
	MString targetString;
	if( parser.isFlagSet(kDocumentFlag) )
	{
		argData.getFlagArgument(kDocumentFlag, 0, materialXDocument);
	}
	if (parser.isFlagSet(kTargetFlag))
	{
		argData.getFlagArgument(kTargetFlag, 0, targetString);
	}
	if (materialXDocument.length() > 0 && targetString.length() > 0)
	{
		// Load document
		MaterialX::DocumentPtr doc = MaterialX::createDocument();
		MaterialX::readFromXmlFile(doc, materialXDocument.asChar());

		// Load libraries
		MaterialX::FilePath libSearchPath = Plugin::instance().getLibrarySearchPath();
		const MaterialX::StringVec libraries = { "stdlib", "pbrlib" };
		loadLibraries(libraries, libSearchPath, doc);

		// Check to make sure the targetString is a valid output node
		if (!validOutputSpecified(doc, targetString.asChar()))
		{
			displayError("A valid output path must be specified");
			return MS::kFailure;
		}

		// Create the MaterialX node
		MObject materialXObject = m_dgMod.createNode(MaterialXNode::s_typeId);

		// Generate a valid Maya node name from the path string (converting '/'s to '_'s)
		std::string nodeName = targetString.asChar();
		std::replace(nodeName.begin(), nodeName.end(), '/', '_');
		m_dgMod.renameNode(materialXObject, nodeName.c_str());

		std::string materialXDocumentContents = MaterialX::writeToXmlString(doc);
		MPlug materialXPlug(materialXObject, MaterialXNode::s_materialXAttr);
		m_dgMod.newPlugValueString(materialXPlug, materialXDocumentContents.c_str());

		MPlug targetPlug(materialXObject, MaterialXNode::s_targetAttr);
		m_dgMod.newPlugValueString(targetPlug, targetString);

		m_dgMod.doIt();
	}

	return MS::kSuccess;
 }

MSyntax CreateMaterialXNodeCmd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag(kDocumentFlag, kDocumentFlagLong, MSyntax::kString);
	syntax.addFlag(kTargetFlag, kTargetFlagLong, MSyntax::kString);
	return syntax;
}

void* CreateMaterialXNodeCmd::creator()
{
	return new CreateMaterialXNodeCmd();
}

// Determines whether the output specified is valid
bool CreateMaterialXNodeCmd::validOutputSpecified(MaterialX::DocumentPtr doc, const std::string &targetOutputPath)
{
	std::vector<MaterialX::TypedElementPtr> elements;
	MaterialX::findRenderableElements(doc, elements);
	for (MaterialX::TypedElementPtr element : elements)
	{
		if (element->getNamePath() == targetOutputPath)
		{
			return true;
		}
	}
	return false;
}

// Sets the value of the specified MaterialXNode attribute
void CreateMaterialXNodeCmd::setAttributeValue(MObject &materialXObject, MObject &attr, const float* values, unsigned int size)
{
	MPlug plug(materialXObject, attr);
	if (size == 1)
	{
		m_dgMod.newPlugValueDouble(plug, *values);
	}
	else
	{
		for (unsigned int i=0; i<size; i++)
		{
			MPlug indexPlug = plug.child(i);
			m_dgMod.newPlugValueDouble(indexPlug, values[i]);
		}
	}
}

void  CreateMaterialXNodeCmd::setAttributeValue(MObject &materialXObject, MObject &attr, const std::string& stringValue)
{
	MPlug plug(materialXObject, attr);
	m_dgMod.newPlugValueString(plug, stringValue.c_str());
}

void CreateMaterialXNodeCmd::createAttribute(MObject &materialXObject, const std::string& /*name*/, const MaterialX::UIPropertyItem& propertyItem)
{
	MFnNumericAttribute numericAttr;
	MFnTypedAttribute typedAttr;
	MObject attr;

	std::string label = propertyItem.label;
	MaterialX::ValuePtr value = propertyItem.value;
	if (value->getTypeString() == MaterialX::TypedValue<MaterialX::Color2>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::k2Double, 0.0);
		m_dgMod.addAttribute(materialXObject, attr);
		MaterialX::Color2 color2 = value->asA<MaterialX::Color2>();
		setAttributeValue(materialXObject, attr, color2.data(), 2);
	}
	else if (value->getTypeString() == MaterialX::TypedValue<MaterialX::Color3>::TYPE)
	{
		attr = numericAttr.createColor(label.c_str(), label.c_str());
		m_dgMod.addAttribute(materialXObject, attr);
		MaterialX::Color3 color3 = value->asA<MaterialX::Color3>();
		setAttributeValue(materialXObject, attr, color3.data(), 3);
	}
	else if (value->getTypeString() == MaterialX::TypedValue<MaterialX::Color4>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::k4Double, 0.0);
		m_dgMod.addAttribute(materialXObject, attr);
		MaterialX::Color4 color4 = value->asA<MaterialX::Color4>();
		setAttributeValue(materialXObject, attr, color4.data(), 4);
	}
	else if (value->getTypeString() == MaterialX::TypedValue<MaterialX::Vector2>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::k2Double, 0.0);
		m_dgMod.addAttribute(materialXObject, attr);
		MaterialX::Vector2 vector2 = value->asA<MaterialX::Vector2>();
		setAttributeValue(materialXObject, attr, vector2.data(), 2);
	}
	else if (value->getTypeString() == MaterialX::TypedValue<MaterialX::Vector3>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::k3Double, 0.0);
		m_dgMod.addAttribute(materialXObject, attr);
		MaterialX::Vector3 vector3 = value->asA<MaterialX::Vector3>();
		setAttributeValue(materialXObject, attr, vector3.data(), 3);
	}
	else if (value->getTypeString() == MaterialX::TypedValue<MaterialX::Vector4>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::k4Double, 0.0);
		m_dgMod.addAttribute(materialXObject, attr);
		MaterialX::Vector4 vector4 = value->asA<MaterialX::Vector4>();
		setAttributeValue(materialXObject, attr, vector4.data(), 4);
	}
	else if (value->getTypeString() == MaterialX::TypedValue<float>::TYPE)
	{
		attr = numericAttr.create(label.c_str(), label.c_str(), MFnNumericData::kDouble, 0.0);
		m_dgMod.addAttribute(materialXObject, attr);
		float floatValue = value->asA<float>();
		setAttributeValue(materialXObject, attr, &floatValue, 1);
	}
	else if (value->getTypeString() == MaterialX::TypedValue<std::string>::TYPE)
	{
		attr = typedAttr.create(label.c_str(), label.c_str(), MFnData::kString, MObject::kNullObj);
		m_dgMod.addAttribute(materialXObject, attr);
		std::string stringValue = value->asA<std::string>();
		setAttributeValue(materialXObject, attr, stringValue);
	}
}

void CreateMaterialXNodeCmd::createAttributes(MObject &materialXObject, const MaterialX::UIPropertyGroup& groups, const MaterialX::UIPropertyGroup& unnamedGroups)
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
