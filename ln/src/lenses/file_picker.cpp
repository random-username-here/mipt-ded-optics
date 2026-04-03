#include "lenses/file_picker.h"
#include "dr4/math/vec2.hpp"
#include "hui/event.hpp"
#include "imm_dr4.hpp"
#include "sgui/input/textedit.hpp"
#include "sgui/theme.hpp"
#include <filesystem>
#include <optional>

#define GAP 20
#define BAR_HEIGHT 30
#define BTN_WIDTH 100
#define ENTRY_SIZE 25
#define ICON_W 30

using dr4::Vec2f;

namespace ln {

void FilePickerEntry::Redraw() const {
    
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

    Imm().Draw(dr4_imm::Text {
        .pos = Vec2f(GAP, GetSize().y/2),
        .text = icon,
        .color = dr4::Color(128, 128, 128),
        .size = sgui::theme::fontSize,
        .align = dr4_imm::VAlign::MIDDLE,
        .font = GetUI()->Font(),
    });


    Imm().Draw(dr4_imm::Text {
        .pos = Vec2f(
            GAP + sgui::theme::fontSize + ICON_W,
            GetSize().y/2
        ),
        .text = label,
        .color = fg,
        .size = sgui::theme::fontSize,
        .align = dr4_imm::VAlign::MIDDLE,
        .font = GetUI()->Font(),
    });
}

FilePicker::FilePicker(hui::UI *ui, Mode mode, Callback cb)
    :Window(ui), mode(mode), cb(cb) {
    
    w_baseBox = new sgui::Box(ui);
    SetChild(w_baseBox);

    w_scroller = new sgui::Scrollbox(ui);
    w_baseBox->AddChild(w_scroller);

    w_fileBox = new sgui::Box(ui);
    w_scroller->SetChild(w_fileBox);

    w_name = new sgui::TextEdit(ui);
    if (mode == Mode::OPEN)
        w_name->SetDisabled(true);
    w_baseBox->AddChild(w_name);
    w_name->SetHandler([this](const std::string &name){
        std::filesystem::path path = currentDir / name;
        for (auto &i : w_entries) {
            if (i->Path() == path) {
                if (w_selected)
                    w_selected->SetSelected(false);
                i->SetSelected(true);
                w_selected = i;
            }
        }
        hasSelected = true;
        selected = path;
    });
    w_name->SetValidator([this](const std::string &name){
        if (name.empty()) return false;
        std::filesystem::path path = currentDir / name;
        if (std::filesystem::exists(path) && !std::filesystem::is_regular_file(path))
            return false;
        return true;
    });

    w_ok = new sgui::Button(ui, mode == Mode::SAVE ? "Save" : "Open");
    w_cancel = new sgui::Button(ui, "Cancel");
    w_baseBox->AddChild(w_ok);
    w_baseBox->AddChild(w_cancel);
    w_ok->SetHandler([this](){
        if (!hasSelected)
            this->cb(std::nullopt);
        else
            this->cb(selected.c_str());
    });
    w_cancel->SetHandler([this](){
        this->cb(std::nullopt);
    });

    SetTitle(mode == Mode::SAVE ? "Save as" : "Open file");
    SetPos(0, 0);
    SetSize(500, 400);
    SetDir(std::filesystem::current_path());
}

void FilePicker::Redraw() const {
    sgui::Window::Redraw();
    Imm().Draw(dr4_imm::Rectangle {
        .pos = Vec2f(0, BAR_HEIGHT-2),
        .size = Vec2f(GetSize().x, 3),
        .fill = sgui::theme::menu::borderColor
    });
    Imm().Draw(dr4_imm::Rectangle {
        .pos = Vec2f(0, 0),
        .size = Vec2f(GetSize().x, BAR_HEIGHT),
        .fill = sgui::theme::menu::bgColor
    });
    Imm().Draw(dr4_imm::Text {
        .pos = Vec2f(GAP, BAR_HEIGHT/2.0),
        .text = currentDir.c_str(),
        .color = sgui::theme::textColor,
        .size = sgui::theme::fontSize,
        .align = dr4_imm::VAlign::MIDDLE,
        .font = GetUI()->Font(),
    });
}

void FilePicker::OnSizeChanged() {
    Window::OnSizeChanged();
    w_scroller->SetPos(0, BAR_HEIGHT);
    w_scroller->SetSize(GetSize().x, std::max<float>(0, GetSize().y - BAR_HEIGHT * 2));
    float bottom = GetSize().y - BAR_HEIGHT;
    float xpos = GetSize().x - BTN_WIDTH * 2 + 1;
    w_name->SetPos(0, bottom);
    w_name->SetSize(xpos+1, BAR_HEIGHT);
    w_ok->SetPos(xpos, bottom);
    w_ok->SetSize(BTN_WIDTH, BAR_HEIGHT);
    xpos += BTN_WIDTH-1;
    w_cancel->SetPos(xpos, bottom);
    w_cancel->SetSize(BTN_WIDTH, BAR_HEIGHT);
    for (auto i : w_entries)
        i->SetSize(w_fileBox->GetSize().x, ENTRY_SIZE);
    ForceRedraw();
}

void FilePicker::SetDir(std::filesystem::path path) {
    printf("setting dir to %s\n", path.c_str());
    newPath = currentDir = path;
    if (w_selected) {
        w_name->SetValue("");
        hasSelected = false;
        w_selected = nullptr;
    }
    w_fileBox->Clear();
    float pos = 0;

    if (currentDir.has_parent_path()) {
        auto w_up = new FilePickerEntry(GetUI(), "", "..", currentDir);
        w_up->SetPos(0, pos);
        w_up->SetSize(w_fileBox->GetSize().x, ENTRY_SIZE);
        w_up->SetHandler([this](){
            newPath = currentDir.parent_path();
        });
        w_fileBox->AddChild(w_up);
        pos += ENTRY_SIZE;
        w_entries.push_back(w_up);
    }

    for (auto const& file : std::filesystem::directory_iterator(path)) {
        std::string icon = "";
        if (file.is_directory()) icon = " ";
        else if (file.is_regular_file()) {
            auto ext = file.path().extension();
            if (ext == ".scn") icon = " ";
            else if (ext == ".obj") icon = "󰆧 ";
            else icon = " ";
        } else {
            icon = " ";
        }
        auto btn = new FilePickerEntry(GetUI(), icon, file.path().filename().c_str(), file.path());
        btn->SetPos(0, pos);
        btn->SetSize(w_fileBox->GetSize().x, ENTRY_SIZE);
        w_fileBox->AddChild(btn);
        pos += ENTRY_SIZE;

        if (file.is_directory()) {
            btn->SetHandler([this, file](){
                // we cannot change directory here,
                // because we will delete this button
                newPath = file.path();
            });
        } else if (file.is_regular_file()) {
            btn->SetHandler([this, btn, file](){
                if (w_selected)
                    w_selected->SetSelected(false);
                btn->SetSelected(true);
                w_selected = btn;
                selected = file.path();
                w_name->SetValue(file.path().filename().c_str());
                hasSelected = true;
            });
        }
        w_entries.push_back(btn);
    }


    w_fileBox->SetSize(w_fileBox->GetSize().x, pos);
    ForceRedraw();
}

hui::EventResult FilePicker::OnIdle(hui::IdleEvent &evt) {
    Window::OnIdle(evt);
    if (newPath != currentDir) {
        SetDir(newPath);
    }
    return hui::EventResult::UNHANDLED;
}

};
