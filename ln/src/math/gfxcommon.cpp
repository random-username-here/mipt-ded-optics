#include "math/gfxcommon.h"
namespace rtr {

vec3f reflect(vec3f dir, vec3f normal) {

    return dir - normal * 2 * vec3f::dot(dir, normal);
}

}
