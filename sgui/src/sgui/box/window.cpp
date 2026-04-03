#include "sgui/box/window.hpp"
#include "dr4/texture.hpp"
#include "hui/event.hpp"
#include "imm_dr4.hpp"
#include "sgui/theme.hpp"
#include "sgui/box/box.hpp"
#include <string_view>

#define RESIZE_SIZE 10

namespace sgui {

Window::Window(hui::UI *ui, std::string_view title)
    :Container(ui), title(title) 
{
    SetTextureExtents({
            theme::window::borderWidth * 2 + theme::window::titleHeight,
            theme::window::borderWidth,
            theme::window::borderWidth,
            theme::window::borderWidth
    });
}

void Window::Redraw() const {
    Imm().Draw(dr4_imm::Rectangle {
        .pos = Vec2f(
            -theme::window::borderWidth,
            -theme::window::borderWidth * 2 - theme::window::titleHeight
        ),
        .size = Vec2f(
            GetSize().x + theme::window::borderWidth * 2,
            GetSize().y + theme::window::borderWidth * 3 + theme::window::titleHeight
        ),
        .fill = theme::window::bg,
        .border = theme::window::border,
        .bw = 1,
    });
    Imm().Draw(dr4_imm::Rectangle {
        .pos = Vec2f(
            -theme::window::borderWidth,
            -theme::window::borderWidth * 2 - theme::window::titleHeight
        ),
        .size = Vec2f(
            GetSize().x + theme::window::borderWidth * 2,
            theme::window::borderWidth * 2 + theme::window::titleHeight
        ),
        .fill = dragged ? theme::window::titlebarBg_drag : theme::window::titlebarBg,
        .border = theme::window::titlebarBorder,
        .bw = 1
    });
    Imm().Draw(dr4_imm::Text {
        .pos = Vec2f(theme::window::xPad, -theme::window::titleHeight / 2.0),
        .text = title,
        .color = theme::window::titlebarText,
        .size = 13,
        .align = dr4::Text::VAlign::MIDDLE,
        .font = GetUI()->Font()
    });
    if (child)
        child->GetFreshTexture().DrawOn(GetTexture());
}

hui::EventResult Window::PropagateToChildren(hui::Event &event) {
    return child ? event.Apply(*child.get()) : hui::EventResult::UNHANDLED;
}

hui::EventResult Window::OnMouseDown(hui::MouseButtonEvent &evt) {
    Vec2f local = evt.pos - GetPos();

    if (Container::OnMouseDown(evt) == hui::EventResult::HANDLED)
        return hui::EventResult::HANDLED;

    struct DragArea {
        float xmin, ymin, xmax, ymax;
        std::function<void(Vec2f)> action;
    };

    float top = -theme::window::borderWidth*2-theme::window::titleHeight;
    DragArea areas[] = {
        { 
            -theme::window::borderWidth,
            top,
            GetSize().x + theme::window::borderWidth,
            0,
            [this](Vec2f d) { SetPos(GetPos() + d); }
        },
        {
            GetSize().x, GetSize().y,
            GetSize().x + RESIZE_SIZE, GetSize().y + RESIZE_SIZE,
            [this](Vec2f d) { SetSize((GetSize() + d).Clamped(Vec2f(0, 0), Vec2f(1000, 1000))); }
        }
    };

    for (size_t i = 0; i < sizeof(areas) / sizeof(areas[0]); ++i) {
        auto &area = areas[i];
        if (area.xmin <= local.x && local.x < area.xmax
                && area.ymin <= local.y && local.y < area.ymax) {
            dragged = true;
            dragger = area.action;
            GetUI()->SetCaptured(this);
            ForceRedraw();
            Box *root = dynamic_cast<Box*>(GetParent());
            if (root)
                root->BringToFront(this);
            return hui::EventResult::HANDLED;
        }
    }

    return hui::EventResult::UNHANDLED;
}

hui::EventResult Window::OnMouseUp(hui::MouseButtonEvent &evt) {
    if (dragged) {
        dragged = false;
        GetUI()->SetCaptured(nullptr);
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    return Container::OnMouseUp(evt);
}

hui::EventResult Window::OnMouseMove(hui::MouseMoveEvent &evt) {
    if (dragged) {
        if (dragger) {
            dragger(evt.rel);
            ForceRedraw();
        }
        return hui::EventResult::HANDLED;
    }
    return Container::OnMouseMove(evt);
}

hui::Widget *Window::SetChild(hui::Widget *nchild) {
    auto v = child.release();
    if (v) UnbecomeParentOf(v);
    if (nchild) BecomeParentOf(nchild);
    child = std::unique_ptr<hui::Widget>(nchild);
    nchild->SetPos(Vec2f(0, 0));
    nchild->SetSize(GetSize());
    ForceRedraw();
    return v;
}

void Window::OnSizeChanged() {
    if (child)
        child->SetSize(GetSize());
}
}
