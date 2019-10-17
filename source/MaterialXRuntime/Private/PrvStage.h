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

class PrvStage : public PrvCompoundElement
{
public:
    PrvStage(const RtToken& name);

    static PrvObjectHandle create(const RtToken& name);

    void addReference(PrvObjectHandle stage);
    void removeReference(const RtToken& name);

    const PrvObjectHandleVec& getReferencedStages() const
    {
        return _refStages;
    }

    PrvObjectHandle findElementByName(const RtToken& name) const override;
    PrvObjectHandle findElementByPath(const string& path) const override;

protected:
    size_t _selfRefCount;
    PrvObjectHandleVec _refStages;
    PrvObjectHandleSet _refStagesSet;
};

}

#endif
