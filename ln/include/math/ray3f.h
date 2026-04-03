#ifndef I_RTR_MATH_RAY3F
#define I_RTR_MATH_RAY3F

#include "math/vec3f.h"

///
/// A 3D ray
///
class ray3f {

    vec3f m_start, m_dir;

public:

    ray3f() {}

    ray3f(vec3f start, vec3f dir) 
        :m_start(start), m_dir(dir.norm()) {}

    ///
    /// Project a point onto the ray
    ///
    inline vec3f project(vec3f point) const {
        return m_start + vec3f::dot(point - m_start, m_dir) * m_dir;
    }

    inline vec3f dir() const { return m_dir; }
    inline vec3f start() const { return m_start; }
};

#endif
