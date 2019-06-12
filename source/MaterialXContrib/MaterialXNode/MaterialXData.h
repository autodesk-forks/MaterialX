#ifndef MATERIALX_DATA_H
#define MATERIALX_DATA_H

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/GenContext.h>

#include "../OGSXMLFragmentWrapper.h"

#include <maya/MString.h>

#include <vector>

struct MaterialXData
{
	MaterialXData(const std::string& materialXDocument, const std::string& elementPath);
	~MaterialXData();

	bool isValidOutput();
	void createXMLWrapper();
	void registerFragments();

    MaterialXData& operator=(const MaterialXData&) = delete;
    MaterialXData& operator=(MaterialXData&&) = delete;

	MaterialX::FilePath libSearchPath;
	MaterialX::DocumentPtr doc;
	MaterialX::ElementPtr element;

	MString fragmentName;
	std::unique_ptr<MaterialX::OGSXMLFragmentWrapper> glslFragmentWrapper;
	std::vector<std::unique_ptr<MaterialX::GenContext>> contexts;

  private:
	void createDocument(const std::string& materialXDocument);
};

#endif // MATERIALX_DATA_H
