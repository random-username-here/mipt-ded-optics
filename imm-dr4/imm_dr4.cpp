#include "imm_dr4.hpp"
#include "dr4/window.hpp"
#include <cassert>
#include <memory>
#include <unordered_map>

namespace dr4_imm {

struct Insts {
    std::unique_ptr<dr4::Line> line;
    std::unique_ptr<dr4::Circle> circ;
    std::unique_ptr<dr4::Rectangle> rect;
    std::unique_ptr<dr4::Text> text;
    std::string str;
    size_t win_refcount;
};

std::unordered_map<dr4::Window*, Insts> insts;

static Insts createInsts(dr4::Window *win) {
    return Insts {
        .line = std::unique_ptr<dr4::Line>(win->CreateLine()),
        .circ = std::unique_ptr<dr4::Circle>(win->CreateCircle()),
        .rect = std::unique_ptr<dr4::Rectangle>(win->CreateRectangle()),
        .text = std::unique_ptr<dr4::Text>(win->CreateText()),
        .win_refcount = 0
    };
}

void _inc_winref(dr4::Window *win) {
    if (!insts.count(win))
        insts[win] = createInsts(win);
    insts[win].win_refcount++;
}

void _dec_winref(dr4::Window *win) {
    assert(insts.count(win));
    insts[win].win_refcount--;
    if (insts[win].win_refcount == 0)
        insts.erase(win);
}

void Imm::Draw(dr4::Texture &tex, const Line &line) {
    assert(insts.count(win));
    auto &inst = insts[win];
    inst.line->SetStart(line.start);
    inst.line->SetEnd(line.end);
    inst.line->SetThickness(line.width);
    inst.line->SetColor(line.color);
    inst.line->DrawOn(tex);
}

void Imm::Draw(dr4::Texture &tex, const Circle &circ) {
    assert(insts.count(win));
    auto &inst = insts[win];
    inst.circ->SetCenter(circ.center);
    inst.circ->SetRadius(circ.radius);
    inst.circ->SetFillColor(circ.fill);
    inst.circ->SetBorderColor(circ.border);
    inst.circ->SetBorderThickness(circ.bw);
    inst.circ->DrawOn(tex);
}

void Imm::Draw(dr4::Texture &tex, const Rectangle &rect) {
    assert(insts.count(win));
    auto &inst = insts[win];
    inst.rect->SetPos(rect.pos);
    inst.rect->SetSize(rect.size);
    inst.rect->SetFillColor(rect.fill);
    inst.rect->SetBorderColor(rect.border);
    inst.rect->SetBorderThickness(rect.bw);
    inst.rect->DrawOn(tex);
}

void Imm::Draw(dr4::Texture &tex, const Text &text) {
    assert(insts.count(win));
    auto &inst = insts[win];
    inst.str = text.text;
    inst.text->SetPos(text.pos);
    inst.text->SetText(inst.str);
    inst.text->SetColor(text.color);
    inst.text->SetFontSize(text.size);
    inst.text->SetVAlign(text.align);
    inst.text->SetFont(text.font);
    inst.text->DrawOn(tex);
}

Vec2f Imm::TextBounds(dr4::Font *font, float size, std::string_view text) {
    assert(insts.count(win));
    auto &inst = insts[win];
    inst.str = text;
    inst.text->SetText(inst.str);
    inst.text->SetFontSize(size);
    inst.text->SetFont(font);
    return inst.text->GetBounds();
}

void Imm::SetTarget(dr4::Texture *tex) { target = tex; }

void Imm::Draw(const Line &line) { Draw(*target, line); }
void Imm::Draw(const Circle &circ) { Draw(*target, circ); }
void Imm::Draw(const Rectangle &rect) { Draw(*target, rect); }
void Imm::Draw(const Text &text) { Draw(*target, text); }


};
