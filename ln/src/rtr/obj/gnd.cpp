#include "rtr/obj/gnd.h"
#include "resfile.hpp"
#include "rtr/obj/obj.h"

namespace rtr {

bool GroundPlane::getIntersection(const ray3f &ray, IntersInfo *hit) const {

    if (ray.start().z() < m_z) {
        if (hit != NULL)
            hit->pos = ray.start();
        return true;
    }

    if (ray.dir().z() >= 0)
        return false;

    if (hit != NULL)
        hit->pos = ray.start() + ray.dir() * (m_z - ray.start().z()) / ray.dir().z();
    return true;
}

vec3f GroundPlane::getNormal(IntersInfo hit, Place place) const {
    return place == OUTSIDE ? vec3f(0, 0, 1) : vec3f(0, 0, -1);

}

float GroundPlane::getLightMultiplier(IntersInfo src, vec3f direction) const {

    return 1.0f;
}

ray3f GroundPlane::getLightRay(vec3f towards, IntersInfo *src) const {

    vec3f orig = vec3f(towards.x(), towards.y(), 0);
    if (src)
        src->pos = orig;

    return ray3f(
        orig,
        vec3f(0, 0, 1) // up
    );
}

static const Material LIGHT_MAT = {
    .baseColor = rgbf(0.75f),
    .reflection = 0.5f
};

static const Material DARK_MAT = {
    .baseColor = rgbf(0.5f),
    .reflection = 0.5f
};

inline float l_fmodPositive(float x, float v) {
    float res = fmod(x, v);
    if (res < 0) res += v;
    return res;
}

const Material& GroundPlane::getMaterial(IntersInfo pos) const {

    // checkers
    return (l_fmodPositive(pos.pos.x(), 2) < 1) ^ (l_fmodPositive(pos.pos.y(), 2) < 1) ? LIGHT_MAT : DARK_MAT;
}

GroundPlane::GroundPlane(const rsf::ResFile::Params &params) :Object(params) {
    m_z = rsf::GetFloat(params, "z").value_or(0);
}
void GroundPlane::Store(rsf::ResFile::Params &params) const {
    Object::Store(params);
    params["z"] = m_z;
}

REGISTER_RESOURCE(GroundPlane)

}
