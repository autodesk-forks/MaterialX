//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvPath.h>

namespace MaterialX
{

PrvPath::PrvPath(PrvObjectHandle root, PrvObjectHandle obj) :
    _root(root)
{
    PrvElement* elem = obj->asA<PrvElement>();
    PrvElement* parent = elem->getParent();

    bool rootFound = false;

    // Get the path from child down to parent and then reverse it
    _path.push_back(elem->getName());
    while (parent)
    {
        if (parent == root.get())
        {
            // Stop when reaching the given root.
            rootFound = true;
            break;
        }
        _path.push_back(parent->getName());
        parent = parent->getParent();
    }
    if (!rootFound)
    {
        throw ExceptionRuntimeError("PrvPath constructor: object is not a child to the given root");
    }
    std::reverse(_path.begin(), _path.end());
}

PrvObjectHandle PrvPath::getObject() const
{
    if (!_root || _path.empty())
    {
        return nullptr;
    }

    PrvElement* parent = _root->asA<PrvElement>();
    PrvObjectHandle elem = nullptr;
    size_t i = 0;
    while (parent)
    {
        elem = parent->findChildByName(_path[i++]);
        parent = i < _path.size() ? elem->asA<PrvElement>() : nullptr;
    }

    return elem;
}

}
