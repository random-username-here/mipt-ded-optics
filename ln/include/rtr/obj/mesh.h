#ifndef I_RTR_MESH
#define I_RTR_MESH

#include "math/vec3f.h"
#include "rtr/obj/obj.h"
#include <cmath>
#include <string_view>
#include <vector>

namespace rtr {

struct Triag {
    vec3f a, b, c;
    vec3f norm;
};

struct Transform {
    vec3f offset = vec3f(0);
    vec3f operator()(vec3f v) const { return v + offset; }
};

class Mesh : public Object, public rsf::ResourceImpl<Mesh> {

    std::vector<Triag> m_triags;
    AABB m_aabb;
    std::string m_filename = "";

    void addTriag(vec3f a, vec3f b, vec3f c, vec3f norm);

public:
    
    Transform transform;

    Mesh() {}

    // throws runtime error if file is bad
    void loadFromOBJFile(std::string_view name);

    const std::string &filename() const { return m_filename; }

    bool getIntersection(const ray3f &ray, IntersInfo *hit) const;
    vec3f getNormal(IntersInfo hit, Place outside) const;
    float getLightMultiplier(IntersInfo src, vec3f direction) const;
    ray3f getLightRay(vec3f towards, IntersInfo *src) const;
    AABB bbox() const { return m_aabb; }

    inline virtual const char* icon() const { return "󰕣"; };

    static const char *Type() { return "rtr.mesh"; } 
    Mesh(const rsf::ResFile::Params &params);
    virtual void Store(rsf::ResFile::Params &params) const override;
    const char *GetType() const override { return Type(); }

};

};

#endif
