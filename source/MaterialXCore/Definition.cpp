//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Definition.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

const string COLOR_SEMANTIC = "color";
const string SHADER_SEMANTIC = "shader";

const string NodeDef::NODE_ATTRIBUTE = "node";
const string TypeDef::SEMANTIC_ATTRIBUTE = "semantic";
const string TypeDef::CONTEXT_ATTRIBUTE = "context";
const string Implementation::NODE_DEF_ATTRIBUTE = "nodedef";
const string Implementation::FILE_ATTRIBUTE = "file";
const string Implementation::FUNCTION_ATTRIBUTE = "function";
const string Implementation::LANGUAGE_ATTRIBUTE = "language";

//
// NodeDef methods
//

InterfaceElementPtr NodeDef::getImplementation(const string& target, const string& language) const
{
    InterfaceElementPtr foundElement = nullptr;
    for (InterfaceElementPtr element : getDocument()->getMatchingImplementations(getName()))
    {
        // If language specified then if it's not a node-graph then check both
        // language and target. Otherwise just check target if the the implementaitons
        // is a nodegraph.
        if (!language.empty())
        {
            // Check direct implementation
            ImplementationPtr implementation = element->asA<Implementation>();
            if (implementation)
            {
                // Check for a language match
                const std::string& lang = implementation->getLanguage();
                if (lang.length() && lang == language)
                {
                    // Check target. Note that it may be empty.
                    if (targetStringsMatch(implementation->getTarget(), target))
                    {
                        return implementation;
                    }
                }
            }

            // Check for a nodegraph implementation which does not require a
            // language check
            else if (element->isA<NodeGraph>())
            {
                if (targetStringsMatch(implementation->getTarget(), target))
                {
                    return element;
                }
            }
        }

        // No language specified, just check target
        else
        {
            if (targetStringsMatch(element->getTarget(), target))
            {
                return element;
            }
        }
    }
    return InterfaceElementPtr();
}

vector<ShaderRefPtr> NodeDef::getInstantiatingShaderRefs() const
{
    vector<ShaderRefPtr> shaderRefs;
    for (MaterialPtr mat : getRoot()->getChildrenOfType<Material>())
    {
        for (ShaderRefPtr shaderRef : mat->getShaderRefs())
        {
            if (shaderRef->getNodeDef() == getSelf())
            {
                shaderRefs.push_back(shaderRef);
            }
        }
    }
    return shaderRefs;
}

bool NodeDef::validate(string* message) const
{
    bool res = true;
    validateRequire(hasType(), res, message, "Missing type");
    if (getType() == MULTI_OUTPUT_TYPE_STRING)
    {
        validateRequire(getOutputCount() >= 2, res, message, "Multioutput nodedefs must have two or more output ports");
    }
    else
    {
        validateRequire(getOutputCount() == 0, res, message, "Only multioutput nodedefs support output ports");
    }
    return InterfaceElement::validate(message) && res;
}

//
// Implementation methods
//

NodeDefPtr Implementation::getNodeDef() const
{
    return getDocument()->getNodeDef(getNodeDefString());
}

} // namespace MaterialX
