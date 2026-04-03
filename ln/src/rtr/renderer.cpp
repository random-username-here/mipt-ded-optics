#include "rtr/renderer.h"
#include "math/ray3f.h"
#include "math/vec3f.h"
#include "math/gfxcommon.h"
#include "rtr/obj/obj.h"
#include "rtr/obj/sphere.h"
#include "rtr/common.h"
#include <algorithm>
#include <assert.h>
#include <math.h>
#include <optional>
#include <iostream>

#define MAX_REFLECTIONS 2

namespace rtr {

std::optional<Scene::Hit> Scene::getRayHit(ray3f ray) {

    float distance = 0;
    std::optional<Hit> result = std::nullopt;

    for (Object *object : m_objects) {
        IntersInfo hit;
        if (!object->getIntersection(ray, &hit))
            continue;
        float thisDist = vec3f::dot(ray.dir(), hit.pos - ray.start());
        if (result == std::nullopt || thisDist < distance) {
            result = Hit { .object = object, .info = hit };
            distance = thisDist;
        }
    }

    return result;
}


///
/// Compute shadow via raycast
/// 0 means fully shaded, 1 means not shaded
///
float Scene::raycastedShadow(const Hit& hit, const Object *lightSource) {
    
    ray3f lightRay = lightSource->getLightRay(hit.info.pos, nullptr);
    vec3f normal = hit.object->getNormal(hit.info, OUTSIDE);
    ray3f reverseRay = ray3f(
            hit.info.pos + normal * m_options.exitDistance,
            -lightRay.dir()
    );

    std::optional<Hit> maybeLightHit = getRayHit(reverseRay);

    if (!maybeLightHit.has_value()) // We went to infinity, nothing obscures view
        return 1.0f;

    Hit lightHit = maybeLightHit.value();

    if (lightHit.object == lightSource)
        return 1.0; // We hit light source itself
    else if (isinff(lightRay.start().x()))
        return 0.0; // Light at infinity, we hit something
    else if (vec3f::dot(lightHit.info.pos - hit.info.pos, reverseRay.dir()) 
            > vec3f::dot(lightRay.start() - hit.info.pos, reverseRay.dir()))
        return 1.0; // Obscured by object further along the ray than light source
    
    // Some object was hit, it obscures the view
    return 0.0;
}

///
/// Compute smooth shadow, but only cast by spheres
///
float Scene::sphereSoftShadow(const Hit &hit, const Object *lightSource) {

    float intensity = 1.0f;
    ray3f lightRay = lightSource->getLightRay(hit.info.pos, nullptr);

    for (const Object *object : m_objects) {
        if (object == hit.object || object == lightSource)
            continue;

        const Sphere *sphere = dynamic_cast<const Sphere*> (object);
        if (sphere == NULL)
            continue; // this was not a sphere, or this was 

        if (vec3f::dot(lightRay.dir(), sphere->getCenter() - lightRay.start())
            > vec3f::dot(lightRay.dir(), hit.info.pos - lightRay.start()))
            continue; // Sphere is further down the light ray

        vec3f proj = lightRay.project(sphere->getCenter());
        float dist2 = (sphere->getCenter() - proj).len2() 
                    / sqr(sphere->getRadius()) - 1.0f;

        intensity *= sigmoid(dist2, 10);
    }

    return intensity;
}

///
/// Compute color component of given light source hitting the object
///
rgbf Scene::litBySource(
        const Hit& hit, 
        const Object *lightSource,
        ray3f cameraRay
) {

    // Ray from light source to the point
    IntersInfo sourceInfo;
    ray3f ray = lightSource->getLightRay(hit.info.pos, &sourceInfo);

    if (lightSource->getMaterial(sourceInfo).emission == 0.0f) // if explicitly set to zero 
        return rgbf(0); // then this is not a light source, it does not matter

    // Object normal
    vec3f normal = hit.object->getNormal(hit.info, OUTSIDE);

    // Object material & emmiter material
    const Material &mat = hit.object->getMaterial(hit.info);
    const Material &lmat = lightSource->getMaterial(sourceInfo);
  
    // Quadratic fallof 
    // FIXME: this works badly because it is actually related to the area,
    //        and quadratic only works for point lights.
    //
    float intensity = 1.0f;
    /*if (!isinff(ray.start().x())) { // Not applied to the sun
        float dist2 = (hit.pos - ray.start()).len2();
        intensity *= 1 / dist2;
    }*/

    // Emitter brightness
    intensity *= lmat.emission;

    // Compute shadow
    float shadow = 1.0f;

    switch(m_options.shadows) {
        case RenderingOptions::SPHERE_SOFT_SHADOWS:
            shadow = sphereSoftShadow(hit, lightSource);
            break;
        case rtr::RenderingOptions::RAYCASTED_SHADOWS:
            shadow = raycastedShadow(hit, lightSource);
            break;
        case rtr::RenderingOptions::NO_SHADOWS:
            shadow = 1.0f;
            break;
        default:
            assert(false);
    }

    // Compute Phong lighting model
    // Ambient is not included here, because it does not depend on
    // light source and shadows.
    rgbf diffuse = shadow * mat.baseColor * lmat.baseColor * mat.diffuse
                 * max(0.0f, vec3f::dot(normal, -ray.dir()));
    
    vec3f reflectedDir = reflect(ray.dir(), normal);

    rgbf specular = shadow * lmat.baseColor * mat.specular * powf(
            max(0.0f, vec3f::dot(reflectedDir, -cameraRay.dir())),
            mat.shinyness
    );

    return (diffuse + specular) * intensity;
}


///
/// Compute pixel color for given ray cast in the scene
///
rgbf Scene::shade(const ShadeQuery &query) {
 
    ray3f ray = query.ray;

    std::optional<Hit> o_hit = getRayHit(query.ray);
    if (!o_hit)
        return m_options.skyColor;

    Hit hit = *o_hit;
    const Material& mat = hit.object->getMaterial(hit.info);

    if (m_options.disableShading)
        return mat.baseColor; 

    vec3f color(0.0f);

    float phongFactor = (1 - mat.refraction) * (1 - mat.reflection);
    float reflectionFactor = (1 - mat.refraction) * mat.reflection;
    float refractionFactor = mat.refraction;
    
    if (query.bouncesLeft && mat.refraction != 0.0f) {
        
        // Formula from https://amrhmorsy.github.io/blog/2024/RefractionVectorCalculation/
        vec3f L = ray.dir();
        vec3f N = hit.object->getNormal(hit.info, query.inside ? INSIDE : OUTSIDE);
        float LdotN = vec3f::dot(L, N);
        float nL = query.inside ? query.inside->getMaterial(hit.info).refractiveIndex : 1.0f,
              nT = query.inside ? 1.0f : mat.refractiveIndex;

        float k = 1 - sqr(nL) / sqr(nT) * (1 - sqr(LdotN));

        if (k >= 0) {
            // no full internal reflection
            vec3f refractedDir = N * (-nL/nT * LdotN - sqrtf(k)) + nL/nT * L;
            color += refractionFactor * shade(ShadeQuery {
                .ray = ray3f(hit.info.pos + refractedDir * m_options.exitDistance, refractedDir),
                .bouncesLeft = query.bouncesLeft - 1,
                .inside = query.inside == hit.object ? nullptr : hit.object,
                .direct = false
            });
        } else {
            // full internal reflection
            reflectionFactor = mat.reflection;
        }
    }

    if (query.bouncesLeft && mat.reflection != 0.0f) {
        vec3f normal = hit.object->getNormal(hit.info, query.inside ? INSIDE : OUTSIDE);
        ray3f reflectedRay = ray3f(
            hit.info.pos + normal * m_options.exitDistance,
            reflect(ray.dir(), normal)
        );
        color += reflectionFactor * shade(ShadeQuery {
                .ray = reflectedRay,
                .bouncesLeft = query.bouncesLeft-1,
                .inside = query.inside,
                .direct = false
        });
    }

    color += phongFactor * mat.baseColor * mat.ambient * m_options.ambientColor * m_options.ambientIntensity;
    color += phongFactor * mat.emission * mat.baseColor; // for light sources

    for (const Object *object : m_objects)
        if (object != hit.object)
            color += phongFactor * litBySource(hit, object, query.ray);

    if (query.inside) {
        float dist = (ray.start() - hit.info.pos).len();
        rgbf factor = query.inside->getMaterial(hit.info).lightScalePerM;
        rgbf scale = vec3f(
            powf(factor.r(), dist),
            powf(factor.g(), dist),
            powf(factor.b(), dist)
        );
        color *= scale;
    }

    if (query.direct && hit.object->isSelected()) {
        color += rgbf(0, 1.0f, 0);
    }

    return color;
}

///
/// Raytrace given pixel
///
rgbf Scene::raytrace(ray3f ray) {

    vec3f color = shade(ShadeQuery {
        .ray = ray,
        .bouncesLeft = m_options.maxRayBounces,
        .inside = nullptr,
        .direct = true
    });

    // Gamma-correct
    float invGamma = 1 / m_options.gammaCorrection;
    color = vec3f(
        powf(color.r(), invGamma),
        powf(color.g(), invGamma),
        powf(color.b(), invGamma)
    );

    // Clamp
    color = rgbf::min(color, rgbf(1.0));

    return color;
}

void Scene::add(Object *obj) {
    printf("add object: start\n");
    onChangeNotify.trigger(true);
    printf("add object: in guard\n");
    obj->m_scene = this;
    m_objects.push_back(obj);
    printf("add object: out of guard\n");
    onChangeNotify.trigger(false);
    printf("add object: notify change\n");
    onObjectAdded.trigger(obj);
}

void Scene::clear() {
    printf("clearing the scene: start\n");
    for (Object *ob : m_objects)
        onObjectDeleted.trigger(ob);
    printf("clearing the scene: delete notifications sent\n");
    onChangeNotify.trigger(true);
    printf("clearing the scene: in lock guard\n");
    for (Object *ob : m_objects)
        delete ob;
    m_objects.clear();
    printf("clearing the scene: out of lock guard\n");
    onChangeNotify.trigger(false);
}

Scene::~Scene() {
    for (Object *ob : m_objects)
        delete ob;
}

void Scene::deselectAll() {
    for (auto &i : m_objects)
        i->m_isSelected = false;
    onSelectionChange.trigger();
}

void Scene::destroy(Object *object) {
    auto idx = std::find(m_objects.begin(), m_objects.end(), object);
    assert(idx != m_objects.end());
    m_objects.erase(idx);
    if (object->isSelected())
        object->setSelected(false);
    onObjectDeleted.trigger(object);
    // TODO: multithreading -- mutex here
    delete object;
}
};

