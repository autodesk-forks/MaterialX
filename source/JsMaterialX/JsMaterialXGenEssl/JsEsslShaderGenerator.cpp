#include <MaterialXGenEssl/EsslShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

mx::ElementPtr findRenderableElement(mx::DocumentPtr doc) {
    mx::StringVec renderablePaths;
    std::vector<mx::TypedElementPtr> elems;
    mx::findRenderableElements(doc, elems);

    for (mx::TypedElementPtr elem : elems)
    {
        mx::TypedElementPtr renderableElem = elem;
        mx::NodePtr node = elem->asA<mx::Node>();
        if (node && node->getType() == mx::MATERIAL_TYPE_STRING)
        {
            std::unordered_set<mx::NodePtr> shaderNodes = getShaderNodes(node, mx::SURFACE_SHADER_TYPE_STRING);
            if (!shaderNodes.empty())
            {
                renderableElem = *shaderNodes.begin();
            }
        }

        const auto& renderablePath = renderableElem->getNamePath();
        mx::ElementPtr renderableElement = doc->getDescendant(renderablePath);
        mx::TypedElementPtr typedElem = renderableElement ? renderableElement->asA<mx::TypedElement>() : nullptr;
        if (typedElem) {
            return renderableElement;
        }
    }

    return nullptr;
}

EMSCRIPTEN_BINDINGS(EsslShaderGenerator)
{
    ems::class_<mx::EsslShaderGenerator, ems::base<mx::ShaderGenerator>>("EsslShaderGenerator")
        .smart_ptr_constructor("EsslShaderGenerator", &std::make_shared<mx::EsslShaderGenerator>)
        ;

    ems::function("findRenderableElement", &findRenderableElement);
}