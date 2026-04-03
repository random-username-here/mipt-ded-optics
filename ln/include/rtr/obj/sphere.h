#ifndef I_RTR_SPHERE
#define I_RTR_SPHERE

#include "resfile.hpp"
#include "rtr/obj/obj.h"

namespace rtr {

class Sphere : public Object, public rsf::ResourceImpl<Sphere> {

    vec3f m_center;
    float m_radius;

public:

    inline Sphere() :m_center(vec3f(0)), m_radius(1) {}

    inline Sphere(vec3f center, float radius) {
        m_center = center;
        m_radius = radius;
    }

    bool getIntersection(const ray3f &ray, IntersInfo *hit) const;
    vec3f getNormal(IntersInfo hit, Place outside) const;
    float getLightMultiplier(IntersInfo src, vec3f direction) const;
    ray3f getLightRay(vec3f towards, IntersInfo *src) const;

    inline vec3f getCenter() const { return m_center; }
    inline float getRadius() const { return m_radius; }

    inline void setCenter(vec3f center) { m_center = center; }
    inline void setRadius(float radius) { m_radius = radius; }
    AABB bbox() const { return AABB { m_center - vec3f(m_radius), m_center + vec3f(m_radius) }; }

    inline virtual const char* icon() const { return "󱥔"; };

    static const char *Type() { return "rtr.sphere"; } 
    Sphere(const rsf::ResFile::Params &params);
    virtual void Store(rsf::ResFile::Params &params) const override;
    const char *GetType() const override { return Type(); }

};

};

#endif
