#ifndef I_RTR_OBJ
#define I_RTR_OBJ

#include "math/vec3f.h"
#include "math/ray3f.h"
#include "resfile.hpp"
#include <cmath>
#include <string>

namespace rtr {

class Scene;

struct Material {

    // Phong model
    rgbf baseColor = rgbf(1.0f);
    float ambient = 1.0f;
    float diffuse = 1.0f;
    float specular = 1.0f;
    float emission = 0.0f;
    float shinyness = 32.0f;

    // Percentage of all non-refracted light reflected
    // The other part goes into phong model
    float reflection = 0.0f;

    // Percentage of all incoming light refracted
    float refraction = 0.0f;
    float refractiveIndex = 1.0f;
    rgbf lightScalePerM = rgbf(1.0f); // How much light is scaled when passing
                                      // through 1m of material
};

class SceneAnnotater {
public:
    virtual void line(vec3f a, vec3f b, rgbf color) = 0;
};

enum Place { OUTSIDE, INSIDE };

struct IntersInfo {
    vec3f pos;
    void *extra = nullptr;
};

struct AABB {
    vec3f min = vec3f(INFINITY), max = vec3f(-INFINITY);
    AABB() = default;
    AABB(vec3f min, vec3f max) :min(min), max(max) {}

    bool isNone() const { return isinf(min.x()); }
    void add(vec3f pt) { 
        min = vec3f::min(min, pt);
        max = vec3f::max(max, pt);
    }

};

class Object : public virtual rsf::Resource {

    friend class Scene;
    Material m_material;
    Scene *m_scene;
    bool m_isSelected = false;
    std::string m_name;
    std::string m_id;
    static size_t mg_idIdx;

public:

    Object() { m_id = "scene." + std::to_string(mg_idIdx++); }

    /// ID for use in serialization
    std::string id() const { return m_id; }

    inline const std::string &name() const { return m_name; }
    inline void setName(const std::string &name) { m_name = name; } 

    /// Obtain intersection point of the object and the ray
    virtual bool getIntersection(const ray3f &ray, IntersInfo *hit) const = 0;

    /// Get normal to the object at given point
    /// Called at vector given by `getIntersection()`
    virtual vec3f getNormal(IntersInfo hit, Place place) const = 0;

    /// Get light ray going from this object to given point
    /// Ray start point may be infinite (for sun).
    virtual ray3f getLightRay(vec3f towards, IntersInfo *source) const = 0;

    /// Get light multiplier along given direction
    /// (given by `getLightDirectionTo`)
    virtual float getLightMultiplier(IntersInfo src, vec3f direction) const = 0;

    virtual inline const Material& getMaterial(IntersInfo pos) const { return m_material; }
    inline Material& getCommonMaterial() { return m_material; }
    
    inline void setMaterial(const Material &mat) { m_material = mat; }

    void setSelected(bool selected);
    inline bool isSelected() const { return m_isSelected; }

    inline Scene *scene() const { return m_scene; }

    inline virtual const char* icon() const { return ""; };

    virtual AABB bbox() const { return AABB(); }

    virtual void debugDraw(SceneAnnotater &annot) const;
    virtual ~Object();

    static const char *Type() { return "rtr.object"; } 
    Object(const rsf::ResFile::Params &params);
    virtual void Store(rsf::ResFile::Params &params) const override;
    const char *GetType() const override { return Type(); }
};

};

#endif
