#pragma once

#include "sgui/widget.hpp"
#include <functional>
#include <string_view>
namespace sgui {

class Button : public Widget {

protected:
    bool pressed = false, hovered = false, selected = false;
    std::string label;
    std::function<void()> handler;

    void Redraw() const override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &evt) override;
    hui::EventResult OnMouseUp(hui::MouseButtonEvent &evt) override;
    void OnHoverGained() override;
    void OnHoverLost() override;

public:

    Button(hui::UI *ui, std::string_view label);

    void SetLabel(std::string_view newLabel) 
        { label = newLabel; ForceRedraw(); }

    void SetHandler(std::function<void()> newHandler)
        { handler = newHandler; }

    void SetSelected(bool sel)
        { selected = sel; ForceRedraw(); }

    bool Selected() const { return selected; }
};

};
