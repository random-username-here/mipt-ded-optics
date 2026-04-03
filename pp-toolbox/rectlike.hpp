#pragma once
#include "dr4/event.hpp"
#include "dr4/texture.hpp"
#include "imm_dr4.hpp"
#include "pp/canvas.hpp"
#include "./base.hpp"
#include "pp/tool.hpp"
#include <cmath>
#include <string_view>

class RectLike : public ShapeWithHandles {

protected:

    friend class RectTool;

    // those may actually be swapped
    Handle topLeft, topRight, bottomLeft, bottomRight;

public:
    RectLike(pp::Canvas *cvs) :ShapeWithHandles(cvs) {
        auto cb = [this](Handle *dragged) {

#define SAME_XY(sameX, sameY) do { sameX.pos.x = dragged->pos.x; sameY.pos.y = dragged->pos.y; } while(0)
            if      (dragged == &topLeft)     SAME_XY(bottomLeft, topRight);
            else if (dragged == &topRight)    SAME_XY(bottomRight, topLeft);
            else if (dragged == &bottomLeft)  SAME_XY(topLeft, bottomRight);
            else if (dragged == &bottomRight) SAME_XY(topRight, bottomLeft);
#undef SAME_XY
        };
        topLeft.callback = cb; topRight.callback = cb;
        bottomLeft.callback = cb; bottomRight.callback = cb;
    }

    void ForEachHandle(std::function<void (Handle *)> cb) override {
        cb(&topLeft); cb(&topRight);
        cb(&bottomLeft); cb(&bottomRight);
    }

    void ForEachHandle(std::function<void (const Handle *)> cb) const override {
        cb(&topLeft); cb(&topRight);
        cb(&bottomLeft); cb(&bottomRight);
    }

    void DrawOn(dr4::Texture &tex) const override {
        ShapeWithHandles::DrawOn(tex);
    };
    bool WillSelect(Vec2f pos) const override = 0;
    
    AABB BBox() const override {
        return AABB {
            .min = Vec2f_min(topLeft.pos, bottomRight.pos),
            .max = Vec2f_max(topLeft.pos, bottomRight.pos),
        };
    }
};

class RectTool : public pp::Tool {

public:
    
    using CreateFn = std::function<RectLike* (pp::Canvas *)>;

private:

    pp::Canvas *cvs;
    RectLike *partial = nullptr;
    std::string icon, name;
    CreateFn fn;

public:

    RectTool(pp::Canvas *cvs, std::string_view icon, std::string_view name, CreateFn fn) 
        :cvs(cvs), icon(icon), name(name), fn(fn) {}

    virtual std::string_view Icon() const { return icon; };
    virtual std::string_view Name() const { return name; };

    virtual bool IsCurrentlyDrawing() const { return partial; };

    virtual void OnStart() {};

    virtual void OnBreak() {
        if (partial) {
            cvs->DelShape(partial);
            partial = nullptr;
        }
    };
    virtual void OnEnd() {
        OnBreak();
    };

    virtual bool OnMouseDown(const dr4::Event::MouseButton &evt) {
        if (partial) {
            partial = nullptr;
        } else {
            partial = fn(cvs);
            partial->topLeft.pos = evt.pos;
            partial->bottomRight.pos = evt.pos;
            partial->topLeft.callback(&partial->topLeft);
            partial->bottomRight.callback(&partial->bottomRight);

            cvs->AddShape(partial);
            cvs->SetSelectedShape(partial);
        }
        return true;
    }
    virtual bool OnMouseUp(const dr4::Event::MouseButton &evt) { return false; }
    virtual bool OnMouseMove(const dr4::Event::MouseMove &evt) {
        if (partial) {
            partial->bottomRight.pos = evt.pos;
            partial->bottomRight.callback(&partial->bottomRight);
            cvs->ShapeChanged(partial);
        }
        return false;
    }
};

class Rect : public RectLike {
  
public:
    using RectLike::RectLike;

    void DrawOn(dr4::Texture &tex) const override {
        Imm().SetTarget(&tex);
        Imm().Draw(dr4_imm::Rectangle {
            .pos = Vec2f_min(topLeft.pos, bottomRight.pos),
            .size = Vec2f_abs(topLeft.pos - bottomRight.pos),
            .border = Cvs()->GetControlsTheme().shapeBorderColor,
            .bw = 1
        });
        RectLike::DrawOn(tex);
    }

    bool WillSelect(Vec2f pos) const override {
        float min = INFINITY;
        min = std::min(min, DistToSeg2(topLeft.pos, topRight.pos, pos));
        min = std::min(min, DistToSeg2(topLeft.pos, bottomLeft.pos, pos));
        min = std::min(min, DistToSeg2(bottomRight.pos, topRight.pos, pos));
        min = std::min(min, DistToSeg2(bottomRight.pos, bottomLeft.pos, pos));
        return min < sqr(triggerRadius);
    };
};
