//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTraversal.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtStage.h>

#include <MaterialXRuntime/Private/PvtPort.h>
#include <MaterialXRuntime/Private/PvtRelationship.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtStage.h>

namespace MaterialX
{

namespace
{

static const RtPrimIterator NULL_PRIM_ITERATOR;
static const RtStageIterator NULL_STAGE_ITERATOR;

using StageIteratorStackFrame = std::tuple<PvtStage*, int, int>;

struct StageIteratorData
{
    PvtDataHandle current;
    RtObjectPredicate predicate;
    vector<StageIteratorStackFrame> stack;
};

}

RtPrimIterator::RtPrimIterator(const RtPrim& prim, RtObjectPredicate predicate) :
    _prim(nullptr),
    _current(-1),
    _predicate(predicate)
{
    if (prim)
    {
        _prim = PvtObject::ptr<PvtPrim>(prim);
        ++*this;
    }
}

RtPrim RtPrimIterator::operator*() const
{
    return _prim->getAllChildren()[_current]->hnd();
}

RtPrimIterator& RtPrimIterator::operator++()
{
    while (_prim && ++_current < int(_prim->getAllChildren().size()))
    {
        if (!_predicate || _predicate(_prim->getAllChildren()[_current]->obj()))
        {
            return *this;
        }
    }
    abort();
    return *this;
}

bool RtPrimIterator::isDone() const
{
    return !(_prim && _current < int(_prim->getAllChildren().size()));
}

const RtPrimIterator& RtPrimIterator::end()
{
    return NULL_PRIM_ITERATOR;
}


template<class T>
const RtObjectIterator<T> RtObjectIterator<T>::NULL_ITERATOR;

template<class T>
T RtObjectIterator<T>::operator*() const
{
    PvtObjectVec& data = *static_cast<PvtObjectVec*>(_ptr);
    return T(data[_current]->hnd());
}

template<class T>
RtObjectIterator<T>& RtObjectIterator<T>::operator++()
{
    if (_ptr && ++_current < int(static_cast<PvtObjectVec*>(_ptr)->size()))
    {
        return *this;
    }
    abort();
    return *this;
}

template<class T>
bool RtObjectIterator<T>::isDone() const
{
    return !(_ptr && _current < int(static_cast<PvtObjectVec*>(_ptr)->size()));
}

template class RtObjectIterator<RtObject>;
template class RtObjectIterator<RtInput>;
template class RtObjectIterator<RtOutput>;
template class RtObjectIterator<RtRelationship>;

RtInputIterator::RtInputIterator(const RtObject& obj) :
    RtObjectIterator()
{
    if (obj.isA<RtPrim>())
    {
        PvtPrim* prim = PvtObject::ptr<PvtPrim>(obj);
        _ptr = prim->_inputs.empty() ? nullptr : &prim->_inputs.vec();
    }
    else if (obj.isA<RtOutput>())
    {
        PvtOutput* output = PvtObject::ptr<PvtOutput>(obj);
        _ptr = output->_connections.empty() ? nullptr : &output->_connections;
    }
    ++*this;
}

RtOutputIterator::RtOutputIterator(const RtObject& obj) :
    RtObjectIterator()
{
    if (obj.isA<RtPrim>())
    {
        PvtPrim* prim = PvtObject::ptr<PvtPrim>(obj);
        _ptr = prim->_outputs.empty() ? nullptr : &prim->_outputs.vec();
    }
    ++*this;
}

RtConnectionIterator::RtConnectionIterator(const RtObject& obj) :
    RtObjectIterator()
{
    if (obj.isA<RtOutput>())
    {
        PvtOutput* out = PvtObject::ptr<PvtOutput>(obj);
        _ptr = out->_connections.empty() ? nullptr : &out->_connections;
    }
    else if (obj.isA<RtRelationship>())
    {
        PvtRelationship* rel = PvtObject::ptr<PvtRelationship>(obj);
        _ptr = rel->_targets.empty() ? nullptr : &rel->_targets;
    }
    ++*this;
}

RtRelationshipIterator::RtRelationshipIterator(const RtObject& obj) :
    RtObjectIterator()
{
    if (obj.isA<RtPrim>())
    {
        PvtPrim* prim = PvtObject::ptr<PvtPrim>(obj);
        _ptr = prim->_rel.empty() ? nullptr : &prim->_rel.vec();
    }
    ++*this;
}


RtStageIterator::RtStageIterator() :
    _ptr(nullptr)
{
}

RtStageIterator::RtStageIterator(const RtStagePtr& stage, RtObjectPredicate predicate) :
    _ptr(nullptr)
{
    // Initialize the stack and start iteration to the first element.
    StageIteratorData* data = new StageIteratorData();
    data->current = nullptr;
    data->predicate = predicate;
    data->stack.push_back(std::make_tuple(PvtStage::ptr(stage), -1, -1));

    _ptr = data;
    ++*this;
}

RtStageIterator::RtStageIterator(const RtStageIterator& other) :
    _ptr(nullptr)
{
    if (other._ptr)
    {
        _ptr = new StageIteratorData();
        *static_cast<StageIteratorData*>(_ptr) = *static_cast<StageIteratorData*>(other._ptr);
    }
}

RtStageIterator& RtStageIterator::operator=(const RtStageIterator& other)
{
    if (other._ptr)
    {
        if (!_ptr)
        {
            _ptr = new StageIteratorData();
        }
        *static_cast<StageIteratorData*>(_ptr) = *static_cast<StageIteratorData*>(other._ptr);
    }
    return *this;
}

RtStageIterator::~RtStageIterator()
{
    delete static_cast<StageIteratorData*>(_ptr);
}

bool RtStageIterator::operator==(const RtStageIterator& other) const
{
    return _ptr && other._ptr ?
        static_cast<StageIteratorData*>(_ptr)->current == static_cast<StageIteratorData*>(other._ptr)->current :
        _ptr == other._ptr;
}

bool RtStageIterator::operator!=(const RtStageIterator& other) const
{
    return _ptr && other._ptr ?
        static_cast<StageIteratorData*>(_ptr)->current != static_cast<StageIteratorData*>(other._ptr)->current :
        _ptr != other._ptr;
}

RtPrim RtStageIterator::operator*() const
{
    return static_cast<StageIteratorData*>(_ptr)->current;
}

bool RtStageIterator::isDone() const
{
    return _ptr == nullptr;
}

const RtStageIterator& RtStageIterator::end()
{
    return NULL_STAGE_ITERATOR;
}

RtStageIterator& RtStageIterator::operator++()
{
    while (_ptr)
    {
        StageIteratorData* data = static_cast<StageIteratorData*>(_ptr);

        if (data->stack.empty())
        {
            // Traversal is complete.
            abort();
            return *this;
        }

        StageIteratorStackFrame& frame = data->stack.back();
        PvtStage* stage = std::get<0>(frame);
        int& primIndex = std::get<1>(frame);
        int& stageIndex = std::get<2>(frame);

        bool pop = true;

        if (primIndex + 1 < int(stage->getRootPrim()->getAllChildren().size()))
        {
            data->current = stage->getRootPrim()->getAllChildren()[++primIndex];
            if (!data->predicate || data->predicate(data->current->obj()))
            {
                return *this;
            }
            pop = false;
        }
        else if (stageIndex + 1 < int(stage->getAllReferences().size()))
        {
            PvtStage* refStage = PvtStage::ptr(stage->getAllReferences()[++stageIndex]);
            if (!refStage->getRootPrim()->getAllChildren().empty())
            {
                data->stack.push_back(std::make_tuple(refStage, 0, stageIndex));
                data->current = refStage->getRootPrim()->getAllChildren()[0];
                if (!data->predicate || data->predicate(data->current->obj()))
                {
                    return *this;
                }
                pop = false;
            }
        }

        if (pop)
        {
            data->stack.pop_back();
        }
    }
    return *this;
}

void RtStageIterator::abort()
{
    delete static_cast<StageIteratorData*>(_ptr);
    _ptr = nullptr;
}

}
