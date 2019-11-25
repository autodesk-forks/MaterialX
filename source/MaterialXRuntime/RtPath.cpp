//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtPath.h>

#include <MaterialXRuntime/Private/PrvPath.h>

namespace MaterialX
{

RtPath::RtPath(RtObject obj) :
    _ptr(nullptr)
{
    if (obj.hasApi(RtApiType::ELEMENT))
    {
        PrvElement* elem = obj.data()->asA<PrvElement>();
        PrvElement* root = elem->getRoot();
        _ptr = new PrvPath(root->shared_from_this(), elem->shared_from_this());
    }
}

RtPath::~RtPath()
{
    delete static_cast<PrvPath*>(_ptr);
}

bool RtPath::isValid() const
{
    return static_cast<PrvPath*>(_ptr)->isValid();
}

RtObjType RtPath::getObjType() const
{
    return static_cast<PrvPath*>(_ptr)->getObject()->getObjType();
}

bool RtPath::hasApi(RtApiType type) const
{
    return static_cast<PrvPath*>(_ptr)->getObject()->hasApi(type);
}

RtObject RtPath::getObject() const
{
    return RtObject(static_cast<PrvPath*>(_ptr)->getObject());
}

string RtPath::getPathString() const
{
    return static_cast<PrvPath*>(_ptr)->getPathString();
}

void RtPath::push(const RtToken& childName)
{
    return static_cast<PrvPath*>(_ptr)->push(childName);
}

void RtPath::pop()
{
    return static_cast<PrvPath*>(_ptr)->pop();
}

bool RtPath::operator==(const RtPath& other) const
{
    const PrvPath* otherPath = static_cast<PrvPath*>(other._ptr);
    return static_cast<PrvPath*>(_ptr)->operator==(*otherPath);
}

}
