#include "rtr/sgui-view.h"
#include "dr4/mouse_buttons.hpp"
#include "hui/event.hpp"
#include "imm_dr4.hpp"
#include "math/ray3f.h"
#include "math/vec3f.h"
#include "rtr/obj/obj.h"
#include "sgui/theme.hpp"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <pthread.h>
#include <unistd.h>
#include <cassert>

// check for restart in 10 pixels
#define RECHECK_INTERVAL 10

// time to redraw after render restart
#define RENDER_RECHECK_TIME 0.01

#define MIN_GOOD_SIZE 4

#define PBAR_HEIGHT 5
#define LINE_EXTENT 5
#define PBAR_COLOR Color(255, 255, 255)
#define TEXT_XOFF 30
#define TEXT_YOFF 5

namespace sgui {

bool RTRRenderThread::Cycle() {
    size_t count = 0;
   
    // clear framebuffer
    //for (size_t px = index; px < view->indices.size(); px += view->threads.size())
    //    view->framebuffer[view->indices[px]] = Color(0, 0, 0);

    // render pixels
    pixel = index;
    while(pixel < view->indices.size()) {
        if (count % RECHECK_INTERVAL == 0 && command.load() != Command::NONE)
            return false;
        size_t pxIndex = view->indices[pixel];
        size_t x = pxIndex % view->width,
               y = pxIndex / view->width;
        rgbf color = view->scene->raytrace(view->RayForPixel(vec3f(x, y)));
        view->framebuffer[pxIndex] = dr4::Color(
            color.r() * 255, color.g() * 255, color.b() * 255
        );
        pixel += view->threads.size();
    }
    return true;
}

void RTRRenderThread::Run() {
    thread = std::thread([this](){
        while (1) {
            printf("%zu: begin render cycle...\n", index);
            bool finished = Cycle();
            printf("%zu: end render cycle, %s\n", index, finished ? "full scene rendered" : "interrupted");
got_command:
            Command cmd = command.load();
            switch (cmd) {
                case Command::RESTART: {
                    printf("%zu: recived restart command\n", index);
                    // wait for other commands
                    command.store(Command::NONE);
                    break;
                }
                case Command::SHUTDOWN:
                    printf("%zu: recived shutdown command\n", index);
                    return;
                /*case Command::STOP:
                    // wait while something else is sent
                    stopped = true;
                    std::atomic_notify_one(&stopped);
                    printf("%zu: recived stop command, waiting\n", index);
                    std::atomic_wait(&command, Command::STOP);
                    printf("%zu: woke up after stop\n", index);
                    stopped = false;
                    goto got_command;*/
                case Command::NONE:
                    if (finished) {
                        printf("%zu: done, waiting for commands\n", index);
                        std::atomic_wait(&command, Command::NONE);
                        goto got_command;
                    }
                    break;
            }
        }
    });
}

void RTRRenderThread::RequestRestart() {
    printf("%zu: sent restart request\n", index);
    command.store(Command::RESTART);
    std::atomic_notify_one(&command);
}
    
void RTRRenderThread::RequestShutdown() {
    printf("%zu: sent shutdown request\n", index);
    command.store(Command::SHUTDOWN);
    std::atomic_notify_one(&command);   
}

void RTRRenderThread::Stop() {
    if (stopped) return;
    printf("%zu: sent stop request\n", index);
    command.store(Command::STOP);
    std::atomic_notify_one(&command); 
    std::atomic_wait(&stopped, false);
    printf("%zu: stop confirmed\n", index);
}

void RTRRenderThread::Join() {
    thread.join();
}

Vec2f RTRSceneAnnotater::project(vec3f vec) {
    auto cam = view->CamVectors();
    vec3f d = vec - cam.origin;
    float scale = view->params.cameraPlaneDistance / vec3f::dot(d, cam.forward),
          right = vec3f::dot(d, cam.right),
          up    = vec3f::dot(d, cam.up);
    if (scale < 1e8) {
        up *= scale;
        right *= scale;
    }
    size_t unit = std::min(view->width, view->height) / 2;
    return Vec2f(
        view->width / 2.0f + right * unit,
        view->height / 2.0f + up * unit
    );
}

void RTRSceneAnnotater::line(vec3f a, vec3f b, rgbf color) {
    view->Imm().Draw(dr4_imm::Line {
        .start = project(a),
        .end = project(b),
        .color = dr4_imm::Color(color.r() * 255, color.g() * 255, color.b() * 255),
        .width = 1
    });
}

RTRView::RTRView(hui::UI *app, rtr::Scene *scene) 
        :Widget(app), scene(scene) {
    assert(scene);

    scene->onObjectAdded.addListener([this](auto _){
        RequestRerender();
    });
    scene->onObjectDeleted.addListener([this](auto _){
        RequestRerender();
    });
    scene->onObjectFieldsChange.addListener([this](auto _){
        RequestRerender();
    });
    scene->onSelectionChange.addListener([this](){
        RequestRerender();
    });
    /*size_t nth;
    scene->onChangeNotify.addListener([this, &nth](bool change){
        // FIXME: this is quite crude way, but STOP seems to
        // deadlock with -O3.
        // It somehow sets the stop confirmed flag, without thread
        // noticing the command.
        // Magic?
        if (change) {
            nth = threads.size();
            SetThreadCount(0);
        } else {
            SetThreadCount(nth);
        }
    });*/

    targetImage = std::unique_ptr<dr4::Image>(app->GetWindow()->CreateImage());
    SetThreadCount(1);
}

void RTRView::SetThreadCount(size_t nthreads) {
    for (auto &i : threads)
        i->RequestShutdown();
    for (auto &i : threads)
        i->Join();
    threads.resize(nthreads);
    for (size_t i = 0; i < nthreads; ++i) {
        threads[i] = std::make_unique<RTRRenderThread>(this, i);
        threads[i]->Run();
    }
}

void RTRView::RequestRerender() {
    for (auto &i : threads)
        i->RequestRestart();
    restartedAt = GetUI()->GetWindow()->GetTime();
    drawnFull = false;
}

RTRView::CameraVectors RTRView::CamVectors() const {
    float sinAz = sinf(params.cameraAzimuth),
          cosAz = cosf(params.cameraAzimuth),
          sinEl = sinf(params.cameraElevation),
          cosEl = cosf(params.cameraElevation);

    vec3f camForward = vec3f(
        - cosAz * cosEl,
        - sinAz * cosEl,
        - sinEl
    );
    vec3f camRight = vec3f(
        - sinAz,
        cosAz,
        0
    );
    vec3f camUp = vec3f(
        cosAz * sinEl,
        sinAz * sinEl,
        - cosEl
    );
    return {
        .up = camUp,
        .right = camRight,
        .forward = camForward,
        .origin = params.cameraTarget - camForward * params.cameraDistance
    };

}

ray3f RTRView::RayForPixel(vec3f pixel) {
    auto cam = CamVectors();
    size_t unit = std::min(width, height) / 2;
    vec3f pos = (pixel - vec3f(width, height) / 2) / unit;
    ray3f cameraRay(
                cam.origin,
                cam.forward * params.cameraPlaneDistance
                + cam.up * pos.y() 
                + cam.right * pos.x());
    return cameraRay;
}


void RTRView::OnSizeChanged() {

    // Temporary stop all threads
    int tcnt = threads.size();
    SetThreadCount(0);

    // Resize it
    targetImage->SetSize(GetSize());

    // Resize indices
    size_t w = GetSize().x, h = GetSize().y;
    width = w; height = h;
    indices.resize(w * h);
    rev_levmap.resize(w * h);
    framebuffer.resize(w * h);
    levmap.clear();

    size_t pow2 = std::log2(std::max(w, h));
    size_t pos = 0;
    for (size_t level = pow2; level != -1; --level) {
        size_t step = 1 << level, pstep = step * 2;
        for (int y = 0; y < h; y += step) {
            for (int x = 0; x < w; x += step) {
                if (level != pow2 && x % pstep == 0 && y % pstep == 0)
                    continue;
                rev_levmap[y * w + x] = pos;
                indices[pos++] = y * w + x;
            }
        }
        levmap.push_back(pos);
    }
    SetThreadCount(tcnt);
}

Color lerp(Color a, Color b, float fac) {
    return Color(
        b.r * fac + a.r * (1 - fac),
        b.g * fac + a.g * (1 - fac),
        b.b * fac + a.b * (1 - fac)
    );
}

void RTRView::Redraw() const {
    size_t minIndex = -1;
    for (auto &i : threads)
        // they only increase, we should be ok
        minIndex = std::min(minIndex, i->pixel);

    if (minIndex == width * height)
        drawnFull = true;

    size_t lev = 0;
    for (size_t i = 0; i < levmap.size(); ++i)
        if (minIndex >= levmap[i])
            lev = i;

    size_t boxSize = 1 << (levmap.size() - lev - 1);
    if (boxSize > MIN_GOOD_SIZE)
        boxSize = MIN_GOOD_SIZE;
    size_t nextSize = boxSize / 2;
    if (nextSize == 0) nextSize = 1;
    
    size_t mask = ~(boxSize - 1);
    
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            size_t basex = x & mask, basey = y & mask;
            size_t last = (basey + nextSize) * width + basex + nextSize;
            size_t cbs = last < rev_levmap.size() && rev_levmap[last] < minIndex ? nextSize : boxSize;
            bool canx = basex + cbs < width,
                 cany = basey + cbs < height;
            float xFrac = (x - basex) * 1.0f / cbs;
            Color topLeft = framebuffer[basey * width + basex];
            Color topRight 
                = canx ? framebuffer[basey * width + basex + cbs] 
                       : topLeft;
            Color bottomLeft
                = cany ? framebuffer[(basey + cbs) * width + basex]
                : topLeft;
            Color bottomRight
                = canx && cany ? framebuffer[(basey + cbs) * width + basex + cbs] 
                : canx ? topRight
                : cany ? bottomLeft
                : topLeft;
            Color top = lerp(topLeft, topRight, xFrac);
            Color bottom = lerp(bottomLeft, bottomRight, xFrac);
            Color fin = lerp(
                top, bottom, (y - basey) * 1.0f / cbs
            );
            targetImage->SetPixel(x, y, fin);
        }
    }
    targetImage->DrawOn(GetTexture());

    RTRSceneAnnotater annot(this);

    auto cam = CamVectors();
    annot.line(vec3f(0), vec3f(1, 0, 0), rgbf(1, 0, 0));
    annot.line(vec3f(0), vec3f(0, 1, 0), rgbf(0, 1, 0));
    annot.line(vec3f(0), vec3f(0, 0, 1), rgbf(0, 0, 1));

    for (auto i : scene->objects()) {
        i->debugDraw(annot);
    }

    // progress bar
    float w = 1.0f * minIndex / height;
    Imm().Draw(dr4_imm::Rectangle {
        .pos = Vec2f(0, height - PBAR_HEIGHT),
        .size = Vec2f(w, PBAR_HEIGHT),
        .fill = PBAR_COLOR
    });

    for (size_t i : levmap) {
        float pos = i * 1.0f / height;

        Imm().Draw(dr4_imm::Rectangle {
            .pos = Vec2f(std::floor(pos), height - PBAR_HEIGHT - LINE_EXTENT),
            .size = Vec2f(1, LINE_EXTENT + PBAR_HEIGHT),
            .fill = PBAR_COLOR
        });
    }

    finishedAt = GetUI()->GetWindow()->GetTime();

    char buf[256];
    int fw = snprintf(nullptr, 0, "%zu", width * height);
    const char *unit = "sec";
    double time = finishedAt - restartedAt;
    if (time > 60) time /= 60, unit = "min";
    if (time > 60) time /= 60, unit = "hours";

    sprintf(buf, "Rendered %*zu of %zu pixels, %10.3lf %s", 
            fw, minIndex, width * height, time, unit);
    Imm().Draw(dr4_imm::Text {
        .pos = Vec2f(TEXT_XOFF, height - PBAR_HEIGHT - LINE_EXTENT - TEXT_YOFF),
        .text = buf,
        .color = PBAR_COLOR,
        .size = theme::fontSize,
        .align = dr4_imm::VAlign::BOTTOM,
        .font = GetUI()->Font(),
    });
}

hui::EventResult RTRView::OnIdle(hui::IdleEvent &evt) {
    if (!drawnFull) {
        double time = GetUI()->GetWindow()->GetTime();
        if (time > restartedAt + RENDER_RECHECK_TIME)
            ForceRedraw();
    }
    return hui::EventResult::HANDLED;
}

RTRView::~RTRView() {
    SetThreadCount(0);
}

static vec3f tv(dr4::Vec2f v) {
    return vec3f(v.x, v.y);
}

hui::EventResult RTRView::OnMouseDown(hui::MouseButtonEvent &evt) {
    if (!GetRect().Contains(evt.pos))
        return hui::EventResult::UNHANDLED;

    if (evt.button == dr4::MouseButtonType::LEFT) {
        // Normal selection
        auto maybeHit = scene->getRayHit(RayForPixel(tv(evt.pos - GetPos())));
        if (!maybeHit) return hui::EventResult::HANDLED;

        /*if (evt.mods & SHIFT) {
            maybeHit->object->setSelected(true);
        } else if (evt.mods & ALT) {
            maybeHit->object->setSelected(false);
        } else {*/
            scene->deselectAll();
            maybeHit->object->setSelected(true);
        //}
    } else if (evt.button == dr4::MouseButtonType::RIGHT) {
        m_mouseAction = MouseAction::MOVE;
        m_mouseStartPos = tv(evt.pos);
        m_mouseInitialVec = params.cameraTarget; 

    } else if (evt.button == dr4::MouseButtonType::MIDDLE) {
        m_mouseAction = MouseAction::ROTATE;
        m_mouseStartPos = tv(evt.pos);
        m_mouseInitialVec = vec3f(-Azimuth(), Elevation());
    }

    return hui::EventResult::HANDLED;
}

hui::EventResult RTRView::OnMouseMove(hui::MouseMoveEvent &evt)  {

    float maxDim = std::max(GetSize().x, GetSize().y);

    if (m_mouseAction == MouseAction::ROTATE) {
        vec3f delta = tv(evt.pos) - m_mouseStartPos;

        vec3f resultVec = m_mouseInitialVec + delta / maxDim * M_PI;
        SetAzimuth(-resultVec.x());
        SetElevation(resultVec.y());
        RequestRerender();

    } else if (m_mouseAction == MouseAction::MOVE) {
        auto cam = CamVectors();
        
        vec3f delta = (tv(evt.pos) - m_mouseStartPos) / maxDim;
        float camDist = params.cameraDistance;

        SetTarget(m_mouseInitialVec - (cam.right * delta.x() + cam.up * delta.y()) * camDist);
        RequestRerender();
    }
    return hui::EventResult::HANDLED;
}

hui::EventResult RTRView::OnMouseUp(hui::MouseButtonEvent &evt) {
    m_mouseAction = MouseAction::NONE;
    GetUI()->SetCaptured(nullptr);
    return hui::EventResult::HANDLED;
}

hui::EventResult RTRView::OnMouseWheel(hui::MouseWheelEvent &evt) {
    const float ZOOM_FACTOR = 0.9;
    float factor = evt.delta.y > 0 ? ZOOM_FACTOR : 1 / ZOOM_FACTOR;
    SetDist(Dist() * factor);
    RequestRerender();
    return hui::EventResult::HANDLED;
}

}
