#ifndef MATERIALX_UTILS_H
#define MATERIALX_UTILS_H

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Interface.h>
#include <MaterialXFormat/File.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

/// Find a given file path under a set of search paths. The search is performed
/// on all subdirectories for each search path
mx::FilePath findInSubdirectories(const mx::FileSearchPath& searchPaths,
                                  const mx::FilePath& filePath);

/// Load in a document and associated libraries from library search path
mx::DocumentPtr loadDocument(const std::string& materialXDocumentPath,
                             const MaterialX::FileSearchPath& librarySearchPath);

/// Find renderable elements in a document. If a specific element is desired then this will
/// be checked to see if it is renderable.
std::vector<mx::TypedElementPtr> getRenderableElements(mx::DocumentPtr document, const std::string &desiredElementPath);

} // namespace MaterialXMaya

#endif
