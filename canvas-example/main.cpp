#include "cum/ifc/dr4.hpp"
#include "cum/ifc/pp.hpp"
#include "cum/manager.hpp"
#include "dr4/event.hpp"
#include "dr4/keycodes.hpp"
#include "dr4/texture.hpp"
#include "dr4/window.hpp"
#include "dr4/math/rect.hpp"
#include "pp/canvas.hpp"
#include "pp/tool.hpp"
#include <dirent.h>
#include <iterator>
#include <memory>
#include <cstring>
#include <functional>
#include <cassert>

using dr4::Vec2f;
using dr4::Color;

void LoadPluginsFromDir(cum::Manager &mgr, std::string dirpath) {
    std::unique_ptr<DIR, int(*)(DIR*)> dir(opendir(dirpath.data()), closedir);
    if (!dir) {
        throw cum::Manager::LoadError(std::string("Failed to open directory ")
                + dirpath + ": " + strerror(errno));
    }

    dirent *item = nullptr;
    while ((item = readdir(dir.get()))) {
        if (item->d_type == DT_DIR && strcmp(item->d_name, ".") != 0 && strcmp(item->d_name, "..") != 0)
            LoadPluginsFromDir(mgr, dirpath + "/" + item->d_name);
        else if (item->d_type == DT_REG) {
            if (strcmp(item->d_name + strlen(item->d_name) - 3, ".so") != 0)
                continue;
            std::string file = dirpath + '/' + item->d_name;
            printf("Try load %s\n", file.data());
            cum::Plugin *plugin = nullptr;
            try {
                plugin = mgr.LoadFromFile(file);
            } catch (cum::Manager::LoadError) {
                // skip it
                continue;
            }
            printf("Loaded %s -- %s\n", plugin->GetIdentifier().data(), plugin->GetName().data());
        }
    }
}

namespace theme {
    Color btn_default = Color(80, 80, 80);
    Color btn_hover   = Color(130, 130, 130);
    Color btn_pressed = Color(150, 150, 150);
    Color btn_text    = Color(255, 255, 255);
    Color btn_border  = Color(150, 150, 150);
    Color btn_selected = Color(0, 150, 0);
    Color btn_selectedBorder = Color(0, 255, 0);
};

dr4::Font *font;

class Button {
    dr4::Rect2f rect;
    dr4::Rectangle *v_rect;
    dr4::Text *v_text;
    std::function<void(Button*)> handler;

    bool pressed = false, hovered = false, selected = false;

public:

    Button(dr4::Window *win, dr4::Rect2f rect, const char *text, std::function<void(Button*)> handler) 
        :rect(rect), v_rect(win->CreateRectangle()), v_text(win->CreateText()), handler(handler) {
        v_rect->SetPos(rect.pos);
        v_rect->SetSize(rect.size);
        v_text->SetFont(font);
        v_text->SetText(text);
        v_text->SetVAlign(dr4::Text::VAlign::MIDDLE);
        v_text->SetPos(rect.pos + rect.size/2 - Vec2f(v_text->GetBounds().x / 2, 0));
        v_text->SetColor(theme::btn_text);
        v_rect->SetBorderColor(theme::btn_border);
    }

    void Render(dr4::Texture &tex) const {
        v_rect->SetBorderColor(theme::btn_border);
        if (selected) {
            v_rect->SetFillColor(theme::btn_selected);
            v_rect->SetBorderColor(theme::btn_selectedBorder);
        } else if (pressed)
            v_rect->SetFillColor(theme::btn_pressed);
        else if (hovered)
            v_rect->SetFillColor(theme::btn_hover);
        else
            v_rect->SetFillColor(theme::btn_default);
        v_rect->DrawOn(tex);
        v_text->DrawOn(tex);
    }

    bool OnEvent(dr4::Event &evt) {
        if (evt.type == dr4::Event::Type::MOUSE_MOVE) {
            hovered = rect.Contains(evt.mouseMove.pos);
        } else if (evt.type == dr4::Event::Type::MOUSE_DOWN) {
            pressed = rect.Contains(evt.mouseButton.pos);
            if (pressed && handler) {
                handler(this);
                return true;
            }
        } else if (evt.type == dr4::Event::Type::MOUSE_UP) {
            pressed = false;
        }
        return false;
    }

    void SetSelected(bool sel) {
        selected = sel;
    }
};

class Canvas : public pp::Canvas {
    std::vector<std::unique_ptr<pp::Shape>> shapes;
    dr4::Window *window;
    pp::Shape *selected = nullptr;
    pp::Tool *tool = nullptr;

    bool RouteToTool(dr4::Event &evt) {
        assert(tool);

        switch(evt.type) {
            case dr4::Event::Type::MOUSE_DOWN:
                return tool->OnMouseDown(evt.mouseButton);
            case dr4::Event::Type::MOUSE_UP:
                return tool->OnMouseUp(evt.mouseButton);
            case dr4::Event::Type::MOUSE_MOVE:
                return tool->OnMouseMove(evt.mouseMove);
            default:
                return false;
        }
    }

    bool RouteToShape(pp::Shape *shape, dr4::Event &evt) { 
        assert(shape);
        switch(evt.type) {
            case dr4::Event::Type::MOUSE_DOWN:
                return shape->OnMouseDown(evt.mouseButton);
            case dr4::Event::Type::MOUSE_UP:
                return shape->OnMouseUp(evt.mouseButton);
            case dr4::Event::Type::MOUSE_MOVE:
                return shape->OnMouseMove(evt.mouseMove);
            default:
                return false;
        }
    }

public:

    Canvas(dr4::Window *window) :window(window) {}

    virtual pp::ControlsTheme GetControlsTheme() const {
        return pp::ControlsTheme {
            .shapeFillColor = Color(200, 200, 200),
            .shapeBorderColor = Color(255, 255, 255),
            .selectColor = Color(255, 0, 0),
            .textColor = Color(255, 255, 255),
            .baseFontSize = 15,
            .handleColor = Color(0, 200, 0),
            .handleHoverColor = Color(0, 230, 0),
            .handleActiveColor = Color(0, 255, 0),
        };
    }

    virtual void AddShape(pp::Shape *shape) {
        shapes.push_back(std::unique_ptr<pp::Shape>(shape));
    }

    virtual void DelShape(pp::Shape *shape) {
        if (shape == selected) {
            shape->OnDeselect();
            selected = nullptr;
        }

        for (size_t i = 0; i < shapes.size(); ++i) {
            if (shapes[i].get() == shape) {
                shapes.erase(shapes.begin() + i);
                return;
            }
        }
        assert(false);
    }

    virtual void SetSelectedShape(pp::Shape *shape) {
        if (selected == shape) return;
        if (selected) selected->OnDeselect();
        if (shape) shape->OnSelect();
        selected = shape;
    }

    virtual pp::Shape *GetSelectedShape() const {
        return selected;
    }

    virtual void ShapeChanged(pp::Shape *shape) {}

    virtual dr4::Window *GetWindow() {
        return window;
    }

    void SetTool(pp::Tool *newTool) {
        if (tool == newTool) return;
        if (tool) tool->OnEnd();
        if (newTool) newTool->OnStart();
        tool = newTool;
    }
    
    pp::Tool *GetTool() {
        return tool;
    }

    void Render(dr4::Texture &tex) const {
        for (const auto &i : shapes)
            i->DrawOn(tex);
    } 

    bool OnEvent(dr4::Event &evt) {
        if (tool)
            return RouteToTool(evt);
        if (selected) {
            if (RouteToShape(selected, evt))
                return true;
        }
        if (evt.type == dr4::Event::Type::MOUSE_DOWN) {
            selected = nullptr;
            for (auto &i : shapes) {
                if (i->OnMouseDown(evt.mouseButton)) {
                    SetSelectedShape(i.get());
                    return true;
                }
            }
        }
        return false;
    }

};

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        puts("Expected a plugin directory to be specified");
        return -1;
    }

    cum::Manager pluginMgr;
    LoadPluginsFromDir(pluginMgr, argv[1]);
    pluginMgr.TriggerAfterLoad();
    
    auto *backend = pluginMgr.GetAnyOfType<cum::DR4BackendPlugin>();
    if (!backend) {
        puts("Backend for DR4 missing");
        return -1;
    }

    printf("Using backend %s\n", backend->GetIdentifier().data());

    std::unique_ptr<dr4::Window> win(backend->CreateWindow());
    win->Open();
    win->SetTitle("Canvas demo");
    win->SetSize(Vec2f(1200, 800));

    std::unique_ptr<dr4::Font> lfont(win->CreateFont());
    lfont->LoadFromFile("font.ttf"); // TODO: find it
    font = lfont.get();

    std::unique_ptr<dr4::Texture> mainTex(win->CreateTexture());
    mainTex->SetSize(win->GetSize());

    Canvas cvs(win.get());
    std::vector<std::unique_ptr<pp::Tool>> tools;
    std::vector<std::unique_ptr<Button>> buttons;

    for (auto plugin : pluginMgr.GetAllOfType<cum::PPToolPlugin>()) {
        printf("Load tools from %s\n", plugin->GetIdentifier().data());
        auto thisTools = plugin->CreateTools(&cvs);
        tools.insert(tools.end(), std::make_move_iterator(thisTools.begin()), std::make_move_iterator(thisTools.end()));
    }

    Button *active = nullptr;

    size_t pos = 20;
    for (auto &toolu : tools) {
        pp::Tool *tool = toolu.get();
        buttons.push_back(std::make_unique<Button>(
                    win.get(), dr4::Rect2f(20, pos, 40, 40),
                    tool->Icon().data(), [tool, &cvs, &active](Button *self){
            if (active) active->SetSelected(false);
            self->SetSelected(true);
            active = self;
            cvs.SetTool(tool);
        }));
        pos += 60;
    }
    
    while (true) {
        while (auto evt = win->PollEvent()) {
            if (evt->type == dr4::Event::Type::QUIT) {
                win->Close();
                goto loop_end;
            }
            if (evt->type == dr4::Event::Type::KEY_DOWN) {
                if (evt->key.sym == dr4::KEYCODE_ESCAPE) {
                    if (cvs.GetTool() && cvs.GetTool()->IsCurrentlyDrawing())
                        cvs.GetTool()->OnBreak();
                    else {
                        cvs.SetTool(nullptr);
                        if (active)
                            active->SetSelected(false);
                        active = nullptr;
                        if (cvs.GetSelectedShape())
                            cvs.SetSelectedShape(nullptr);
                    }
                }
            }
            bool got = false;
            for (auto &i : buttons) {
                got = i->OnEvent(*evt);
                if (got) break;
            }
            if (!got)
                cvs.OnEvent(*evt);
        }
        mainTex->Clear(Color(20, 20, 20));

        cvs.Render(*mainTex);
        for (auto &i : buttons)
            i->Render(*mainTex);

        win->Draw(*mainTex);
        win->Display();
    }
loop_end:
    {}
}
