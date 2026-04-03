#include "lenses/ui/toolbox.h"
#include "lenses/ui/tool.h"
#include "rtr/obj/obj.h"
#include "sgui/theme.hpp"
#include "sgui/view/text.hpp"
#include <algorithm>
#include <cxxabi.h>

#define PAD 10
#define INNER_W 400
#define SECTION_GAP 20

namespace ln {

Toolbox::Toolbox(hui::UI *app, rtr::Scene *scene)
    :Box(app), m_scene(scene) {
    scene->onSelectionChange.addListener([this](){ refresh(); });
    refresh();
}

void Toolbox::createToolsFor(rtr::Object* obj, float &ypos, const std::type_info *ti, std::unordered_set<void*> &used) {

    if (used.count((void*) ti))
        return;
    used.insert((void*) ti);

    const __cxxabiv1::__si_class_type_info *si = dynamic_cast<const __cxxabiv1::__si_class_type_info*>(ti);
    if (si)
        createToolsFor(obj, ypos, dynamic_cast<const std::type_info*>(si->__base_type), used);
    
    const __cxxabiv1::__vmi_class_type_info *vmi = dynamic_cast<const __cxxabiv1::__vmi_class_type_info*>(ti);
    if (vmi) {
        // may create multiple tools for virtual classes,
        // but we do not use them here
        for (int i = 0; i < vmi->__base_count; ++i)
            createToolsFor(obj, ypos, dynamic_cast<const std::type_info*>(vmi->__base_info[i].__base_type), used);
    }

    char *realname = abi::__cxa_demangle(ti->name(), NULL, NULL, NULL);
    auto *title = new sgui::Text(GetUI(), realname);
    title->SetColor(sgui::theme::theme::titleColor);
    free(realname);

    title->SetSize(GetSize().x, 30);
    title->SetPos(PAD, ypos);
    ypos += title->GetSize().y;
    AddChild(title);

    const ToolRegistrationBase *reg = toolRegistry().getToolFor(*ti);
    if (reg) {
        auto *tool = reg->createTool(GetUI(), obj);
        tool->SetSize(GetSize().x, tool->GetSize().y);
        tool->SetPos(PAD, ypos);
        AddChild(tool);
        ypos += tool->GetSize().y;
    } else {
        auto notRegisteredMsg = new sgui::Text(GetUI(), "No tool registered");
        notRegisteredMsg->SetColor(dr4::Color(128, 128, 128));
        notRegisteredMsg->SetSize(GetSize().x, 30);
        notRegisteredMsg->SetPos(PAD, ypos);
        AddChild(notRegisteredMsg);
        ypos += notRegisteredMsg->GetSize().y;
    }
    ypos += SECTION_GAP; 
}

void Toolbox::refresh() {

    children.clear();

    size_t numSelected = 0;
    rtr::Object *selected = nullptr;

    for (auto *obj : m_scene->objects()) {
        if (obj->isSelected()) {
            ++numSelected;
            selected = obj;
        }
    }


    if (numSelected != 1) {
        auto text = new sgui::Text(
                GetUI(),
                numSelected ? "Multiple selected" : "Nothing selected"
        );
        text->SetSize(GetSize().x, 30);
        text->SetPos(PAD, PAD);
        SetSize(GetSize().x, 30);
        AddChild(text);
        return;
    }

    float ypos = PAD;

    std::unordered_set<void*> objects;
    createToolsFor(selected, ypos, &typeid(*selected), objects);
    SetSize(GetSize().x, ypos);
}

void Toolbox::OnSizeChanged() {
    for (auto &i : children) {
        i->SetSize(GetSize().x, i->GetSize().y);
        i->ForceRedraw();
    }
}

};
