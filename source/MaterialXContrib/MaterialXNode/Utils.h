#ifndef UTILS_H
#define UTILS_H

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Interface.h>
#include <MaterialXFormat/File.h>

void getInputs(MaterialX::OutputPtr output, std::vector<MaterialX::ValueElementPtr> &results);
void loadLibrary(const MaterialX::FilePath& file, MaterialX::DocumentPtr doc);
void loadLibraries(
	const MaterialX::StringVec& libraryNames,
	const MaterialX::FilePath& searchPath,
	MaterialX::DocumentPtr doc,
	const MaterialX::StringSet* excludeFiles = nullptr);

#endif /* UTILS_H */
