#include "CreateMaterialXNodeCmd.h"
#include "MaterialXNode.h"

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>

#include <maya/MArgParser.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStringArray.h>
#include <maya/MFnNumericAttribute.h>
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
		MObject materialXObject = m_dgMod.createNode(MaterialXNode::s_typeId);
		std::string nodeName = targetString.asChar();
		std::replace(nodeName.begin(), nodeName.end(), '/', '_');
		m_dgMod.renameNode(materialXObject, nodeName.c_str());

		MaterialX::DocumentPtr doc = MaterialX::createDocument();
		MaterialX::readFromXmlFile(doc, materialXDocument.asChar());

		MaterialX::ElementPtr element = doc->getDescendant(targetString.asChar());
		if (element->isA<MaterialX::Node>())
		{
			// Create attributes for all inputs/parameters on the node
			MaterialX::NodePtr node = element->asA<MaterialX::Node>();
			for (MaterialX::ElementPtr element2 : node->getChildren())
			{
				if (element2->isA<MaterialX::Input>())
				{
					MaterialX::InputPtr input = element2->asA<MaterialX::Input>();
					createAttribute(materialXObject, input->getName(), input->getType(), input->getValue()->getValueString());
				}
				if (element2->isA<MaterialX::Parameter>())
				{
					MaterialX::ParameterPtr parameter = element2->asA<MaterialX::Parameter>();
					createAttribute(materialXObject, parameter->getName(), parameter->getType(), parameter->getValue()->getValueString());
				}
			}
		}
		else if (element->isA<MaterialX::NodeGraph>())
		{
			// TODO: Implement me!
		}

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

// Retrieves the floating point values from a string
void getValues(const std::string& value, std::vector<double> &results, int numValues)
{
	std::stringstream ss(value);
	std::string s;
	int i = 0;
	while (getline(ss, s, ' ') && i++ < numValues) {
		results.push_back(std::stod(s));
	}
}

// Sets the value of the specified MaterialXNode attribute
void CreateMaterialXNodeCmd::setAttributeValue(MObject &materialXObject, MObject &attr, std::vector<double> &values)
{
	MPlug plug(materialXObject, attr);
	int i = 0;
	if (values.size() == 1)
	{
		m_dgMod.newPlugValueDouble(plug, values[0]);
	}
	else
	{
		for (double value : values)
		{
			MPlug indexPlug = plug.child(i++);
			m_dgMod.newPlugValueDouble(indexPlug, value);
		}
	}
}

// Creates the specified attribute on the provided MaterialXNode
void CreateMaterialXNodeCmd::createAttribute(MObject &materialXObject, const std::string &name, const std::string &type, const std::string &value)
{
	MFnNumericAttribute numericAttr;
	MObject attr;
	if (type == MaterialX::TypedValue<MaterialX::Color2>::TYPE)
	{
		attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k2Double, 0.0);
		std::vector<double> values;
		getValues(value, values, 2);
		m_dgMod.addAttribute(materialXObject, attr);
		setAttributeValue(materialXObject, attr, values);
	}
	else if (type == MaterialX::TypedValue<MaterialX::Color3>::TYPE)
	{
		attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k3Double, 0.0);
		std::vector<double> values;
		getValues(value, values, 3);
		m_dgMod.addAttribute(materialXObject, attr);
		setAttributeValue(materialXObject, attr, values);
	}
	else if (type == MaterialX::TypedValue<MaterialX::Color4>::TYPE)
	{
		attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k4Double, 0.0);
		std::vector<double> values;
		getValues(value, values, 4);
		m_dgMod.addAttribute(materialXObject, attr);
		setAttributeValue(materialXObject, attr, values);
	}
	else if (type == MaterialX::TypedValue<MaterialX::Vector2>::TYPE)
	{
		attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k2Double, 0.0);
		std::vector<double> values;
		getValues(value, values, 2);
		m_dgMod.addAttribute(materialXObject, attr);
		setAttributeValue(materialXObject, attr, values);
	}
	else if (type == MaterialX::TypedValue<MaterialX::Vector3>::TYPE)
	{
		attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k3Double, 0.0);
		std::vector<double> values;
		getValues(value, values, 3);
		m_dgMod.addAttribute(materialXObject, attr);
		setAttributeValue(materialXObject, attr, values);
	}
	else if (type == MaterialX::TypedValue<MaterialX::Vector4>::TYPE)
	{
		attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::k4Double, 0.0);
		std::vector<double> values;
		getValues(value, values, 4);
		m_dgMod.addAttribute(materialXObject, attr);
		setAttributeValue(materialXObject, attr, values);
	}
	else if (type == MaterialX::TypedValue<float>::TYPE)
	{
		attr = numericAttr.create(name.c_str(), name.c_str(), MFnNumericData::kDouble, 0.0);
		std::vector<double> values;
		getValues(value, values, 1);
		m_dgMod.addAttribute(materialXObject, attr);
		setAttributeValue(materialXObject, attr, values);
	}

	// TODO: Implement other data types
}
