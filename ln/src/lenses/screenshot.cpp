#include "lenses/screenshot.h"
#include "cum/manager.hpp"
#include "dr4/event.hpp"
#include "dr4/keycodes.hpp"
#include "hui/event.hpp"
#include "cum/ifc/pp.hpp"
#include "pp/tool.hpp"
#include "sgui/box/box.hpp"
#include "sgui/box/window.hpp"
#include "sgui/input/button.hpp"
#include "sgui/theme.hpp"
#include "sgui/view/text.hpp"
#include <cassert>
#include <iostream>
#include <memory>

namespace ln {

using dr4::Color;

bool Screenshotter::RouteToTool(dr4::Event &evt) {
    assert(tool);

    switch(evt.type) {
        case dr4::Event::Type::MOUSE_DOWN:
            return tool->OnMouseDown(evt.mouseButton);
        case dr4::Event::Type::MOUSE_UP:
            return tool->OnMouseUp(evt.mouseButton);
        case dr4::Event::Type::MOUSE_MOVE:
            return tool->OnMouseMove(evt.mouseMove);
        case dr4::Event::Type::TEXT_EVENT:
            return tool->OnText(evt.text);
        case dr4::Event::Type::KEY_DOWN:
            return tool->OnKeyDown(evt.key);
        case dr4::Event::Type::KEY_UP:
            return tool->OnKeyUp(evt.key);
        default:
            return false;
    }
}

bool Screenshotter::RouteToShape(pp::Shape *shape, dr4::Event &evt) { 
    assert(shape);
    switch(evt.type) {
        case dr4::Event::Type::MOUSE_DOWN:
            return shape->OnMouseDown(evt.mouseButton);
        case dr4::Event::Type::MOUSE_UP:
            return shape->OnMouseUp(evt.mouseButton);
        case dr4::Event::Type::MOUSE_MOVE:
            return shape->OnMouseMove(evt.mouseMove);
        case dr4::Event::Type::TEXT_EVENT:
            return shape->OnText(evt.text);
        case dr4::Event::Type::KEY_DOWN:
            return shape->OnKeyDown(evt.key);
        case dr4::Event::Type::KEY_UP:
            return shape->OnKeyUp(evt.key);
        default:
            return false;
    }
}

hui::EventResult eres(bool v) {
    return v ? hui::EventResult::HANDLED : hui::EventResult::UNHANDLED;
}

dr4::Event Screenshotter::ToDR4Event(const hui::Event *evt) {
    if (auto se = dynamic_cast<const hui::MouseButtonEvent*>(evt)) {
        return dr4::Event { 
            .type = se->pressed ? dr4::Event::Type::MOUSE_DOWN : dr4::Event::Type::MOUSE_UP,
            .mouseButton = { .button = se->button, .pos = se->pos }
        };
    } else if (auto se = dynamic_cast<const hui::MouseMoveEvent*>(evt)) {
        return dr4::Event {
            .type = dr4::Event::Type::MOUSE_MOVE,
            .mouseMove = { .pos = se->pos, .rel = se->rel }
        };
    } else if (auto se = dynamic_cast<const hui::KeyEvent*>(evt)) {
        return dr4::Event {
            .type = se->pressed ? dr4::Event::Type::KEY_DOWN : dr4::Event::Type::KEY_UP,
            .key = { .sym = se->key, .mods = se->mods }
        };
    } else if (auto se = dynamic_cast<const hui::TextEvent*>(evt)) {
        return dr4::Event {
            .type = dr4::Event::Type::TEXT_EVENT,
            .text = { .unicode = se->text }
        };
    } else {
        return dr4::Event { .type = dr4::Event::Type::UNKNOWN };
    }
    // TODO: scroll & text events
}

hui::EventResult Screenshotter::PropagateToChildren(hui::Event &hevt) {

    if (hevt.Apply(*toolWin.get()) == hui::EventResult::HANDLED)
        return hui::EventResult::HANDLED;

    dr4::Event evt = ToDR4Event(&hevt);

    if (evt.type == dr4::Event::Type::KEY_DOWN) {
        if(evt.key.sym == dr4::KEYCODE_ESCAPE) {
            if (tool && tool->IsCurrentlyDrawing()) {
                tool->OnBreak();
            } else {
                if (tool) {
                    tool->OnEnd();
                    tool = nullptr;
                }
                if (toolButton) {
                    toolButton->SetSelected(false);
                    toolButton = nullptr;
                }
                if (selected) {
                    selected->OnDeselect();
                    selected = nullptr;
                }
            }
            ForceRedraw();
            return hui::EventResult::HANDLED;
        } else if (evt.key.sym == dr4::KEYCODE_DELETE) {
            if (selected) {
                DelShape(selected);
            }
            return hui::EventResult::HANDLED;
        }
    }

    if (tool) {
        bool handled = RouteToTool(evt);
        ForceRedraw();
        return eres(handled);
    }

    if (selected) {
        if (RouteToShape(selected, evt)) {
            ForceRedraw(); // some plugins do not notify about shape changes
            return hui::EventResult::HANDLED;
        }
    }
    if (evt.type == dr4::Event::Type::MOUSE_DOWN) {
        selected = nullptr;
        for (auto &i : shapes) {
            if (i->OnMouseDown(evt.mouseButton)) {
                SetSelectedShape(i.get());
                ForceRedraw();
                return hui::EventResult::HANDLED;
            }
        }
    }
    return hui::EventResult::UNHANDLED;
}

void Screenshotter::Redraw() const {
    GetTexture().Clear(Color(0, 0, 0, 0));
    for (const auto &i : shapes)
        i->DrawOn(GetTexture());
    toolWin->DrawOn(GetTexture());
}

pp::ControlsTheme Screenshotter::GetControlsTheme() const {
    return pp::ControlsTheme {
        .shapeFillColor = Color(200, 200, 200),
        .shapeBorderColor = Color(255, 255, 255),
        .selectColor = Color(0, 255, 0),
        .textColor = Color(255, 255, 255),
        .baseFontSize = 15,
        .handleColor = Color(0, 200, 0),
        .handleHoverColor = Color(0, 230, 0),
        .handleActiveColor = Color(0, 255, 0),
    };
}

void Screenshotter::AddShape(pp::Shape *shape) {
    shapes.push_back(std::unique_ptr<pp::Shape>(shape));
    ForceRedraw();
}

void Screenshotter::DelShape(pp::Shape *shape) {
    if (shape == selected) {
        shape->OnDeselect();
        selected = nullptr;
    }

    for (size_t i = 0; i < shapes.size(); ++i) {
        if (shapes[i].get() == shape) {
            shapes.erase(shapes.begin() + i);
            ForceRedraw();
            return;
        }
    }
    assert(false);
}

void Screenshotter::SetSelectedShape(pp::Shape *shape) {
    if (isSettingSelection) return;
    isSettingSelection = true; // vova's plugin does SetSelectedShape() in OnSelect()
    if (selected == shape) return;
    if (selected) selected->OnDeselect();
    if (shape) shape->OnSelect();
    selected = shape;
    ForceRedraw();
    isSettingSelection = false;
}

#define TOOL_BTN_SIZE 30
#define PADDING 10
#define OFF 30
#define NAME_W 200

Screenshotter::Screenshotter(hui::UI *ui, cum::Manager *mgr, std::function<void()> endCb, sgui::TopMenu *menu) :Container(ui), onEnd(endCb) {

    toolWin = std::make_unique<sgui::Window>(ui, "Annotation tools");
    BecomeParentOf(toolWin.get());

    auto box = new sgui::Box(ui);
    toolWin->SetChild(box);

    float pos = PADDING, ypos = PADDING, maxw = 0;

    auto submenu = menu->AddSubmenu("Annotate");
    submenu->AddItem("Annotation mode only", nullptr);

    auto toolPlugins = mgr->GetAllOfType<cum::PPToolPlugin>();

    float textH = Imm().TextBounds(GetUI()->Font(), sgui::theme::fontSize, "A").y;

    for (auto plugin : toolPlugins) {
        auto pluginMenu = submenu->AddSubmenu(plugin->GetName());

        auto thisTools = plugin->CreateTools(this);
        sgui::Text *name = new sgui::Text(ui, plugin->GetName());
        name->SetSize(NAME_W, TOOL_BTN_SIZE);
        name->SetPos(pos, ypos + TOOL_BTN_SIZE/2.0 - textH/2);
        box->AddChild(name);
        pos += NAME_W + PADDING;

        for (auto &tool : thisTools) {
            auto tptr = tool.get();
            tools.push_back(std::move(tool));
            auto btn = new sgui::Button(ui, tptr->Icon());

            auto handler = [this, tptr, btn](){
                if (this->tool == tptr) return;
                if (this->tool)
                    this->tool->OnEnd();
                if (toolButton)
                    toolButton->SetSelected(false);

                this->tool = tptr;
                toolButton = btn;
                tptr->OnStart();
                btn->SetSelected(true);
            };

            std::string mname = "";
            mname += tptr->Icon();
            mname += "   ";
            mname += tptr->Name();

            pluginMenu->AddItem(mname, handler);

            btn->SetPos(pos, ypos);
            btn->SetSize(TOOL_BTN_SIZE, TOOL_BTN_SIZE);
            box->AddChild(btn);

            btn->SetHandler(handler);
            pos += PADDING + TOOL_BTN_SIZE;
        }
        maxw = std::max(pos, maxw);
        pos = PADDING;
        ypos += PADDING + TOOL_BTN_SIZE;
    }

    auto closeBtn = new sgui::Button(ui, "Close");
    closeBtn->SetHandler(endCb);
    closeBtn->SetPos(pos, ypos);
    closeBtn->SetSize(100, TOOL_BTN_SIZE);
    box->AddChild(closeBtn);
    ypos += PADDING + TOOL_BTN_SIZE;

    toolWin->SetSize(maxw, ypos);
    toolWin->SetPos(OFF, ui->GetRoot()->GetSize().y - ypos - OFF);    
}

};
