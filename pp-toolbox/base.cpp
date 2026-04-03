#include "./base.hpp"
#include "dr4/window.hpp"
#include "imm_dr4.hpp"
#include "pp/canvas.hpp"

bool ShapeWithHandles::OnMouseDown(const dr4::Event::MouseButton &evt) {
    bool selected = Cvs()->GetSelectedShape() == this;
    bool clickOnShape = WillSelect(evt.pos);

    ForEachHandle([this, &evt](Handle *h){
        Vec2f delta = h->pos - evt.pos;
        float dist2 = delta.x * delta.x + delta.y * delta.y;
        if (dist2 < handleTriggerRadius * handleTriggerRadius)
            draggedHandle = h;
    });

    if (draggedHandle) {
        dragOffset = draggedHandle->pos - evt.pos;
        cvs->ShapeChanged(this);
    } else if (clickOnShape) {
        dragOffset = -evt.pos;
        fullDrag = true;
    }
    return clickOnShape || draggedHandle != nullptr;
}

bool ShapeWithHandles::OnMouseUp(const dr4::Event::MouseButton &evt) {
    if (draggedHandle) {
        cvs->ShapeChanged(this);
        draggedHandle = nullptr;
        fullDrag = false;
        return true;
    } else if (fullDrag) {
        fullDrag = false;
        return true;
    }
    return false;
}

bool ShapeWithHandles::OnMouseMove(const dr4::Event::MouseMove &evt) {
    if (draggedHandle) {
        draggedHandle->pos = evt.pos + dragOffset;
        if (draggedHandle->callback)
            draggedHandle->callback(draggedHandle);
        cvs->ShapeChanged(this);
        return true;
    } else if (fullDrag) {
        Vec2f delta = evt.pos + dragOffset;
        dragOffset = -evt.pos;
        ForEachHandle([delta](Handle *h){
            h->pos += delta;
        });
        cvs->ShapeChanged(this);
        return true;
    }
    return false;
}

void ShapeWithHandles::DrawOn(dr4::Texture &tex) const {
    if (Cvs()->GetSelectedShape() != this)
        return;

    auto theme = cvs->GetControlsTheme();

    AABB box = BBox();
    Imm().SetTarget(&tex);
    Imm().Draw(dr4_imm::Rectangle {
        .pos = box.min - Vec2f(frameOffset, frameOffset),
        .size = box.max - box.min + Vec2f(frameOffset, frameOffset) * 2,
        .border = theme.selectColor,
        .bw = 1
    });

    ForEachHandle([this, &theme, &tex](const Handle *h){
        Imm().Draw(dr4_imm::Circle {
            .center = h->pos,
            .radius = handleRadius,
            .fill = draggedHandle == h ? theme.handleActiveColor : theme.handleColor
        });
    });
}
