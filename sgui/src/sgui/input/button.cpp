#include "sgui/input/button.hpp"
#include "hui/event.hpp"
#include "imm_dr4.hpp"
#include "sgui/theme.hpp"

namespace sgui {

struct ColorSet {
    Color bg;
    Color border;
    Color text;
};

void Button::Redraw() const {
  
    ColorSet colors = { 
        theme::button::bgColor,
        theme::button::borderColor,
        theme::button::textColor
    };

    if (selected)
        colors = {
            theme::button::bgColor_selected,
            theme::button::borderColor_selected,
            theme::button::textColor_selected
        };
    else if (pressed)
        colors = {
            theme::button::bgColor_press,
            theme::button::borderColor,
            theme::button::textColor
        };
    else if (hovered)
        colors = {
            theme::button::bgColor_hover,
            theme::button::borderColor,
            theme::button::textColor
        };

    Imm().Draw(dr4_imm::Rectangle {
        .pos = Vec2f(0, 0),
        .size = GetSize(),
        .fill = colors.bg,
        .border = colors.border,
        .bw = 1
    });

    Vec2f size = Imm().TextBounds(GetUI()->Font(), theme::button::fontSize, label);

    Imm().Draw(dr4_imm::Text {
        .pos = GetSize() / 2 - Vec2f(size.x / 2, 0),
        .text = label,
        .color = colors.text,
        .size = theme::button::fontSize,
        .align = dr4_imm::VAlign::MIDDLE,
        .font = GetUI()->Font(),
    });
}

hui::EventResult Button::OnMouseDown(hui::MouseButtonEvent &evt) {
    if (GetRect().Contains(evt.pos)) {
        pressed = true;
        ForceRedraw();
        GetUI()->SetCaptured(this);
        if (handler)
            handler();
        return hui::EventResult::HANDLED;
    }
    return  hui::EventResult::UNHANDLED;
}

hui::EventResult Button::OnMouseUp(hui::MouseButtonEvent &evt) {
    if (pressed) {
        pressed = false;
        ForceRedraw();
        GetUI()->SetCaptured(nullptr);
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}

void Button::OnHoverGained() {
    hovered = true;
    ForceRedraw();
}

void Button::OnHoverLost() {
    hovered = false;
    ForceRedraw();
}

Button::Button(hui::UI *ui, std::string_view label) :Widget(ui), label(label) {}

};
