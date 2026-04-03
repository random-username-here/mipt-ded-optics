///
/// Common maths for graphics programming
///
#ifndef I_MATH_GFXCOMMON
#define I_MATH_GFXCOMMON

#include "math/vec3f.h"
#include <cmath>

namespace rtr {


///
/// Given normal vector of the surface and direction vector,
/// reflect that direction vector from the surface
///
///          normal
///            ↑       
///        ╲   │   ↑
///         ╲ α│α ╱
///     dir  ╲-│-╱ result
///           ↓│╱
///     ──────────────
///         surface
///
vec3f reflect(vec3f dir, vec3f normal);

/// Square some number
inline float sqr(float x) { return x * x; }

inline float sigmoid(float x, float hardness) {
    return 1 / (1 + expf(- x * hardness));
}

};

#endif
