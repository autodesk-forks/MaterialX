#ifndef UTILS_H
#define UTILS_H

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Interface.h>
#include <MaterialXFormat/File.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

void loadLibrary(const mx::FilePath& file, mx::DocumentPtr doc);

void loadLibraries(const mx::StringVec& libraryNames,
                   const mx::FileSearchPath& searchPath,
                   mx::DocumentPtr doc,
                   const mx::StringSet* excludeFiles = nullptr);

} // namespace MaterialXMaya

#endif
