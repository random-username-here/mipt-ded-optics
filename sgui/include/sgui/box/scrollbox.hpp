#pragma once

#include "hui/event.hpp"
#include "sgui/input/button.hpp"
#include "sgui/widget.hpp"
#include "sgui/theme.hpp"

namespace sgui {

class Scrollbox;

class Scroller : public Button {

    Vec2f dragOffset;
    std::function<void(float)> handler;

    hui::EventResult OnMouseMove(hui::MouseMoveEvent &evt) override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &evt) override;

public:
    Scroller(hui::UI *ui) :Button(ui, theme::scrollbar::scrollerIcon) {}

    void SetHandler(std::function<void(float)> handler)
        { this->handler = handler; }
};

class Scrollbox : public Container {

    std::unique_ptr<Button> upBtn, downBtn;
    std::unique_ptr<Scroller> scroller;
    std::unique_ptr<hui::Widget> contents;
    float scroll = 0, prevChildSize = 0;

    void SetScroll(float scroll);

    void Redraw() const override;
    hui::EventResult PropagateToChildren(hui::Event &event) override;
    hui::EventResult OnMouseWheel(hui::MouseWheelEvent &evt) override;
    hui::EventResult OnIdle(hui::IdleEvent &evt) override;
    void OnSizeChanged() override;

public:

    Scrollbox(hui::UI *ui);
    hui::Widget *SetChild(hui::Widget *wgt);


};

};
