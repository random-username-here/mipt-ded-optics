#include "lenses/ui/scene_tree.h"
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "hui/event.hpp"
#include "imm_dr4.hpp"
#include "sgui/theme.hpp"
#include <cstdio>
#include <cxxabi.h>

#define PAD_LEFT 10
#define ICON_SPACE 10
#define SPACE 5

namespace ln {

using dr4::Vec2f;
using dr4::Color;

Color blend(Color a, Color b, float fac) {
    return Color(
        a.r * fac + b.r * (1 - fac),
        a.g * fac + b.g * (1 - fac),
        a.b * fac + b.b * (1 - fac)
    );
}

void SceneTreeItem::Redraw() const {

    bool selected = m_obj ? m_obj->isSelected() : false;

    dr4_imm::Color bg;
    dr4_imm::Color fg;

    if (selected) {
        bg = pressed ? sgui::theme::item::bgColor_selected_press :
             sgui::theme::item::bgColor_selected;
        fg = sgui::theme::item::textColor;
    } else {
        bg = pressed ? sgui::theme::item::bgColor_press :
             sgui::theme::item::bgColor;
        fg = sgui::theme::item::textColor;
    }

    Imm().Draw(dr4_imm::Rectangle {
        .pos = Vec2f(0, 0),
        .size = GetSize(),
        .fill = bg
    });


    float labelHeight = sgui::theme::fontSize;

    if (m_obj) {
        int stat;
        char *realname = abi::__cxa_demangle(typeid(*m_obj).name(), NULL, NULL, &stat);

        Imm().Draw(dr4_imm::Text {
            .pos = Vec2f(PAD_LEFT, GetSize().y/2),
            .text = m_obj->icon(),
            .color = sgui::theme::theme::titleColor,
            .size = sgui::theme::fontSize,
            .align = dr4_imm::VAlign::MIDDLE,
            .font = GetUI()->Font(),
        });

        float labelW = Imm().TextBounds(
                GetUI()->Font(), sgui::theme::fontSize,
                realname
        ).x;

        Imm().Draw(dr4_imm::Text {
            .pos = Vec2f(
                PAD_LEFT + sgui::theme::fontSize + ICON_SPACE,
                GetSize().y/2
            ),
            .text = realname,
            .color = blend(fg, bg, 0.7),
            .size = sgui::theme::fontSize,
            .align = dr4_imm::VAlign::MIDDLE,
            .font = GetUI()->Font(),
        });

        Imm().Draw(dr4_imm::Text {
            .pos = Vec2f(
                PAD_LEFT + sgui::theme::fontSize + ICON_SPACE + labelW + SPACE,
                GetSize().y/2
            ),
            .text = m_obj->name(),
            .color = fg,
            .size = sgui::theme::fontSize,
            .align = dr4_imm::VAlign::MIDDLE,
            .font = GetUI()->Font(),
        });
        std::free(realname);
    }
}

void SceneTreeItem::updateSelection() {
    if (!m_obj) return;
    if (m_selected != m_obj->isSelected()) {
        m_selected = m_obj->isSelected();
        ForceRedraw();
    }
}

hui::EventResult SceneTreeItem::OnMouseDown(hui::MouseButtonEvent &evt) {
    if (!GetRect().Contains(evt.pos))
        return hui::EventResult::UNHANDLED;

    if (!m_obj)
        return hui::EventResult::HANDLED;

    /*if (evt.mods & sgui::SHIFT) {
        m_obj->setSelected(true);
    } else if (evt.mods & sgui::ALT) {
        m_obj->setSelected(false);
    } else {*/
        m_obj->scene()->deselectAll();
        m_obj->setSelected(true);
    //}
    return hui::EventResult::HANDLED;
}

SceneTree::SceneTree(hui::UI *app, rtr::Scene *scene)
    :Box(app) {
    m_scene = scene;
    refresh();
    scene->onObjectAdded.addListener([this](auto _) { refresh(); });
    scene->onObjectDeleted.addListener([this](auto _) { refresh(); });
    scene->onObjectFieldsChange.addListener([this](auto _) { refreshNames(); });
    scene->onSelectionChange.addListener([this]() { refreshSelected(); });
}

void SceneTree::refreshNames() {
    for (auto &wgt : children) {
        SceneTreeItem *item = dynamic_cast<SceneTreeItem*>(wgt.get());
        if (item)
            item->ForceRedraw();
    }
}

void SceneTree::refreshSelected() {
    for (auto &wgt : children) {
        SceneTreeItem *item = dynamic_cast<SceneTreeItem*>(wgt.get());
        if (item)
            item->updateSelection();
    }
}

void SceneTree::refresh() {
    const float itemHeight = 30;
    children.clear();

    float pos = 0;
    for (auto *obj : m_scene->objects()) {
        SceneTreeItem *item = new SceneTreeItem(GetUI(), obj);
        item->SetPos(0, pos);
        item->SetSize(GetSize().x, itemHeight);
        AddChild(item);
        pos += itemHeight;
    }
    SetSize(GetSize().x, pos);
    ForceRedraw();
}

void SceneTree::OnSizeChanged() {
    for (auto &i : children) {
        i->SetSize(GetSize().x, i->GetSize().y);
        i->ForceRedraw();
    }
}

};

