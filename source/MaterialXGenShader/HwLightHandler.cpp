#include <MaterialXGenShader/HwLightHandler.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

#include <iterator>

namespace MaterialX
{

int HwLightHandler::getLightType(NodePtr node) const
{
    int typeId = -1;
    auto pos = std::find(_lightCategories.begin(), _lightCategories.end(), node->getCategory());
    if (pos != _lightCategories.end())
    {
        typeId = static_cast<int>(std::distance(_lightCategories.begin(), pos));
    }
    return typeId;
}

HwLightHandler::HwLightHandler()
{
}

HwLightHandler::~HwLightHandler()
{
}

void HwLightHandler::addLightSource(NodePtr node)
{
    _lightSources.push_back(node);
    _lightCategories.insert(node->getCategory());
}

void HwLightHandler::bindLightShaders(HwShaderGenerator& shadergen, const GenOptions& options) const
{
    for (auto lightSource : _lightSources)
    {
        shadergen.bindLightShader(*lightSource->getNodeDef(), getLightType(lightSource), options);
    }
}

}
