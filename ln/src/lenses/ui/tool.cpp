#include "lenses/ui/tool.h"
#include "dr4/math/color.hpp"
#include "sgui/input/textedit.hpp"
#include "sgui/theme.hpp"
#include <algorithm>
#include <cassert>
#include <typeindex>

namespace ln {

void ToolRegistry::registerTool(const ToolRegistrationBase *registration) {
    assert(registration);
    m_registered[std::type_index(registration->type())] = registration;
}

const ToolRegistrationBase* ToolRegistry::getToolFor(const std::type_info &type) const {
    std::type_index idx = std::type_index(type);
    if (m_registered.count(idx))
        return m_registered.at(idx);
    else
        return nullptr;
}

ToolRegistry& toolRegistry() {
    static ToolRegistry registry;
    return registry;
}

#define LABEL_W 150
#define ACT_LABEL_W 500
#define ENTRY_H 30
#define MIN_W 10
#define GAP 10
#define PAD 10


Tool::Entry Tool::addField(
        std::string_view name,
        std::string_view initial,
        sgui::TextEdit::ValidatorType validator
) {
    float pos = ecount * (ENTRY_H + GAP);
    
    auto *text = new sgui::Text(GetUI(), name);
    text->SetSize(ACT_LABEL_W, ENTRY_H);
    text->SetPos(PAD, pos + ENTRY_H/2.0f - sgui::theme::fontSize/2);
    AddChild(text);

    auto *field = new sgui::TextEdit(GetUI());
    field->SetPos(PAD + LABEL_W, pos);
    field->SetSize(std::max<float>(MIN_W, GetSize().x - PAD * 3 - LABEL_W), ENTRY_H);
    field->SetValue(initial);
    field->SetValidator(validator);
    AddChild(field);
    BringToFront(field);

    ++ecount;
    SetSize(GetSize().x, ecount * (ENTRY_H + GAP));
    
    return Entry { text, field };
}

sgui::Text *Tool::addText(std::string_view str, dr4::Color color) {

    float pos = ecount * (ENTRY_H + GAP);
    
    auto *text = new sgui::Text(GetUI(), str);
    text->SetSize(ACT_LABEL_W, ENTRY_H);
    text->SetPos(PAD, pos + ENTRY_H/2.0f - sgui::theme::fontSize/2);
    text->SetColor(color);
    AddChild(text);

    ++ecount;
    SetSize(GetSize().x, ecount * (ENTRY_H + GAP));
    return text;
}

void Tool::OnSizeChanged() {
    float w = GetSize().x;
    for (auto &i : children) { 
        if (dynamic_cast<sgui::TextEdit*>(i.get())) {
            i->SetSize(std::max<float>(w - LABEL_W - PAD * 3, MIN_W), ENTRY_H);
            i->ForceRedraw();
        }
    }
}
};
