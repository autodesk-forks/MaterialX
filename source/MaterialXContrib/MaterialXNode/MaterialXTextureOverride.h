#ifndef MaterialXTextureOverride_H
#define MaterialXTextureOverride_H

#include "../OGSXMLFragmentWrapper.h"
#include <MaterialXCore/Document.h>
#include "MaterialXShadingNodeImpl.h"

#include <maya/MPxShadingNodeOverride.h>

/// VP2 Texture fragment override
class MaterialXTextureOverride
    : public MaterialXShadingNodeImpl<MHWRender::MPxShadingNodeOverride>
{
  public:
    static MHWRender::MPxShadingNodeOverride* creator(const MObject&);

    static const MString REGISTRANT_ID, DRAW_CLASSIFICATION;

  private:
    /// Inheriting the constructor from the base class.
    using MaterialXShadingNodeImpl<MHWRender::MPxShadingNodeOverride>::MaterialXShadingNodeImpl;
};

#endif
