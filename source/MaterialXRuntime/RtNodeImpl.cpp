//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtStrings.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{
namespace
{
    // TODO: We should derive this from a data driven XML schema.
    class PvtNodeImplPrimSpec : public PvtPrimSpec
    {
    public:
        PvtNodeImplPrimSpec()
        {
            addPrimAttribute(RtStrings::DOC, RtType::STRING);
            addPrimAttribute(RtStrings::NODEDEF, RtType::IDENTIFIER);
            addPrimAttribute(RtStrings::TARGET, RtType::IDENTIFIER);
            addPrimAttribute(RtStrings::FILE, RtType::STRING);
            addPrimAttribute(RtStrings::SOURCECODE, RtType::STRING);
            addPrimAttribute(RtStrings::FUNCTION, RtType::STRING);
            addPrimAttribute(RtStrings::FORMAT, RtType::IDENTIFIER);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNodeImpl, "nodeimpl");

RtPrim RtNodeImpl::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtString DEFAULT_NAME("nodeimpl1");
    const RtString primName = name.empty() ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    return primH;
}

const RtPrimSpec& RtNodeImpl::getPrimSpec() const
{
    static const PvtNodeImplPrimSpec s_primSpec;
    return s_primSpec;
}

void RtNodeImpl::setTarget(const RtString& target)
{
    RtTypedValue* attr = createAttribute(RtStrings::TARGET, RtType::IDENTIFIER);
    attr->getValue().asIdentifier() = target;
}

const RtString& RtNodeImpl::getTarget() const
{
    const RtTypedValue* attr = getAttribute(RtStrings::TARGET, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : RtString::EMPTY;
}

void RtNodeImpl::setNodeDef(const RtString& language)
{
    RtTypedValue* attr = createAttribute(RtStrings::NODEDEF, RtType::IDENTIFIER);
    attr->asIdentifier() = language;
}

const RtString& RtNodeImpl::getNodeDef() const
{
    const RtTypedValue* attr = getAttribute(RtStrings::NODEDEF, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : RtString::EMPTY;
}

void RtNodeImpl::setImplName(const RtString& implname)
{
    RtTypedValue* attr = createAttribute(RtStrings::IMPLNAME, RtType::IDENTIFIER);
    attr->asIdentifier() = implname;
}

const RtString& RtNodeImpl::getImplName() const
{
    const RtTypedValue* attr = getAttribute(RtStrings::IMPLNAME, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : RtString::EMPTY;
}

}
