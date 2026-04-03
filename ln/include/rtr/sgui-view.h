#ifndef I_MLS2_RTR_SG_VIEW
#define I_MLS2_RTR_SG_VIEW

#include "dr4/math/color.hpp"
#include "dr4/texture.hpp"
#include "hui/event.hpp"
#include "hui/ui.hpp"
#include "math/ray3f.h"
#include "math/vec3f.h"
#include "rtr/obj/obj.h"
#include "rtr/renderer.h"
#include "sgui/widget.hpp"
#include <pthread.h>
#include <semaphore.h>
#include <atomic>
#include <thread>

namespace sgui {

class RTRView;

class RTRRenderThread {
    friend class RTRView;

    enum class Command { NONE, RESTART, SHUTDOWN, STOP };

    RTRView *view;
    size_t index;
    size_t pixel;
    std::atomic<Command> command = Command::NONE;
    std::atomic<bool> stopped = false;
    std::thread thread;

    /// Compute one frame. Will return early if shouldRestart is set.
    bool Cycle();

    /// Start the thread
    void Run();

    /// Stop thread doing things (when we want to update scene)
    void Stop();

    /// Request thread to start computations from beginning
    void RequestRestart();

    /// Request shutdown. Thread will stop some time afterwards
    void RequestShutdown();

    void Join();

public:
    RTRRenderThread(RTRView *view, size_t idx) 
        :view(view), index(idx), pixel(idx) {}
};

class RTRSceneAnnotater : public rtr::SceneAnnotater {
    friend class RTRView;
    const RTRView *view;
    RTRSceneAnnotater(const RTRView *view) :view(view) {}
    Vec2f project(vec3f vec);
public:
    virtual void line(vec3f a, vec3f b, rgbf color);
};

class RTRView : public Widget {

    friend class RTRRenderThread;
    friend class RTRSceneAnnotater;

    struct RenderingParams {
        vec3f cameraTarget = vec3f(0);
        // relative to target
        float cameraAzimuth = 0, cameraElevation = 0, cameraDistance = 10;   
        float cameraPlaneDistance = 2;
    };

    struct CameraVectors {
        vec3f up, right, forward, origin;
    };

    // Options
    rtr::Scene *scene;
    RenderingParams params;

    // Rendering
    std::vector<size_t> indices;
    std::vector<dr4::Color> framebuffer;
    std::vector<size_t> levmap;
    std::vector<size_t> rev_levmap;
    std::vector<std::unique_ptr<RTRRenderThread>> threads;
    size_t width, height;

    double restartedAt = 0;
    mutable double finishedAt = 0;
    std::unique_ptr<dr4::Image> targetImage;
    mutable bool drawnFull = false;

    // Interaction
    enum class MouseAction {
        NONE, ROTATE, MOVE
    };

    MouseAction m_mouseAction;
    vec3f m_mouseInitialVec;
    vec3f m_mouseStartPos;

    CameraVectors CamVectors() const;
    ray3f RayForPixel(vec3f pos);

    virtual void OnSizeChanged() override;
    virtual void Redraw() const override;
    virtual hui::EventResult OnIdle(hui::IdleEvent &evt) override;

    virtual hui::EventResult OnMouseDown(hui::MouseButtonEvent &evt) override;
    virtual hui::EventResult OnMouseMove(hui::MouseMoveEvent &evt) override;
    virtual hui::EventResult OnMouseUp(hui::MouseButtonEvent &evt) override;
    virtual hui::EventResult OnMouseWheel(hui::MouseWheelEvent &evt) override;

    virtual ~RTRView();

public:

    RTRView(hui::UI *app, rtr::Scene *scene);

    inline void SetAzimuth(float azimuth)     { params.cameraAzimuth = azimuth; }
    inline void SetElevation(float elevation) { params.cameraElevation = elevation; }
    inline void SetDist(float distance)       { params.cameraDistance = distance; }
    inline void SetPlaneDist(float distance)  { params.cameraPlaneDistance = distance; }
    inline void SetTarget(vec3f target)       { params.cameraTarget = target; }

    inline float Azimuth()   { return params.cameraAzimuth; } 
    inline float Elevation() { return params.cameraElevation; } 
    inline float Dist()      { return params.cameraDistance; }
    inline float PlaneDist() { return params.cameraPlaneDistance; }
    inline vec3f Target()    { return params.cameraTarget; }
    size_t ThreadCount() const { return threads.size(); }

    /// Set number of threads. Will restart the threads immidiately.
    void SetThreadCount(size_t nthreads);
    /// Ask threads to begin from start
    void RequestRerender();
};

}

#endif
