#include "sgui/input/textedit.hpp"
#include "dr4/keycodes.hpp"
#include "hui/event.hpp"
#include "imm_dr4.hpp"
#include "sgui/theme.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace sgui {

void TextEdit::Redraw() const {

    float height = theme::fontSize,
          pad = (GetSize().y - height) / 2;

    bool ok = m_validator(m_value);
    Imm().Draw(dr4_imm::Rectangle {
        .pos = Vec2f(0, 0),
        .size = GetSize(),
        .fill = focused ? theme::textedit::bgColor_active 
            : theme::textedit::bgColor,
        .border = ok ? theme::textedit::borderColor
            : theme::textedit::borderColor_bad,
        .bw = 1
    });

    if (focused) {
        float curPos = Imm().TextBounds(
                GetUI()->Font(), theme::fontSize, 
                m_value.substr(0, m_pos)).x;
        Imm().Draw(dr4_imm::Line {
            .start = Vec2f(pad + curPos, pad),
            .end = Vec2f(pad + curPos, pad + height),
            .color = theme::textedit::textColor,
            .width = 1
        });
    }


    Imm().Draw(dr4_imm::Text {
        .pos = Vec2f(pad, GetSize().y/2),
        .text = m_value,
        .color = theme::textedit::textColor,
        .size = theme::fontSize,
        .align = dr4_imm::VAlign::MIDDLE,
        .font = GetUI()->Font(),
    });
}

void TextEdit::OnFocusGained() { focused = true; ForceRedraw(); puts("focused"); }
void TextEdit::OnFocusLost() { focused = false; ForceRedraw(); puts("unfocus"); }


hui::EventResult TextEdit::OnMouseDown(hui::MouseButtonEvent &evt) {
    if (!GetRect().Contains(evt.pos))
        return hui::EventResult::UNHANDLED;
    if (disabled) return hui::EventResult::HANDLED;
    float lineHeight = theme::fontSize;
    float pad = (GetSize().y - lineHeight) / 2;
    size_t l = 0, r = m_value.size() + 1;
    while (r - l > 1) {
        size_t m = (l + r) / 2;
        if (Imm().TextBounds(GetUI()->Font(), theme::fontSize, m_value.substr(0, m)).x
                < evt.pos.x - GetPos().x - pad) {
            l = m;
        } else {
            r = m;
        }
    }
    m_pos = l;
    ForceRedraw();
    GetUI()->ReportFocus(this);
    return hui::EventResult::HANDLED;
}

hui::EventResult TextEdit::OnText(hui::TextEvent &evt) {
    if (GetUI()->GetFocused() != this)
        return hui::EventResult::UNHANDLED;
    if (disabled) return hui::EventResult::HANDLED;
    m_value.insert(m_pos, evt.text);
    m_pos += strlen(evt.text);
    ForceRedraw();
    if (m_handler && m_validator(m_value)) {
        m_handler(m_value);
    }
    return hui::EventResult::HANDLED;
}

hui::EventResult TextEdit::OnKeyDown(hui::KeyEvent &evt) {
    if (GetUI()->GetFocused() != this)
        return hui::EventResult::UNHANDLED;
    if (disabled) return hui::EventResult::HANDLED;
    // TODO: unicode support
    if (evt.key == dr4::KEYCODE_LEFT) {
        if (m_pos > 0) --m_pos, ForceRedraw();
    } else if (evt.key == dr4::KEYCODE_RIGHT) {
        if (m_pos < m_value.size()) ++m_pos, ForceRedraw();
    } else if (evt.key == dr4::KEYCODE_ESCAPE) {
        //app()->setFocused(nullptr);
        ForceRedraw();
    } else if (evt.key == dr4::KEYCODE_BACKSPACE) {
        if (m_pos > 0) {
            m_value.erase(m_value.begin() + (m_pos - 1));
            --m_pos;
            if (m_handler && m_validator(m_value)) {
                m_handler(m_value);
            }
            ForceRedraw();
        }
    }
    return hui::EventResult::HANDLED;
}

bool TextEdit::NoValidator(const std::string &value) {
    return true;
}

bool TextEdit::NumberValidator(const std::string& value) {
    char *end = nullptr;
    strtof(value.c_str(), &end);
    return end == value.c_str() + value.size();
}

};
