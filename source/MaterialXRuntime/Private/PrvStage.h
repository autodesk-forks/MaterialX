//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PRVSTAGE_H
#define MATERIALX_PRVSTAGE_H

#include <MaterialXRuntime/Private/PrvElement.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PrvStage : public PrvElement
{
public:
    PrvStage(const RtToken& name, const RtToken& sourceUri);

    static PrvObjectHandle createNew(const RtToken& name);

    void initialize();

    const RtToken& getSourceUri() const
    {
        return _sourceUri;
    }

    void setSourceUri(const RtToken& uri)
    {
        _sourceUri = uri;
    }

    void addReference(PrvObjectHandle stage);

    void removeReference(const RtToken& name);

    void removeReferences();

    size_t numReferences() const;

    PrvObjectHandle getReference(size_t index) const;

    PrvObjectHandle findReference(const RtToken& name) const;

    const PrvObjectHandleVec& getReferencedStages() const
    {
        return _refStages;
    }

    PrvObjectHandle findChildByName(const RtToken& name) const override;

    PrvObjectHandle findChildByPath(const string& path) const override;

protected:
    size_t _selfRefCount;
    PrvObjectHandleVec _refStages;
    PrvObjectHandleSet _refStagesSet;
    RtToken _sourceUri;
};

}

#endif
