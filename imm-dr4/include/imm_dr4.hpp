#pragma once

#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "dr4/window.hpp"
#include <string_view>

namespace dr4_imm {

using Color = dr4::Color;
using Vec2f = dr4::Vec2f;

static const Color transparent = Color(0, 0, 0, 0);
static const Color errorColor = Color(1, 0, 1, 0);

struct Line {
    Vec2f start, end;
    Color color = errorColor;
    float width = 1;
};

struct Circle {
    Vec2f center;
    float radius;
    Color fill = transparent;
    Color border = errorColor;
    float bw = 0;
};

struct Rectangle {
    Vec2f pos;
    Vec2f size;
    Color fill = transparent;
    Color border = errorColor;
    float bw = 0;
};

using VAlign = dr4::Text::VAlign;

struct Text {

    Vec2f pos;
    std::string_view text;
    Color color = errorColor;
    float size = 10;
    VAlign align = VAlign::TOP;
    dr4::Font *font = nullptr;
};

void _inc_winref(dr4::Window *win);
void _dec_winref(dr4::Window *win);

// shared ptrs, here they go
class Imm {
    dr4::Window *win;
    dr4::Texture *target = nullptr;

public:

    Imm(dr4::Window *win) :win(win) { _inc_winref(win); }
    Imm(const Imm &ref) { _inc_winref(win); }
    Imm &operator=(const Imm &ref) { _inc_winref(win); return *this; }
    ~Imm() { _dec_winref(win); }

    void Draw(dr4::Texture &tex, const Line &line);
    void Draw(dr4::Texture &tex, const Circle &circ);
    void Draw(dr4::Texture &tex, const Rectangle &rect);
    void Draw(dr4::Texture &tex, const Text &text);

    void SetTarget(dr4::Texture *tex);
    void Draw(const Line &line);
    void Draw(const Circle &circ);
    void Draw(const Rectangle &rect);
    void Draw(const Text &text);


    Vec2f TextBounds(dr4::Font *font, float size, std::string_view text);
};

};

