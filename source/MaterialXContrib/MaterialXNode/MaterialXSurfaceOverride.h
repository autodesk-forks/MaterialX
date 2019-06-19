#ifndef MaterialXSurfaceOverride_H
#define MaterialXSurfaceOverride_H

#include "../OGSXMLFragmentWrapper.h"
#include <MaterialXCore/Document.h>
#include "MaterialXShadingNodeImpl.h"

#include <maya/MPxSurfaceShadingNodeOverride.h>

class MaterialXSurfaceOverride
    : public MaterialXShadingNodeImpl<MHWRender::MPxSurfaceShadingNodeOverride>
{
  public:
    static MHWRender::MPxSurfaceShadingNodeOverride* creator(const MObject&);

    static const MString REGISTRANT_ID, DRAW_CLASSIFICATION;

  private:
    using MaterialXShadingNodeImpl<MHWRender::MPxSurfaceShadingNodeOverride>::MaterialXShadingNodeImpl;
};

#endif /* MATERIALX_NODE_OVERRIDE_H */
