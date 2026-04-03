#pragma once
#include "rtr/obj/mesh.h"
#include "resfile.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>

namespace rtr {

std::optional<vec3f> rayTriagIntersection(ray3f ray, const Triag &triag, const Transform &tsf, float *tout) {
    constexpr float epsilon = std::numeric_limits<float>::epsilon();

    vec3f edge1 = tsf(triag.b) - tsf(triag.a);
    vec3f edge2 = tsf(triag.c) - tsf(triag.a);
    vec3f ray_cross_e2 = vec3f::cross(ray.dir(), edge2);
    float det = vec3f::dot(edge1, ray_cross_e2);

    if (det > -epsilon && det < epsilon)
        return {};    // This ray is parallel to this triangle.

    float inv_det = 1.0 / det;
    vec3f s = ray.start() - tsf(triag.a);
    float u = inv_det * vec3f::dot(s, ray_cross_e2);

    if ((u < 0 && abs(u) > epsilon) || (u > 1 && abs(u-1) > epsilon))
        return {};

    vec3f s_cross_e1 = vec3f::cross(s, edge1);
    float v = inv_det * vec3f::dot(ray.dir(), s_cross_e1);

    if ((v < 0 && abs(v) > epsilon) || (u + v > 1 && abs(u + v - 1) > epsilon))
        return {};

    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = inv_det * vec3f::dot(edge2, s_cross_e1);

    if (t > epsilon) { // ray intersection
        if (tout) *tout = t;
        return  vec3f(ray.start() + ray.dir() * t);
    } else // This means that there is a line intersection but not a ray intersection.
        return {};
}

bool rayAABBTest(ray3f ray, const AABB &aabb) {
    using std::min;
    using std::max;

    vec3f dirfrac = vec3f(1 / ray.dir().x(), 1 / ray.dir().y(), 1 / ray.dir().z());
    vec3f vmin = (aabb.min - ray.start()) * dirfrac,
          vmax = (aabb.max - ray.start()) * dirfrac;

    float tmin = max(max(min(vmin.x(), vmax.x()), min(vmin.y(), vmax.y())), min(vmin.z(), vmax.z()));
    float tmax = min(min(max(vmin.x(), vmax.x()), max(vmin.y(), vmax.y())), max(vmin.z(), vmax.z()));
    if (tmax < 0) // behind ray start
        return false;
    else if (tmin > tmax) // no intersection
        return false;
    else
        return true;
}

void Mesh::addTriag(vec3f a, vec3f b, vec3f c, vec3f norm) {
    m_triags.push_back({ a, b, c, norm });
    m_aabb.add(a);
    m_aabb.add(b);
    m_aabb.add(c);
}

void loadOBJFace(std::istream &str, int &vi, int &ti, int &ni) {
    vi = -1;
    ti = -1;
    ni = -1;
    char _;
    str >> vi;
    if (str.peek() != '/')
        return;
    str.get();
    if (str.peek() == '/') {
    } else if (isdigit(str.peek())) {
        str >> ti;
    } else {
        return;
    }
    str.get();
    str >> ni;
}

void Mesh::loadFromOBJFile(std::string_view name) {
    m_filename = name;


    std::string name_str(name);
    std::ifstream ifs(name_str);
    if (ifs.fail())
        throw std::runtime_error(strerror(errno));
 
    m_aabb = AABB();
    m_triags.clear();   

    std::vector<vec3f> verts, norms;

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream str(line);
        std::string name;
        str >> name;


        if (name == "v" || name == "vn") {
            float a, b, c;
            str >> a >> b >> c;
            if (name == "v") {
                printf("vertex %zu : %f %f %f\n", verts.size(), a, b, c);
                verts.push_back(vec3f(a, b, c));
            } else {
                printf("norm %zu : %f %f %f\n", norms.size(), a, b, c);
                norms.push_back(vec3f(a, b, c));
            }
        } else if (name == "f") {
            printf("got face\n");
            int av = -1, bv = -1, cv = -1,
                at = -1, bt = -1, ct = -1,
                an = -1, bn = -1, cn = -1;
            loadOBJFace(str, av, at, an);
            loadOBJFace(str, bv, bt, bn);
            str >> std::ws;

            while (isdigit(str.peek())) {
                loadOBJFace(str, cv, ct, cn);
                printf("triag: %d %d %d norm %d\n", av-1, bv-1, cv-1, an-1);
                m_triags.push_back({
                    .a = verts[av-1], .b = verts[bv-1], .c = verts[cv-1],
                    .norm = norms[an-1]
                });
                bv = cv; bt = ct; bn = cn;
                str >> std::ws;
            }
        }
    }
}

bool Mesh::getIntersection(const ray3f &ray, IntersInfo *hit) const {
    if (!rayAABBTest(ray, m_aabb)) return false;
    float bestT = INFINITY;
    for (const auto &i : m_triags) {
        float curT = 0;
        auto ints = rayTriagIntersection(ray, i, transform, &curT);
        if (ints && curT < bestT) {
            bestT = curT;
            if (hit) {
                hit->pos = ints.value();
                hit->extra = (void*) &i;
            }
        }
    }
    return !std::isinf(bestT);
}

vec3f Mesh::getNormal(IntersInfo hit, Place outside) const {
    const Triag *triag = (const Triag*) hit.extra;
    return triag ? triag->norm : vec3f(1, 0, 0);
}

ray3f Mesh::getLightRay(vec3f towards, IntersInfo *src) const {
    if (src) src->pos = vec3f(transform.offset);
    return ray3f(transform.offset, towards - transform.offset);
}

float Mesh::getLightMultiplier(IntersInfo src, vec3f direction) const {
    return 1.0f;
}

Mesh::Mesh(const rsf::ResFile::Params &params) :Object(params) {
    auto name = rsf::GetText(params, "mesh.filename");
    if (name)
        loadFromOBJFile(name.value());
    transform.offset = vec3f(
        rsf::GetFloat(params, "mesh.x").value_or(0),
        rsf::GetFloat(params, "mesh.y").value_or(0),
        rsf::GetFloat(params, "mesh.z").value_or(0)
    );
}
void Mesh::Store(rsf::ResFile::Params &params) const {
    Object::Store(params);
    params["mesh.filename"] = m_filename;
    params["mesh.x"] = transform.offset.x();
    params["mesh.y"] = transform.offset.y();
    params["mesh.z"] = transform.offset.z();
}
REGISTER_RESOURCE(Mesh)


};
