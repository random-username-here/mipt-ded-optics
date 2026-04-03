#include "sgui/view/text.hpp"
#include "imm_dr4.hpp"

namespace sgui {

void Text::Redraw() const {
    GetTexture().Clear(dr4::Color(0, 0, 0, 0));
    Imm().Draw(dr4_imm::Text {
        .pos = Vec2f(0, 0),
        .text = text,
        .color = color,
        .size = size,
        .align = dr4_imm::VAlign::TOP,
        .font = GetUI()->Font(),
    });
}

};
