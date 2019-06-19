#ifndef MaterialXShadingNodeImpl_H
#define MaterialXShadingNodeImpl_H

#include <maya/MViewport2Renderer.h>

template <class BASE>
class MaterialXShadingNodeImpl : public BASE
{
  public:
    MHWRender::DrawAPI supportedDrawAPIs() const override
    {
        return MHWRender::kOpenGL | MHWRender::kOpenGLCoreProfile;
    }

    MString fragmentName() const override;
    //void getCustomMappings(MHWRender::MAttributeParameterMappingList& mappings) override;

    void updateDG() override;

    void updateShader(
        MHWRender::MShaderInstance&,
        const MHWRender::MAttributeParameterMappingList&
    ) override;

    bool valueChangeRequiresFragmentRebuild(const MPlug*) const override
    {
        return false;
    }

  protected:
    ~MaterialXShadingNodeImpl() override;

  private:
    MaterialXShadingNodeImpl(const MObject&);

    MObject _object;

    // Is editing allowed?
    bool _enableEditing = false;
};

#endif /* MATERIALX_NODE_OVERRIDE_H */
