#pragma once
#include "dr4/event.hpp"
#include "dr4/texture.hpp"
#include "imm_dr4.hpp"
#include "pp/canvas.hpp"
#include "./base.hpp"
#include "pp/tool.hpp"
#include <cmath>

class Circle : public ShapeWithHandles {

    friend class CircleTool;

    Handle center, radius;

    Circle(pp::Canvas *cvs) :ShapeWithHandles(cvs) {}

    void ForEachHandle(std::function<void (Handle *)> cb) override {
        cb(&center);
        cb(&radius);
    }

    void ForEachHandle(std::function<void (const Handle *)> cb) const override {
        cb(&center);
        cb(&radius);
    }

    void DrawOn(dr4::Texture &tex) const override {
        Imm().SetTarget(&tex);
        Imm().Draw(dr4_imm::Circle {
            .center = center.pos,
            .radius = Vec2f_len(radius.pos - center.pos),
            .border = Cvs()->GetControlsTheme().shapeBorderColor,
            .bw = 1
        });
        ShapeWithHandles::DrawOn(tex);
    }

    bool WillSelect(Vec2f pos) const override {
        float r = Vec2f_len(center.pos - radius.pos);
        float d = Vec2f_len(center.pos - pos);
        return std::abs(r - d) < triggerRadius;
    }
    
    AABB BBox() const override {
        float r = Vec2f_len(radius.pos - center.pos);
        return AABB {
            .min = center.pos - Vec2f(r, r),
            .max = center.pos + Vec2f(r, r),
        };
    }
};

class CircleTool : public pp::Tool {

    pp::Canvas *cvs;
    Circle *partial = nullptr;

public:
    CircleTool(pp::Canvas *cvs) :cvs(cvs) {}

    virtual std::string_view Icon() const { return "󰕖"; };
    virtual std::string_view Name() const { return "Circle"; };

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
            partial = new Circle(cvs);
            partial->center.pos = evt.pos;
            partial->radius.pos = evt.pos;
            cvs->AddShape(partial);
            cvs->SetSelectedShape(partial);
        }
        return true;
    }
    virtual bool OnMouseUp(const dr4::Event::MouseButton &evt) { return false; }
    virtual bool OnMouseMove(const dr4::Event::MouseMove &evt) {
        if (partial) {
            partial->radius.pos = evt.pos;
            cvs->ShapeChanged(partial);
        }
        return false;
    }
    

};
