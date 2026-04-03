#pragma once

#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "pp/canvas.hpp"
#include "pp/shape.hpp"
#include <cmath>
#include <cstdlib>
#include <functional>
#include "imm_dr4.hpp"

using dr4::Vec2f;

const float triggerRadius = 4.0f;
const float handleRadius = 5.0f;
const float handleTriggerRadius = 7.0f;
const float frameOffset = 10.0f;

// UI vectors were not meant for geometry
inline float Vec2f_dot(Vec2f a, Vec2f b) { return a.x * b.x + a.y * b.y; }
inline float Vec2f_len2(Vec2f a) { return Vec2f_dot(a, a); }
inline float Vec2f_len(Vec2f a) { return std::sqrt(Vec2f_len2(a)); }
inline Vec2f Vec2f_min(Vec2f a, Vec2f b) { return Vec2f(std::min(a.x, b.x), std::min(a.y, b.y)); }
inline Vec2f Vec2f_max(Vec2f a, Vec2f b) { return Vec2f(std::max(a.x, b.x), std::max(a.y, b.y)); }
inline Vec2f Vec2f_abs(Vec2f a) { return Vec2f(std::abs(a.x), std::abs(a.y)); }

inline float sqr(float x) { return x * x; } 

static float DistToSeg2(Vec2f a, Vec2f b, Vec2f p) {
    float ab = Vec2f_len2(a - b);
    float t = std::max(0.0f, std::min(1.0f, Vec2f_dot(p - a, b - a) / ab));
    Vec2f proj = a + t * (b - a);
    return Vec2f_len2(p - proj);
}

struct Handle {

    Vec2f pos;

    /// Thing called when point is moved
    /// Not called if shape if all shape is dragged
    std::function<void(Handle *handle)> callback = nullptr;

    /// A thing which gives point position for given cursor position
    /// For example, point must move horizontally => this returns (x, 0)
    //std::function<Vec2f(Vec2f)> limiter = nullptr;

};

struct AABB {
    Vec2f min = Vec2f(INFINITY, INFINITY), max = Vec2f(-INFINITY, -INFINITY);

    void Add(Vec2f vec) {
        min = Vec2f_min(min, vec);
        max = Vec2f_max(max, vec);
    }
};

class ShapeWithHandles : public pp::Shape {

    Handle *draggedHandle = nullptr;
    Vec2f dragOffset;
    bool fullDrag = false;

    mutable dr4_imm::Imm imm;
    pp::Canvas *cvs;

    virtual bool OnMouseDown(const dr4::Event::MouseButton &evt) override;
    virtual bool OnMouseUp(const dr4::Event::MouseButton &evt) override;
    virtual bool OnMouseMove(const dr4::Event::MouseMove &evt) override;

protected:

    // why are they needed here?
    void SetPos(Vec2f pos) override {}
    Vec2f GetPos() const override { return {0,0}; }

    ShapeWithHandles(pp::Canvas *cvs) :cvs(cvs), imm(cvs->GetWindow()) {}

    virtual void ForEachHandle(std::function<void(const Handle*)> cb) const = 0;
    virtual void ForEachHandle(std::function<void(Handle*)> cb) = 0;
    virtual void DrawOn(dr4::Texture &tex) const override;

    virtual bool WillSelect(Vec2f pos) const = 0;
    virtual AABB BBox() const = 0; 

    pp::Canvas *Cvs() const { return cvs; }
    dr4_imm::Imm &Imm() const { return imm; }
};
