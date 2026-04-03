#include "cum/ifc/dr4.hpp"
#include "cum/manager.hpp"
#include "dr4/math/rect.hpp"
#include "dr4/math/vec2.hpp"
#include "sgui/box/box.hpp"
#include "sgui/box/scrollbox.hpp"
#include "sgui/box/window.hpp"
#include "sgui/input/button.hpp"
#include "sgui/input/textedit.hpp"
#include "sgui/menu.hpp"
#include "sgui/ui.hpp"
#include "sgui/root.hpp"
#include "sgui/view/text.hpp"

using dr4::Vec2f;

const std::string_view lorem 
    = "Lorem ipsum dolor sit amet consectetur\n"
      "adipiscing elit. Quisque faucibus ex sapien\n"
      "vitae pellentesque sem placerat. In id cursus\n"
      "mi pretium tellus duis convallis. Tempus leo eu\n"
      "aenean sed diam urna tempor. Pulvinar vivamus\n"
      "fringilla lacus nec metus bibendum egestas. Iaculis\n"
      "massa nisl malesuada lacinia integer nunc posuere.\n"
      "Ut hendrerit semper vel class aptent taciti"
      "sociosqu.\n"
      "Ad litora torquent per conubia nostra inceptos\n"
      "himenaeos.\n";


int main () {
    cum::Manager mgr;
    mgr.LoadFromFile("dr4-sdl2/isd.dr4-sdl2-backend.so");
    mgr.TriggerAfterLoad();
 
    auto *backend = mgr.GetAnyOfType<cum::DR4BackendPlugin>();
    if (!backend) {
        puts("Backend for DR4 missing");
        return -1;
    }

    std::unique_ptr<dr4::Window> win(backend->CreateWindow());
    win->Open();
    win->SetTitle("Canvas demo");
    win->SetSize(Vec2f(1200, 800));

    sgui::UI ui(win.get());
    auto *root = new sgui::Root(&ui);
    ui.SetRoot(root);
    root->SetRect(dr4::Rect2f(Vec2f(0, 0), win->GetSize()));

    sgui::Window *uiwin = new sgui::Window(&ui, "Demo window");
    uiwin->SetPos(Vec2f(100, 100));
    uiwin->SetSize(Vec2f(300, 400));
    root->AddChild(uiwin);

    sgui::Window *uiwin2 = new sgui::Window(&ui, "Other window");
    uiwin2->SetPos(Vec2f(600, 100));
    uiwin2->SetSize(Vec2f(200, 200));
    root->AddChild(uiwin2);

    sgui::Scrollbox *box = new sgui::Scrollbox(&ui);
    uiwin->SetChild(box);

    sgui::Box *chbox = new sgui::Box(&ui);
    chbox->SetSize(300, 500);
    box->SetChild(chbox);

    sgui::Text *text = new sgui::Text(&ui, lorem);
    text->SetPos(10, 10);
    text->SetSize(300, 200);
    chbox->AddChild(text);

    sgui::Button *bbtn = new sgui::Button(&ui, "Other button");
    bbtn->SetSize(150, 30);
    bbtn->SetPos(10, 250);
    chbox->AddChild(bbtn);

    sgui::TextEdit *edit = new sgui::TextEdit(&ui);
    edit->SetValue("Hello world!");
    edit->SetSize(150, 30);
    edit->SetPos(10, 290);
    chbox->AddChild(edit);

    sgui::Button *btn = new sgui::Button(&ui, "Demo button");
    uiwin2->SetChild(btn);
    btn->SetHandler([btn](){ puts("click"); btn->SetSelected(!btn->Selected()); });

    auto top = new sgui::TopMenu(&ui);
    top->SetPos(0, 0);
    top->SetSize(win->GetSize().x, 0);
    auto menu = top->AddSubmenu("Menu");
    menu->AddItem("One", [](){ puts("menu 1"); });
    menu->AddItem("Two", [](){ puts("menu 2"); });
    auto *sub = menu->AddSubmenu("Submenu");
    sub->AddItem("Three", [](){ puts("menu 3"); });
    auto *sub2 = sub->AddSubmenu("Submenu");
    sub2->AddItem("Four", [](){ puts("menu 4"); });

    root->AddChild(top);
    root->BringToFront(top);

    while (true) {
        while (auto evt = win->PollEvent()) {
            if (evt->type == dr4::Event::Type::QUIT) {
                win->Close();
                return 0;
            }
            ui.ProcessEvent(*evt);
        }
        win->Clear(dr4::Color(10, 10, 10));
        win->Draw(*ui.GetTexture());
        win->Display();
    }
}
