#ifndef MATERIALX_DATA_H
#define MATERIALX_DATA_H

/// @file
/// MaterialX Data wrapper

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXContrib/OGSXMLFragmentWrapper.h>

#include <maya/MString.h>

/// @class MaterialXData
/// Wrapper for MaterialX associated data. 
///
/// Keeps track of an element to render and it's associated document.
///
/// Can optionally create and cache an OGS XML wrapper instance 
/// which wraps up the interface and shader code shader code generated based 
/// on the specified element to render.
/// Currently only code for GLSL is generated.
///
struct MaterialXData
{
  public:
    /// Create MaterialX data constainer.
    /// The element path and document that the element resides in are passed in
    /// as input arguments
    MaterialXData(const std::string& materialXDocumentPath, const std::string& elementPath);
    ~MaterialXData();

    /// Returns whether the element set to render is a valid output
    bool isValidOutput();

    /// Create the OGS XML wrapper for shader fragments associated
    /// with the element set to render
    void createXMLWrapper();

    /// Register the fragment(s)
    void registerFragments(const std::string& ogsXmlPath);

    MaterialXData& operator=(const MaterialXData&) = delete;
    MaterialXData& operator=(MaterialXData&&) = delete;

    /// Return MaterialX document 
    MaterialX::DocumentPtr getDocument() const
    {
        return _document;
    }

    /// Return name of shader fragment
    const MString& getFragmentName() const
    {
        return _fragmentName;
    }

    /// Retuern pointer to the OGS XML wrapper
    MaterialX::OGSXMLFragmentWrapper* getFragmentWrapper() const
    {
        return _xmlFragmentWrapper.get();
    }

    /// Return if the element to render represents a shader graph
    /// as opposed to a texture grraph.
    bool elementIsAShader() const;

  protected:
    MaterialX::FilePath _librarySearchPath;
    MaterialX::DocumentPtr _document;
    MaterialX::ElementPtr _element;

    MString _fragmentName;
    std::unique_ptr<MaterialX::OGSXMLFragmentWrapper> _xmlFragmentWrapper;
    std::vector<std::unique_ptr<MaterialX::GenContext>> _contexts;

  private:
    void createDocument(const std::string& materialXDocument);
};

#endif // MATERIALX_DATA_H
