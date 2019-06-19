#ifndef MaterialXSurfaceOverride_H
#define MaterialXSurfaceOverride_H

#include "../OGSXMLFragmentWrapper.h"
#include <MaterialXCore/Document.h>
#include "MaterialXShadingNodeImpl.h"

#include <maya/MPxSurfaceShadingNodeOverride.h>

/// VP2 surface shading node override
class MaterialXSurfaceOverride
    : public MaterialXShadingNodeImpl<MHWRender::MPxSurfaceShadingNodeOverride>
{
  public:
    static MHWRender::MPxSurfaceShadingNodeOverride* creator(const MObject&);

    static const MString REGISTRANT_ID, DRAW_CLASSIFICATION;

  private:
    /// Inheriting the constructor from the base class.
    using MaterialXShadingNodeImpl<MHWRender::MPxSurfaceShadingNodeOverride>::MaterialXShadingNodeImpl;
};

#endif
