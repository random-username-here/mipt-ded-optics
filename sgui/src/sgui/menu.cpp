#include "sgui/menu.hpp"
#include "hui/event.hpp"
#include "imm_dr4.hpp"
#include "sgui/box/box.hpp"
#include "sgui/theme.hpp"

#define ITEM_W 100
#define ITEM_H 25
#define TEXT_OFF 10
#define EXTRA_SPACE 30

namespace sgui {

bool Menu::IsVisible() const {
    if (opener && opener->hover) return true;
    hui::Widget *hover = GetUI()->GetHovered();
    if (!hover) return false;
    sgui::MenuItem *item = dynamic_cast<sgui::MenuItem*>(hover);
    if (!item) return false;
    while (item != nullptr) {
        sgui::Menu *menu = dynamic_cast<sgui::Menu*>(item->GetParent());
        if (!menu) return false;
        if (menu == this) return true;
        item = menu->opener;
    }
    return false;
}

void Menu::Redraw() const {
    visibleOnLastDraw = IsVisible();
    if (visibleOnLastDraw) {
        Imm().Draw(dr4_imm::Rectangle {
                .pos = Vec2f(0, 0),
                .size = GetSize(),
                .fill = theme::menu::bgColor,
                .border = theme::menu::borderColor,
                .bw = 1
        });
        for (int i = items.size()-1; i >= 0; --i)
            items[i]->DrawOn(GetTexture());
    } else {
        GetTexture().Clear(Color(0, 0, 0, 0));
    }
}

void Menu::AddItem(MenuItem *item) {
    BecomeParentOf(item);
    float w = Imm().TextBounds(GetUI()->Font(), theme::fontSize, item->text).x + TEXT_OFF * 2 + EXTRA_SPACE;
    if (w > GetSize().x) {
        SetSize(w, GetSize().y);
        for (auto &i : items)
            i->SetSize(w, i->GetSize().y);
    }
    item->SetPos(0, items.size() * ITEM_H);
    item->SetSize(GetSize().x, ITEM_H);
    items.push_back(std::unique_ptr<MenuItem>(item));
    SetSize(GetSize().x, items.size() * ITEM_H); 
}

hui::EventResult Menu::PropagateToChildren(hui::Event &evt) {
    if (!visibleOnLastDraw) return hui::EventResult::UNHANDLED;
    for (auto &i : items)
        if (evt.Apply(*i.get()) == hui::EventResult::HANDLED)
            return hui::EventResult::HANDLED;
    return hui::EventResult::UNHANDLED;
}

void MenuItem::Redraw() const {
    Color color = theme::menu::textColor;
    bool hasAction = true;
    if (!submenu && !onClick) {
        color = theme::menu::textColor_arrow;
        hasAction = false;
    }
    Imm().Draw(dr4_imm::Rectangle {
        .pos = Vec2f(1, 1),
        .size = GetSize() - Vec2f(3, 3),
        .fill = (hover && hasAction) 
                    ? theme::menu::bgColor_hover 
                    : theme::menu::bgColor
    });

    Imm().Draw(dr4_imm::Text {
        .pos = Vec2f(TEXT_OFF, ITEM_H / 2.0f),
        .text = text,
        .color = color,
        .size = theme::menu::fontSize,
        .align = dr4_imm::VAlign::MIDDLE,
        .font = GetUI()->Font()
    });
    if (submenu && dynamic_cast<Menu*>(GetParent()) != nullptr) {
        const char *arrow = "";
        float w = Imm().TextBounds(
                GetUI()->Font(),
                theme::menu::fontSize, 
                arrow
        ).x;
        Imm().Draw(dr4_imm::Text {
            .pos = Vec2f(GetSize().x - TEXT_OFF - w, ITEM_H / 2.0f),
            .text = arrow,
            .color = theme::menu::textColor_arrow,
            .size = theme::menu::fontSize,
            .align = dr4_imm::VAlign::MIDDLE,
            .font = GetUI()->Font()
        });
    }
}

void MenuItem::OnVisChange() {
    auto *item = this;
    if (submenu) {
        bool parentIsMenu = dynamic_cast<Menu*>(GetParent()) != nullptr; 
        submenu->ForceRedraw();
        submenu->SetPos(GetAbsolutePos()
                + (parentIsMenu ? Vec2f(GetSize().x - 1, 0) : Vec2f(0, GetSize().y - 1)));
        if (auto *box = dynamic_cast<Box*>(submenu->GetParent())) {
            box->BringToFront(submenu);
        }
    }
    while (item != nullptr) {
        sgui::Menu *menu = dynamic_cast<sgui::Menu*>(item->GetParent());
        if (!menu) break;
        menu->ForceRedraw();
        item = menu->opener;
    }
}

void MenuItem::OnHoverGained() {
    hover = true;
    ForceRedraw();
    OnVisChange();
}

void MenuItem::OnHoverLost() {
    hover = false;
    ForceRedraw();
    OnVisChange();
}

hui::EventResult MenuItem::OnMouseDown(hui::MouseButtonEvent &evt) {
    if (GetRect().Contains(evt.pos)) {
        if (onClick)
            onClick();
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}

void Menu::AddItem(std::string_view text, std::function<void()> handler) {
    auto it = new MenuItem(GetUI(), text, handler);
    AddItem(it);
}
Menu* Menu::AddSubmenu(std::string_view text) {
    auto menu = new Menu(GetUI());
    auto it = new MenuItem(GetUI(), text, menu);
    AddItem(it);
    return menu;
}

void TopMenu::Redraw() const {
    Imm().Draw(dr4_imm::Rectangle {
            .pos = Vec2f(-1, -1),
            .size = GetSize() + Vec2f(2, 1),
            .fill = theme::menu::bgColor,
            .border = theme::menu::borderColor,
            .bw = 1
    });
    for (int i = items.size()-1; i >= 0; --i)
        items[i]->DrawOn(GetTexture());
}

hui::EventResult TopMenu::PropagateToChildren(hui::Event &evt) {
    for (auto &i : items)
        if (evt.Apply(*i.get()) == hui::EventResult::HANDLED)
            return hui::EventResult::HANDLED;
    return hui::EventResult::UNHANDLED;
}

void TopMenu::AddItem(MenuItem *item) {
    BecomeParentOf(item);
    item->SetPos(items.size() * ITEM_W, 0);
    item->SetSize(ITEM_W, ITEM_H);
    items.push_back(std::unique_ptr<MenuItem>(item));
    SetSize(GetSize().x, ITEM_H);
}
    
Menu* TopMenu::AddSubmenu(std::string_view text) {
    auto menu = new Menu(GetUI());
    auto it = new MenuItem(GetUI(), text, menu);
    AddItem(it);
    return menu;
}

};
