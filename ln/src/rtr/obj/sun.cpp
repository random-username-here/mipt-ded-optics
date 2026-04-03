#include "rtr/obj/sun.h"
#include "rtr/obj/obj.h"
#include <cassert>

namespace rtr {

bool Sun::getIntersection(const ray3f &ray, IntersInfo *hit) const {
    return false;
}

vec3f Sun::getNormal(IntersInfo hit, Place place) const {
    assert(false);
}

float Sun::getLightMultiplier(IntersInfo src, vec3f direction) const {
    return 1.0f;
}

ray3f Sun::getLightRay(vec3f towards, IntersInfo *src) const {
    src->pos = vec3f(0);
    return ray3f(towards, m_sunDir);
}

Sun::Sun(const rsf::ResFile::Params &params) :Object(params) {
    m_sunDir = vec3f(
        rsf::GetFloat(params, "x").value_or(0),
        rsf::GetFloat(params, "y").value_or(0),
        rsf::GetFloat(params, "z").value_or(0)
    );
}

void Sun::Store(rsf::ResFile::Params &params) const {
    Object::Store(params);
    params["x"] = m_sunDir.x();
    params["y"] = m_sunDir.y();
    params["z"] = m_sunDir.z();
}
REGISTER_RESOURCE(Sun)

};
