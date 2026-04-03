#include "rtr/obj/sphere.h"
#include "resfile.hpp"
#include "rtr/obj/obj.h"

namespace rtr {

bool Sphere::getIntersection(const ray3f &ray, IntersInfo *hit) const {

    vec3f projection = ray.project(m_center);

    float radius2 = m_radius * m_radius;
    float dist2 = (projection - m_center).len2();
    if (dist2 > radius2)
        return false;


    vec3f off = ray.dir() * sqrtf(radius2 - dist2);
    vec3f firstHit = projection - off;

    if (vec3f::dot(firstHit - ray.start(), ray.dir()) > 0) {
        hit->pos = firstHit;
        return true;
    }

    vec3f secondHit = projection + off;
    if (vec3f::dot(secondHit - ray.start(), ray.dir()) > 0) {
        hit->pos = secondHit;
        return true;
    }

    return false;
}

vec3f Sphere::getNormal(IntersInfo hit, Place place) const {
    vec3f local = hit.pos - m_center;
    if (place == OUTSIDE) // we are in direction to sphere
        return local.norm();
    else
        return -local.norm();
}

float Sphere::getLightMultiplier(IntersInfo src, vec3f direction) const {

    return 1.0f;
}

ray3f Sphere::getLightRay(vec3f towards, IntersInfo *src) const {
    vec3f dir = (towards - m_center).norm();
    vec3f pos = m_center + dir * m_radius;
    if (src) src->pos = pos;
    return ray3f(pos, dir);
}

Sphere::Sphere(const rsf::ResFile::Params &params) :Object(params) {
    m_center = vec3f(
        rsf::GetFloat(params, "x").value_or(0),
        rsf::GetFloat(params, "y").value_or(0),
        rsf::GetFloat(params, "z").value_or(0)
    );
    m_radius = rsf::GetFloat(params, "r").value_or(1);
}

void Sphere::Store(rsf::ResFile::Params &params) const {
    Object::Store(params);
    params["x"] = m_center.x();
    params["y"] = m_center.y();
    params["z"] = m_center.z();
    params["r"] = m_radius;
}
REGISTER_RESOURCE(Sphere)

};
