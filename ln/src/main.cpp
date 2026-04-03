#include "cum/ifc/dr4.hpp"
#include "cum/manager.hpp"
#include "dr4/event.hpp"
#include "dr4/keycodes.hpp"
#include "dr4/window.hpp"
#include "hui/event.hpp"
#include "lenses/file_picker.h"
#include "lenses/screenshot.h"
#include "lenses/ui/scene_tree.h"
#include "lenses/ui/toolbox.h"
#include "resfile.hpp"
#include "rtr/obj/mesh.h"
#include "rtr/obj/obj.h"
#include "rtr/renderer.h"
#include "rtr/obj/gnd.h"
#include "rtr/obj/sun.h"
#include "rtr/obj/sphere.h"
#include "rtr/sgui-view.h"
#include "sgui/box/box.hpp"
#include "sgui/box/scrollbox.hpp"
#include "sgui/box/window.hpp"
#include "sgui/input/button.hpp"
#include "sgui/menu.hpp"
#include "sgui/root.hpp"
#include "sgui/theme.hpp"
#include "sgui/ui.hpp"
#include "sgui/widget.hpp"
#include <sys/stat.h>

#include <dirent.h>
#include <iostream>
#include <memory>
#include <cstring>
#include <cassert>

using dr4::Vec2f;
using dr4::Color;

void LoadPluginFromFile(cum::Manager &mgr, const std::string &file) {
    printf("Try load %s\n", file.data());
    cum::Plugin *plugin = nullptr;
    try {
        plugin = mgr.LoadFromFile(file);
    } catch (cum::Manager::LoadError &e) {
        printf("-> Failed: %s\n", e.what());
        return;
    }
    printf("-> Loaded %s -- %s\n", plugin->GetIdentifier().data(), plugin->GetName().data());
}

void LoadPluginsFromDir(cum::Manager &mgr, std::string dirpath) {
    struct stat st;
    stat(dirpath.data(), &st);
    if (S_ISREG(st.st_mode)) {
        LoadPluginFromFile(mgr, dirpath);
        return;
    }

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
            LoadPluginFromFile(mgr, dirpath + '/' + item->d_name);
        }
    }
}

static void SetupScene(rtr::Scene &scene) {
   
    scene.renderingOptions() = rtr::RenderingOptions {
        .maxRayBounces = 8,
        .ambientColor = rgbf(1.0f),
        .ambientIntensity = 0.2f,
        .skyColor = rgbf(0.3, 0.4, 0.6),
        .shadows = rtr::RenderingOptions::NO_SHADOWS,
        .exitDistance = 0.01
    };

    auto gnd = scene.create<rtr::GroundPlane>(-1.0f);
    gnd->setName("ground");

    auto sun = scene.create<rtr::Sun>(vec3f(-1, 1, -1));
    sun->setMaterial(rtr::Material {
        .baseColor = rgbf(1.0f),
        .emission = 0.5f
    });
    sun->setName("sun");

    auto refractive = scene.create<rtr::Sphere>(vec3f(-1.5, -1.5, 0), 1);
    refractive->setMaterial(rtr::Material {
        .baseColor = rgbf(1.0f),
        .refraction = 0.9f,
        .refractiveIndex = 1.52f, // window glass
        .lightScalePerM = rgbf(0.7f, 0.7f, 0.7f)
    });
    refractive->setName("refractive");

    auto red = scene.create<rtr::Sphere>(vec3f(0, 0, 0), 0.5f);
    red->setMaterial(rtr::Material {
        .baseColor = rgbf(1, 0, 0)
    });
    red->setName("red");

    auto reflective = scene.create<rtr::Sphere>(vec3f(1.5, -1.5, 0), 1);
    reflective->setMaterial(rtr::Material {
        .baseColor = rgbf(1),
        .reflection = 0.9f
    });
    reflective->setName("reflective");

    auto secondReflective = scene.create<rtr::Sphere>(vec3f(1.5, 1.5, 0), 1);
    secondReflective->setMaterial(rtr::Material {
        .baseColor = rgbf(0),
        .reflection = 0.7f,
    });
    secondReflective->setName("reflective2");

    auto coloredRefractive = scene.create<rtr::Sphere>(vec3f(-1.5, 1.5, 0), 1);
    coloredRefractive->setMaterial(rtr::Material {
        .baseColor = rgbf(0.5, 0.5, 1),
        .refraction = 0.9f,
        .refractiveIndex = 1.77f, // saphire
        .lightScalePerM = rgbf(0.0f, 0.5f, 1.0f)
    });
    coloredRefractive->setName("coloredRefractive");

    auto mesh = scene.create<rtr::Mesh>();
    mesh->loadFromOBJFile("../obj/cube.obj");
    //mesh->loadFromOBJFile("../obj/cat/12221_Cat_v1_l3.obj");
    mesh->transform.offset = vec3f(5, 0, 0);
    mesh->setName("mesh");
    mesh->setMaterial(rtr::Material {
        .baseColor = rgbf(1, 1, 1),
        .emission = 0.1f
    });

}

sgui::Window *createListWindow(sgui::UI *ui, rtr::Scene *scene) {
    auto *win = new sgui::Window(ui, "Objects");
    auto *scroller = new sgui::Scrollbox(ui);
    win->SetChild(scroller);
    auto *tree = new ln::SceneTree(ui, scene);
    tree->SetSize(300 - sgui::theme::scrollbar::width, tree->GetSize().y);
    scroller->SetChild(tree);
    win->SetSize(300, 400);
    return win;
}

sgui::Window *createToolbox(sgui::UI *ui, rtr::Scene *scene) {
    auto *win = new sgui::Window(ui, "Toolbox");
    auto *scroller = new sgui::Scrollbox(ui);
    win->SetChild(scroller);
    auto *tree = new ln::Toolbox(ui, scene);
    tree->SetSize(400 - sgui::theme::scrollbar::width, tree->GetSize().y);
    scroller->SetChild(tree);
    win->SetSize(400, 500);
    return win;
}

#define ROTATION_SPEED 0.1
#define ZOOM_FACTOR 0.7
#define BTN_COLS 3

sgui::Window *createControls(sgui::UI *ui, rtr::Scene *scene, sgui::RTRView *view, sgui::TopMenu *menu) {
    struct btndef { const char *icon; std::function<void()> act; };

    struct itmdef { int idx; const char *name; };

    btndef buttons[] = {
        //--------------- row 1
        { "", [view](){
            view->SetDist(view->Dist() * ZOOM_FACTOR);
            view->RequestRerender();
        }},
        { "", [view](){
            view->SetElevation(view->Elevation() + ROTATION_SPEED);
            view->RequestRerender();
        }},
        { "", [view](){
            view->SetDist(view->Dist() / ZOOM_FACTOR);
            view->RequestRerender();
        }},
        //--------------- row 2
        { "", [view](){
            view->SetAzimuth(view->Azimuth() - ROTATION_SPEED);
            view->RequestRerender();
        }},
        { "", [view](){
            view->SetPlaneDist(2);
            view->SetTarget(vec3f(0, 0, 0));
            view->SetDist(100);
            view->SetElevation(M_PI_2 / 3);
            view->SetAzimuth(5 * M_PI_2 / 2);
            view->RequestRerender();
        }},
        { "", [view](){
            view->SetAzimuth(view->Azimuth() + ROTATION_SPEED);
            view->RequestRerender();
        }},
        //--------------- row 3
        { "", [view](){
            view->SetPlaneDist(view->PlaneDist() * 0.9);
            view->RequestRerender();
        }},
        { "", [view](){
            view->SetElevation(view->Elevation() - ROTATION_SPEED);
            view->RequestRerender();
        }},
        { "", [view](){
            view->SetPlaneDist(view->PlaneDist() / 0.9);
            view->RequestRerender();
        }},
        //--------------- row 4
        { "", [scene](){
            scene->create<rtr::Sphere>();
        }},
        { "", [scene](){
            std::vector<rtr::Object*> selected;
            for (auto i : scene->objects())
                if (i->isSelected())
                    selected.push_back(i);
            for (auto i : selected)
                scene->destroy(i);
        }},
    };

#define RC(r, c) (r * 3 + c)
    
    itmdef items[] = {
        { RC(1, 1), "Home" },
        { RC(0, 0), "Zoom in" },
        { RC(0, 2), "Zoom out" },
        { RC(0, 1), "Up" },
        { RC(1, 0), "Left" },
        { RC(1, 2), "Right" },
        { RC(2, 1), "Down" },
        { RC(2, 0), "Increase FOV" },
        { RC(2, 2), "Decrease FOV" },
        { RC(3, 0), "Add sphere" },
        { RC(3, 1), "Delete selected" },
    };

#undef RC

    auto ctls = menu->AddSubmenu("Controls");
    for (size_t i = 0; i < sizeof(items) / sizeof(items[0]); ++i) {
        auto &item = items[i];
        auto &btn = buttons[item.idx];
        ctls->AddItem(std::string(btn.icon) + "   " + item.name, btn.act);
    }

    
    auto *win = new sgui::Window(ui, "Controls");
    auto *box = new sgui::Box(ui);
    win->SetChild(box);

    const float BTN_SIZE = 30;
    float xpos = 10, ypos = 10, h = 10;
    for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {

        if (i % 3 == 0)
            h += BTN_SIZE + 10;

        auto btn = new sgui::Button(ui, buttons[i].icon);
        btn->SetSize(BTN_SIZE, BTN_SIZE);
        btn->SetPos(xpos, ypos);
        btn->SetHandler(buttons[i].act);
        box->AddChild(btn);
        
        if (i % 3 == 2) {
            xpos = 10;
            ypos += BTN_SIZE + 10;
        } else {
            xpos += BTN_SIZE + 10;
        }
    }

    win->SetSize(40 + BTN_SIZE * 3, h);
    return win;
}

void setupMenu(sgui::Box *desktop, sgui::TopMenu *menu, cum::Manager *mgr, rtr::Scene *scene, sgui::RTRView *view) {
    auto file = menu->AddSubmenu("File");

    file->AddItem("Save", [scene, desktop](){
        ln::FilePicker *picker = new ln::FilePicker(
                desktop->GetUI(),
                ln::FilePicker::Mode::SAVE
        );
        picker->SetCallback([scene, desktop, picker](auto path){
            if (path) {
                rsf::ResFile file;
                for (auto i : scene->objects())
                    file.Add(i->id(), i);
                file.WriteFile(path.value());
            }
            desktop->DelChild(picker);
        });
        desktop->AddChild(picker);
        desktop->BringToFront(picker);
        picker->SetPos(300, 300);
    });

    file->AddItem("Load", [scene, view, desktop](){
        ln::FilePicker *picker = new ln::FilePicker(
                desktop->GetUI(),
                ln::FilePicker::Mode::OPEN
        );
        picker->SetCallback([scene, desktop, picker, view](auto path){
            if (path) {
                int cnt = view->ThreadCount();
                view->SetThreadCount(0);
                rsf::ResFile file;
                file.LoadFile(path.value());
                scene->clear();
                for (auto i : file.GetKeys()) {
                    auto obj = file.Get<rtr::Object>(i);
                    if (!obj) continue;
                    file.Unmanage(obj);
                    scene->add(obj);
                }
                view->SetThreadCount(cnt);
            }
            desktop->DelChild(picker);
        });
        desktop->AddChild(picker);
        desktop->BringToFront(picker);
        picker->SetPos(300, 300);
    });

    
    auto plugins = menu->AddSubmenu("Plugins");
    for (auto &plugin : mgr->GetAll()) {
        auto sub = plugins->AddSubmenu(plugin->GetName());
        std::string idstr = "id: ";
        idstr += plugin->GetIdentifier();
        sub->AddItem(idstr, nullptr);
    }
}

void addCreationMenu(rtr::Scene *scene, sgui::TopMenu *menu) {
    auto submenu = menu->AddSubmenu("Create");

    struct Option { std::string_view name; std::function<void()> fn; };
    
#define SIMPLE_CREATEFUNC(name, type) { name, [scene](){  scene->deselectAll(); auto obj = scene->create<type>(); obj->setSelected(true); }}

    Option options[] = {
        SIMPLE_CREATEFUNC("Sphere", rtr::Sphere),
        SIMPLE_CREATEFUNC("Mesh", rtr::Mesh),
        SIMPLE_CREATEFUNC("Ground plane", rtr::GroundPlane),
        SIMPLE_CREATEFUNC("Sun", rtr::Sun),
    };
#undef SIMPLE_CREATEFUNC

    for (size_t i = 0; i < sizeof(options) / sizeof(options[0]); ++i)
        submenu->AddItem(options[i].name, options[i].fn);    
}

int main(int argc, const char *argv[]) {

    // Load plugins

    cum::Manager pluginMgr;
    for (int i = 1; i < argc; ++i)
        LoadPluginsFromDir(pluginMgr, argv[i]);
    pluginMgr.TriggerAfterLoad();
    
    auto *backend = pluginMgr.GetAnyOfType<cum::DR4BackendPlugin>();
    if (!backend) {
        puts("Backend for DR4 missing");
        return -1;
    }

    // Create scene

    rtr::Scene scene;
    SetupScene(scene);

    // Open window

    std::unique_ptr<dr4::Window> win(backend->CreateWindow());
    win->Open();
    win->SetTitle("Lenses");
    win->SetSize(Vec2f(1200, 800));

    sgui::UI ui(win.get());
    win->SetDefaultFont(ui.Font());
    
    auto *root = new sgui::Root(&ui);
    ui.SetRoot(root);
    root->SetRect(dr4::Rect2f(Vec2f(0, 0), win->GetSize()));

    auto desktop = new sgui::Box(&ui);
    desktop->SetRect(dr4::Rect2f(Vec2f(0, 0), win->GetSize()));
    root->AddChild(desktop);

    auto menu = new sgui::TopMenu(&ui);
    menu->SetSize(win->GetSize().x, 0);
    menu->SetPos(0, 0);
    root->AddChild(menu);
    root->BringToFront(menu);

    // Setup ui

    auto *view = new sgui::RTRView(&ui, &scene);
    view->SetRect(sgui::Rect2f(Vec2f(0, 0), win->GetSize()));

    view->SetDist(20);
    view->SetElevation(M_PI_2 / 3);
    view->RequestRerender();
    view->SetThreadCount(7);
    desktop->AddChild(view);
   
    setupMenu(desktop, menu, &pluginMgr, &scene, view);
    addCreationMenu(&scene, menu);

    auto *listWin = createListWindow(&ui, &scene);
    listWin->SetPos(30, 60);
    desktop->AddChild(listWin);
    desktop->BringToFront(listWin);

    auto *toolboxWin = createToolbox(&ui, &scene);
    toolboxWin->SetPos(win->GetSize().x - 430, 60);
    desktop->AddChild(toolboxWin);
    desktop->BringToFront(toolboxWin);
    
    auto *controlsWin = createControls(&ui, &scene, view, menu);
    controlsWin->SetPos(30, 550);
    desktop->AddChild(controlsWin);
    desktop->BringToFront(controlsWin);

    bool screenshotIsOpen = false;
    ln::Screenshotter *scr = new ln::Screenshotter(&ui, &pluginMgr, [desktop, &scr, &screenshotIsOpen](){
        desktop->ExtractChild(scr);
        screenshotIsOpen = false;
    }, menu);
    scr->SetSize(win->GetSize());
    scr->SetPos(0, 0);

    double prevTime = win->GetTime();

    while (true) {
        while (auto evt = win->PollEvent()) {
            if (evt->type == dr4::Event::Type::QUIT) {
                win->Close();
                goto loop_end;
            }
            if (evt->type == dr4::Event::Type::KEY_DOWN && evt->key.sym == dr4::KEYCODE_TILDE) {
                desktop->AddChild(scr);
                desktop->BringToFront(scr);
                screenshotIsOpen = true;
            }
            ui.ProcessEvent(*evt);
        }

        double curTime = win->GetTime();
        hui::IdleEvent idle;
        idle.deltaTime = curTime - prevTime;
        idle.absTime = curTime;
        prevTime = curTime;
        if (!screenshotIsOpen)
            ui.OnIdle(idle);
        else
            idle.Apply(*scr);
        win->Clear(dr4::Color(10, 10, 10));
        win->Draw(*ui.GetTexture());
        win->Display();
    }

loop_end:
    {}
}
