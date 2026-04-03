#pragma once

#include "dr4/texture.hpp"
#include "dr4/window.hpp"
#include "hui/ui.hpp"
#include "imm_dr4.hpp"

namespace sgui {

class UI : public hui::UI {

    mutable dr4_imm::Imm imm;
    std::unique_ptr<dr4::Font> font;

public:

    UI(dr4::Window *win) :hui::UI(win), imm(win) {
        font = std::unique_ptr<dr4::Font>(win->CreateFont());
        // FIXME!
        font->LoadFromFile("font.ttf");
    }

    dr4_imm::Imm &Imm() const { return imm; };

    dr4::Font *Font() const { return font.get(); }


};

};
