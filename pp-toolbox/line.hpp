#pragma once
#include "dr4/texture.hpp"
#include "imm_dr4.hpp"
#include "pp/canvas.hpp"
#include "./base.hpp"
#include "pp/tool.hpp"

class PolyLine : public ShapeWithHandles {

    friend class PolyTool;

    std::vector<Handle> points;

    PolyLine(pp::Canvas *cvs) :ShapeWithHandles(cvs) {

    }

    void ForEachHandle(std::function<void (Handle *)> cb) override {
        for (auto &i : points)
            cb(&i);
    }

    void ForEachHandle(std::function<void (const Handle *)> cb) const override {
        for (auto &i : points)
            cb(&i);
    }

    Handle* AddPoint() {
        points.push_back(Handle {});
        return &points.back();
    }

    void DelLast() {
        if (points.size()) {
            points.pop_back();
        }
    }

    size_t PointCount() const { return points.size(); }

    void DrawOn(dr4::Texture &tex) const override {
        Imm().SetTarget(&tex);
        auto pal = Cvs()->GetControlsTheme();
        for (size_t i = 0; i < points.size()-1; ++i) {
            Imm().Draw(dr4_imm::Line {
                .start = points[i].pos,
                .end = points[i+1].pos,
                .color = pal.shapeBorderColor,
                .width = 1
            });
        }
        ShapeWithHandles::DrawOn(tex);
    }

    bool WillSelect(Vec2f pos) const override {
        for (size_t i = 0; i < points.size()-1; ++i) {
            if (DistToSeg2(points[i].pos, points[i+1].pos, pos) < sqr(triggerRadius))
                return true;
        }
        return false;
    }
 
    AABB BBox() const override {
        AABB box;
        for (auto &i : points)
            box.Add(i.pos);
        return box;
    }
};

class PolyTool : public pp::Tool {

    pp::Canvas *cvs;
    PolyLine *partial = nullptr;
    Handle *end = nullptr;

public:
    PolyTool(pp::Canvas *cvs) :cvs(cvs) {}

    virtual std::string_view Icon() const { return "󰕡"; };
    virtual std::string_view Name() const { return "Polyline"; };

    virtual bool IsCurrentlyDrawing() const { return partial; };

    virtual void OnBreak() {
        if (partial) {
            if (partial->PointCount() == 2)
                cvs->DelShape(partial);
            else
                partial->DelLast();
            partial = nullptr;
            end = nullptr;
        }
    };

    virtual void OnEnd() {
        if (partial) {
            cvs->DelShape(partial);
            partial = nullptr;
            end = nullptr;
        }
    };

    virtual bool OnMouseDown(const dr4::Event::MouseButton &evt) {
        if (!partial) {
            partial = new PolyLine(cvs);
            cvs->AddShape(partial);
            cvs->SetSelectedShape(partial);
            Handle *start = partial->AddPoint();
            start->pos = evt.pos;
        }
        end = nullptr;
        return true;
    }
    virtual bool OnMouseUp(const dr4::Event::MouseButton &evt) { return false; }
    virtual bool OnMouseMove(const dr4::Event::MouseMove &evt) {
        if (partial && !end) 
            end = partial->AddPoint();
        if (end) {
            end->pos = evt.pos;
            cvs->ShapeChanged(partial);
        }
        return false;
    }
};
