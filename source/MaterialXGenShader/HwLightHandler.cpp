#include <MaterialXGenShader/HwLightHandler.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

#include <iterator>

namespace MaterialX
{

HwLightHandler::HwLightHandler()
{
}

HwLightHandler::~HwLightHandler()
{
}

void HwLightHandler::addLightSource(NodePtr node)
{
    _lightSources.push_back(node);
}

}
