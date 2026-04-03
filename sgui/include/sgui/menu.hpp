#pragma once

#include "hui/event.hpp"
#include "sgui/root.hpp"
#include "sgui/widget.hpp"
#include <memory>
#include <string_view>
#include <functional>

namespace sgui {

class MenuItem;

class Menu : public sgui::Container {
    friend class MenuItem;
    MenuItem *opener = nullptr;
    std::vector<std::unique_ptr<MenuItem>> items;
    mutable bool visibleOnLastDraw = false;

    bool IsVisible() const;
    void Redraw() const override;
    hui::EventResult PropagateToChildren(hui::Event &evt) override;
    hui::EventResult OnMouseMove(hui::MouseMoveEvent &evt) override {
        if (!visibleOnLastDraw) return hui::EventResult::UNHANDLED;
        return Container::OnMouseMove(evt);
    };
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &evt) override {
        if (!visibleOnLastDraw) return hui::EventResult::UNHANDLED;
        return Container::OnMouseDown(evt);
    }
    hui::EventResult OnMouseUp(hui::MouseButtonEvent &evt) override {
        if (!visibleOnLastDraw) return hui::EventResult::UNHANDLED;
        return Container::OnMouseUp(evt);
    }
    hui::EventResult OnMouseWheel(hui::MouseWheelEvent &evt) override {
        if (!visibleOnLastDraw) return hui::EventResult::UNHANDLED;
        return Container::OnMouseWheel(evt);
    }

public:
    Menu(hui::UI *ui) :Container(ui) {
        sgui::Root *root = dynamic_cast<sgui::Root*>(GetUI()->GetRoot());
        if (root)
            root->AddChild(this);
    }
    void AddItem(MenuItem *item);

    void AddItem(std::string_view text, std::function<void()> handler);
    Menu* AddSubmenu(std::string_view text);
};

class TopMenu : public sgui::Container {
    std::vector<std::unique_ptr<MenuItem>> items;
    void Redraw() const override;
    hui::EventResult PropagateToChildren(hui::Event &evt) override;

public:
    using Container::Container;
    void AddItem(MenuItem *item);
    Menu* AddSubmenu(std::string_view text);
};

class MenuItem : public sgui::Widget {
    friend class Menu;
    std::string text;
    Menu *submenu = nullptr;
    std::function<void()> onClick;
    bool hover = false;

    void OnVisChange();
    void Redraw() const override;
    void OnHoverGained() override;
    void OnHoverLost() override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &evt) override;

public:

    MenuItem(hui::UI *ui, std::string_view text, Menu *sub)
        :Widget(ui), text(text), submenu(sub) { sub->opener = this; }

    MenuItem(hui::UI *ui, std::string_view text, std::function<void()> handler)
        :Widget(ui), text(text), onClick(handler) {}
};

};
