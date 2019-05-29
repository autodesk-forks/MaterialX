#include "Utils.h"

#include <MaterialXCore/Traversal.h>
#include <MaterialXFormat/XmlIo.h>

void getInputs(MaterialX::OutputPtr output, std::vector<MaterialX::ValueElementPtr> &results)
{
	// TODO: Not all upstream element unconnected inputs are published inputs in a nodegraph, so we should prune the resulting list accordingly.

	// Iterate over all upstream elements of our output
	MaterialX::ElementPtr currentElement;
	MaterialX::PortElementPtr portElement;
	for (MaterialX::GraphIterator it = output->traverseGraph().begin(); it != MaterialX::GraphIterator::end(); ++it)
	{
		// Store the unconnected inputs/parameters
		currentElement = it.getUpstreamElement();
		const std::vector<MaterialX::ElementPtr>& childElements = currentElement->getChildren();
		for (MaterialX::ElementPtr childElement : childElements)
		{
			if (!childElement->getUpstreamElement() && childElement->isA<MaterialX::ValueElement>())
			{
				results.push_back(childElement->asA<MaterialX::ValueElement>());
			}
		}
	}
}

void loadLibrary(const MaterialX::FilePath& file, MaterialX::DocumentPtr doc)
{
	MaterialX::DocumentPtr libDoc = MaterialX::createDocument();
	MaterialX::readFromXmlFile(libDoc, file);
	MaterialX::CopyOptions copyOptions;
	copyOptions.skipDuplicateElements = true;
	doc->importLibrary(libDoc, &copyOptions);
}

void loadLibraries(const MaterialX::StringVec& libraryNames,
	const MaterialX::FilePath& searchPath,
	MaterialX::DocumentPtr doc,
	const MaterialX::StringSet* excludeFiles)
{
	for (const std::string& library : libraryNames)
	{
		MaterialX::FilePath libraryPath = searchPath / library;
		for (const MaterialX::FilePath& path : libraryPath.getSubDirectories())
		{
			for (const MaterialX::FilePath& filename : path.getFilesInDirectory(MaterialX::MTLX_EXTENSION))
			{
				if (!excludeFiles || !excludeFiles->count(filename))
				{
					loadLibrary(path / filename, doc);
				}
			}
		}
	}
}
