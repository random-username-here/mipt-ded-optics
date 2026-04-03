#ifndef I_RTR_RENDERER
#define I_RTR_RENDERER

#include "math/ray3f.h"
#include "math/vec3f.h"
#include "rtr/obj/obj.h"
#include "sgui/event.h"
#include <vector>
#include <optional>

namespace rtr {

struct RenderingOptions {

    enum ShadowMode { NO_SHADOWS, SPHERE_SOFT_SHADOWS, RAYCASTED_SHADOWS };

    size_t maxRayBounces = 0;  // Set to 0 to disable reflections & refractions
    rgbf ambientColor = rgbf(1.0f);
    float ambientIntensity = 0.0f;
    rgbf skyColor = rgbf(0.0f);
    bool disableShading = false; // Plot object base color
    ShadowMode shadows = SPHERE_SOFT_SHADOWS;
    float gammaCorrection = 2.2f; // Gamma correction factor
    float exitDistance = 0.01f; // Distance to offset recasted ray points away from the surface
                                // to not hit the same object again
};


class Scene {

public:
    struct Hit {
        Object *object;
        IntersInfo info;
    };

private:


    // TODO: stack of objects we are inside of
    struct ShadeQuery {
        ray3f ray;
        size_t bouncesLeft;
        Object *inside = NULL;
        bool direct = true;
    };

    std::vector<Object*> m_objects;
    RenderingOptions m_options;

    float raycastedShadow(const Hit& hit, const Object *lightSource);
    float sphereSoftShadow(const Hit& hit, const Object *lightSource);

    rgbf litBySource(const Hit& hit, const Object *lightSource, ray3f cameraRay);
    rgbf shade(const ShadeQuery &query);

public:

    sgui::Event<> onSelectionChange;
    sgui::Event<Object*> onObjectAdded;
    sgui::Event<Object*> onObjectDeleted;
    sgui::Event<Object*> onObjectFieldsChange;
    sgui::Event<bool> onChangeNotify;

    void add(Object *obj);

    void clear();

    template<typename T, typename ...Args>
    T* create(Args ...args) {
        T* obj = new T(args...);
        add(obj);
        return obj;
    }
    
    void destroy(Object *object);

    inline const std::vector<Object*> objects() const { return m_objects; }

    std::optional<Hit> getRayHit(ray3f ray);
    rgbf raytrace(ray3f ray);

    inline RenderingOptions &renderingOptions() { return m_options; } 

    void deselectAll();

    ~Scene();
};


}

#endif
