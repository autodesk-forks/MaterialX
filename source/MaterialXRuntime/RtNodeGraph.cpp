//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtObject.h>

#include <MaterialXRuntime/Private/PrvNodeGraph.h>
#include <MaterialXRuntime/Private/PrvStage.h>

namespace MaterialX
{

RtNodeGraph::RtNodeGraph(const RtObject& obj) :
    RtNode(obj)
{
}

RtObject RtNodeGraph::createNew(const RtToken& name, RtObject parent)
{
    PrvObjectHandle nodegraph = PrvNodeGraph::createNew(name);

    if (parent)
    {
        if (!parent.hasApi(RtApiType::STAGE))
        {
            throw ExceptionRuntimeError("Given parent object is not a valid stage");
        }
        parent.data()->asA<PrvStage>()->addChild(nodegraph);
    }

    return RtObject(nodegraph);
}

RtApiType RtNodeGraph::getApiType() const
{
    return RtApiType::NODEGRAPH;
}

void RtNodeGraph::addNode(RtObject node)
{
    return data()->asA<PrvNodeGraph>()->addNode(node.data());
}

size_t RtNodeGraph::numNodes() const
{
    return data()->asA<PrvNodeGraph>()->numChildren();
}

RtObject RtNodeGraph::getNode(size_t index) const
{
    PrvObjectHandle node = data()->asA<PrvNodeGraph>()->getChild(index);
    return RtObject(node);
}

RtObject RtNodeGraph::findNode(const RtToken& name) const
{
    PrvObjectHandle node = data()->asA<PrvNodeGraph>()->findChildByName(name);
    return RtObject(node);
}

RtPort RtNodeGraph::getOutputSocket(size_t index) const
{
    return data()->asA<PrvNodeGraph>()->getOutputSocket(index);
}

RtPort RtNodeGraph::getInputSocket(size_t index) const
{
    return data()->asA<PrvNodeGraph>()->getInputSocket(index);
}

RtPort RtNodeGraph::findOutputSocket(const RtToken& name) const
{
    return data()->asA<PrvNodeGraph>()->findOutputSocket(name);
}

RtPort RtNodeGraph::findInputSocket(const RtToken& name) const
{
    return data()->asA<PrvNodeGraph>()->findInputSocket(name);
}

string RtNodeGraph::asStringDot() const
{
    return data()->asA<PrvNodeGraph>()->asStringDot();
}

}
