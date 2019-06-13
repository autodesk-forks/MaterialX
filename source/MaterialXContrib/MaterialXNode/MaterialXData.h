#ifndef MATERIALX_DATA_H
#define MATERIALX_DATA_H

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXContrib/OGSXMLFragmentWrapper.h>

#include <maya/MString.h>

#include <vector>

using OGSXMLFragmentWrapperPtr = std::unique_ptr<MaterialX::OGSXMLFragmentWrapper>;

struct MaterialXData
{
  public:
    MaterialXData(const std::string& materialXDocument, const std::string& elementPath);
	~MaterialXData();

	bool isValidOutput();
	void createXMLWrapper();
	void registerFragments();

    MaterialXData& operator=(const MaterialXData&) = delete;
    MaterialXData& operator=(MaterialXData&&) = delete;

    MaterialX::DocumentPtr getDocument() const
    {
        return _doc;
    }

    const MString& getFragmentName()
    {
        return _fragmentName;
    }

    MaterialX::OGSXMLFragmentWrapper* getFragmentWrapper() const
    {
        return _xmlFragmentWrapper.get();
    }

  protected:
	MaterialX::FilePath _libSearchPath;
	MaterialX::DocumentPtr _doc;
	MaterialX::ElementPtr _element;

	MString _fragmentName;
    OGSXMLFragmentWrapperPtr _xmlFragmentWrapper;
	std::vector<std::unique_ptr<MaterialX::GenContext>> _contexts;

  private:
	void createDocument(const std::string& materialXDocument);
};

#endif // MATERIALX_DATA_H
