#include "sgui/box/scrollbox.hpp"
#include "hui/event.hpp"
#include "imm_dr4.hpp"
#include "sgui/input/button.hpp"
#include "sgui/theme.hpp"
#include <memory>

namespace sgui {

hui::EventResult Scroller::OnMouseDown(hui::MouseButtonEvent &evt) {
    if (Button::OnMouseDown(evt) == hui::EventResult::HANDLED) {
        dragOffset = evt.pos - GetPos();
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}

hui::EventResult Scroller::OnMouseMove(hui::MouseMoveEvent &evt) {
    if (pressed) {
        handler((evt.pos - dragOffset).y);
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}

void Scrollbox::SetScroll(float scroll) {
    float childHeight = contents ? contents->GetSize().y : 1;
    if (scroll > childHeight - GetSize().y)
        scroll = childHeight - GetSize().y;
    if (scroll < 0)
        scroll = 0;
    float areaHeight = GetSize().y - theme::scrollbar::width * 2;
    float scrollerHeight = areaHeight * std::min(GetSize().y / childHeight, 1.0f);
    scroller->SetPos(
        GetSize().x - theme::scrollbar::width,
        scroll / childHeight * areaHeight + theme::scrollbar::width
    );
    if (contents)
        contents->SetPos(0, -scroll);
    this->scroll = scroll;
    ForceRedraw();
}

void Scrollbox::Redraw() const {
    GetTexture().Clear(Color(0, 0, 0, 0));
    contents->GetFreshTexture().DrawOn(GetTexture());
    Imm().Draw(dr4_imm::Rectangle {
        .pos = Vec2f(GetSize().x - theme::scrollbar::width, 0),
        .size = Vec2f(theme::scrollbar::width, GetSize().y),
        .fill = theme::scrollbar::bg
    });
    upBtn->GetFreshTexture().DrawOn(GetTexture());
    downBtn->GetFreshTexture().DrawOn(GetTexture());
    scroller->GetFreshTexture().DrawOn(GetTexture());
}

hui::EventResult Scrollbox::PropagateToChildren(hui::Event &event) {
    for (auto i : {
            (hui::Widget*) upBtn.get(),
            (hui::Widget*) downBtn.get(),
            (hui::Widget*) scroller.get(),
            contents.get() 
    }) {
        if (i && event.Apply(*i) == hui::EventResult::HANDLED)
            return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}
    
hui::EventResult Scrollbox::OnMouseWheel(hui::MouseWheelEvent &evt) {
    if (!GetRect().Contains(evt.pos))
        return hui::EventResult::UNHANDLED;
    SetScroll(scroll - evt.delta.y * theme::scrollbar::wheelSpeed);
    ForceRedraw();
    return hui::EventResult::HANDLED;
}

hui::Widget *Scrollbox::SetChild(hui::Widget *wgt) {
    auto v = contents.release();
    if (v) UnbecomeParentOf(v);

    contents = std::unique_ptr<hui::Widget>(wgt);
    BecomeParentOf(wgt);
    OnSizeChanged();
    ForceRedraw();

    return v;
}

Scrollbox::Scrollbox(hui::UI *ui) :Container(ui) {
    upBtn = std::make_unique<Button>(ui, theme::scrollbar::upIcon);
    downBtn = std::make_unique<Button>(ui, theme::scrollbar::downIcon);
    scroller = std::make_unique<Scroller>(ui);

    BecomeParentOf(upBtn.get());
    BecomeParentOf(downBtn.get());
    BecomeParentOf(scroller.get());

    upBtn->SetSize(theme::scrollbar::width, theme::scrollbar::width);
    downBtn->SetSize(theme::scrollbar::width, theme::scrollbar::width);
    SetSize(Vec2f(100, 100));
    upBtn->SetHandler([this]() { SetScroll(scroll - theme::scrollbar::buttonScrollDist); });
    downBtn->SetHandler([this]() { SetScroll(scroll + theme::scrollbar::buttonScrollDist); });
    scroller->SetHandler([this](float y) {
        float scroll = (y - theme::scrollbar::width)
                     / (GetSize().y - theme::scrollbar::width * 2) 
                     * (contents ? contents->GetSize().y : 0);
        SetScroll(scroll);
    });
}
    
void Scrollbox::OnSizeChanged() {
    upBtn->SetPos(
            GetSize().x - theme::scrollbar::width, 0
    );
    downBtn->SetPos(
            GetSize().x - theme::scrollbar::width,
            GetSize().y - theme::scrollbar::width
    );
    float childHeight = contents ? contents->GetSize().y : 1;
    float areaHeight = GetSize().y - theme::scrollbar::width * 2;
    float scrollerHeight = areaHeight * std::min(GetSize().y / childHeight, 1.0f);
    scroller->SetSize(theme::scrollbar::width, scrollerHeight);
    SetScroll(scroll);
    scroller->ForceRedraw();
    if (contents) {
        contents->SetSize(
            std::max<float>(GetSize().x - theme::scrollbar::width, 0),
            contents->GetSize().y
        );
    }
}

hui::EventResult Scrollbox::OnIdle(hui::IdleEvent &evt) {
    // hack
    if (contents && prevChildSize != contents->GetSize().y) {
        OnSizeChanged();
        prevChildSize = contents->GetSize().y;
    }
    return hui::EventResult::HANDLED;
}

};
