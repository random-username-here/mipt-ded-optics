///
/// XY plane, ABOVE which camera is placed
///
#ifndef I_RTR_XYPLANE
#define I_RTR_XYPLANE

#include "rtr/obj/obj.h"

namespace rtr {

class GroundPlane : public Object, public rsf::ResourceImpl<GroundPlane> {

    float m_z;

public:

    inline GroundPlane(float z = 0) {
        m_z = z;
    }
    

    inline float z() const { return m_z; }
    inline void setZ(float z) { m_z = z; }

    bool getIntersection(const ray3f &ray, IntersInfo *hit) const;
    vec3f getNormal(IntersInfo hit, Place outside) const;
    float getLightMultiplier(IntersInfo src, vec3f direction) const;
    ray3f getLightRay(vec3f towards, IntersInfo *src) const;
    const Material& getMaterial(IntersInfo pos) const;

    inline virtual const char* icon() const { return "󰄺"; };
    static const char *Type() { return "rtr.ground"; } 
    GroundPlane(const rsf::ResFile::Params &params);
    virtual void Store(rsf::ResFile::Params &params) const override;
    const char *GetType() const override { return Type(); }

};

};

#endif
