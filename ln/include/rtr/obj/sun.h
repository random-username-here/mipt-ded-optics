///
/// The sun
///
#ifndef I_RTR_SUN

#include "rtr/obj/obj.h"

namespace rtr {

class Sun : public Object, public rsf::ResourceImpl<Sun> {

    vec3f m_sunDir;

public:

    inline Sun(vec3f dir = vec3f(-1, 1, -1)) {
        m_sunDir = dir;
    }

    bool getIntersection(const ray3f &ray, IntersInfo *hit) const;
    vec3f getNormal(IntersInfo hit, Place outside) const;
    float getLightMultiplier(IntersInfo src, vec3f direction) const;
    ray3f getLightRay(vec3f towards, IntersInfo *src) const;

    inline virtual const char* icon() const { return ""; };

    static const char *Type() { return "rtr.sun"; } 
    Sun(const rsf::ResFile::Params &params);
    virtual void Store(rsf::ResFile::Params &params) const override;
    const char *GetType() const override { return Type(); }

};

};

#endif
