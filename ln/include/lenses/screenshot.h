#pragma once
#include "cum/manager.hpp"
#include "dr4/event.hpp"
#include "dr4/window.hpp"
#include "hui/event.hpp"
#include "hui/ui.hpp"
#include "pp/canvas.hpp"
#include "pp/tool.hpp"
#include "sgui/box/window.hpp"
#include "sgui/input/button.hpp"
#include "sgui/menu.hpp"
#include "sgui/widget.hpp"
#include <functional>
#include <vector>
#include <memory>

namespace ln {

class Screenshotter : public pp::Canvas, public sgui::Container {

    std::vector<std::unique_ptr<pp::Shape>> shapes;
    std::vector<std::unique_ptr<pp::Tool>> tools;

    pp::Shape *selected = nullptr;

    pp::Tool *tool = nullptr;
    sgui::Button *toolButton = nullptr;
    std::unique_ptr<sgui::Window> toolWin;
    bool isSettingSelection = false;

    std::function<void()> onEnd;

    bool RouteToTool(dr4::Event &evt);
    bool RouteToShape(pp::Shape *shape, dr4::Event &evt); 
    dr4::Event ToDR4Event(const hui::Event *evt);
    hui::EventResult PropagateToChildren(hui::Event &evt) override;
    void Redraw() const override;

public:

    Screenshotter(hui::UI *ui, cum::Manager *mgr, std::function<void()> endCb, sgui::TopMenu *menu);

    virtual dr4::Window *GetWindow() override { return GetUI()->GetWindow(); }  

    virtual pp::ControlsTheme GetControlsTheme() const override;
    virtual void AddShape(pp::Shape *shape) override;
    virtual void DelShape(pp::Shape *shape) override;
    virtual void SetSelectedShape(pp::Shape *shape) override;

    virtual pp::Shape *GetSelectedShape() const override {
        return selected;
    }

    virtual void ShapeChanged(pp::Shape *shape) override { ForceRedraw(); }
};

};
