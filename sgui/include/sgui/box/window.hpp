#pragma once

#include "hui/event.hpp"
#include "hui/ui.hpp"
#include "sgui/widget.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <functional>

namespace sgui {

class Window : public Container {


    std::string title;
    bool dragged;
    std::function<void(Vec2f)> dragger;
    std::unique_ptr<hui::Widget> child;

protected:
    void OnSizeChanged() override;
    void Redraw() const override;
    hui::EventResult PropagateToChildren(hui::Event &event) override;

    hui::EventResult OnMouseDown(hui::MouseButtonEvent &evt) override;
    hui::EventResult OnMouseUp(hui::MouseButtonEvent &evt) override;
    hui::EventResult OnMouseMove(hui::MouseMoveEvent &evt) override;

public:

    Window(hui::UI *ui, std::string_view title = "Untitled");

    void SetTitle(std::string_view newTitle) { title = newTitle; ForceRedraw(); }
    hui::Widget *SetChild(hui::Widget *nchild);

};

};
