#pragma once
#include "dr4/math/color.hpp"
#include "dr4/math/rect.hpp"
#include "dr4/math/vec2.hpp"
#include "hui/widget.hpp"
#include "hui/container.hpp"
#include "imm_dr4.hpp"
#include "sgui/ui.hpp"

namespace sgui {

using dr4::Color;
using dr4::Vec2f;
using dr4::Rect2f;

class Widget : public hui::Widget {

    using hui::Widget::Widget;

public:
    UI *GetUI() const { return (UI*) hui::Widget::GetUI(); }
protected:
    dr4_imm::Imm &Imm() const {
        auto &imm = ((sgui::UI*) GetUI())->Imm();
        imm.SetTarget(&GetTexture());
        return imm;
    }


    ~Widget() {
        GetUI()->BeforeDelete(this);
    }
};

class Container : public hui::Container {
    using hui::Container::Container;

public:
    UI *GetUI() const { return (UI*) hui::Widget::GetUI(); }

protected:
    dr4_imm::Imm &Imm() const {
        auto &imm = ((sgui::UI*) GetUI())->Imm();
        imm.SetTarget(&GetTexture());
        return imm;
    }
    ~Container() {
        GetUI()->BeforeDelete(this);
    }
};

};
