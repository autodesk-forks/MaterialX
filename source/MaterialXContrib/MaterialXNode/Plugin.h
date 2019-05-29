#ifndef PLUGIN_H
#define PLUGIN_H

#include <MaterialXFormat/File.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>

class Plugin
{
  public:
	static Plugin& instance();

	void initialize(const std::string& loadPath);

	MaterialX::FilePath getLibrarySearchPath() const
	{
		return m_librarySearchPath;
	}

  private:
	Plugin()
	{
	}

	std::string m_librarySearchPath;
};

#endif /* PLUGIN_H */
