#include "Util.h"

#include <MaterialXCore/Traversal.h>
#include <MaterialXFormat/XmlIo.h>

#include <iostream>

namespace mx = MaterialX;

namespace MaterialXMaya
{

void loadLibrary(const mx::FilePath& filePath, mx::DocumentPtr doc)
{
    std::cout << "Load library: " << filePath.asString() << std::endl;
	mx::DocumentPtr libDoc = mx::createDocument();
    mx::XmlReadOptions readOptions;
    readOptions.skipDuplicateElements = true;
	mx::readFromXmlFile(libDoc, filePath, mx::EMPTY_STRING, &readOptions);
	mx::CopyOptions copyOptions;
	copyOptions.skipDuplicateElements = true;
	doc->importLibrary(libDoc, &copyOptions);
}

void loadLibraries(const mx::StringVec& libraryNames,
	               const mx::FileSearchPath& searchPath,
	               mx::DocumentPtr doc,
	               const mx::StringSet* excludeFiles)
{
	for (const std::string& libraryName : libraryNames)
	{
		mx::FilePath libraryPath = searchPath.find(libraryName);
		for (const mx::FilePath& path : libraryPath.getSubDirectories())
		{
			for (const mx::FilePath& filename : path.getFilesInDirectory(mx::MTLX_EXTENSION))
			{
				if (!excludeFiles || !excludeFiles->count(filename))
				{
					loadLibrary(path / filename, doc);
				}
			}
		}
	}
}

} // namespace MaterialXMaya

