#pragma once

#include "hui/event.hpp"
#include "hui/ui.hpp"
#include "sgui/box/box.hpp"
#include "sgui/box/scrollbox.hpp"
#include "sgui/box/window.hpp"
#include "sgui/input/textedit.hpp"
#include <functional>
#include <string_view>
#include <filesystem>

namespace ln {

class FilePickerEntry : public sgui::Button {
    void Redraw() const override;
    std::filesystem::path path;
    std::string icon;
public:
    FilePickerEntry(hui::UI *ui, std::string_view icon, std::string_view name, std::filesystem::path path)
        :Button(ui, name), path(path), icon(icon) {}

    const std::filesystem::path Path() const { return path; }
};

class FilePicker : public sgui::Window {
public:
    enum class Mode { OPEN, SAVE };
    using Callback = std::function<void(std::optional<std::string_view> name)>;

private:

    Callback cb;
    Mode mode;
    std::filesystem::path currentDir;
    sgui::Box *w_baseBox = nullptr;
    sgui::Scrollbox *w_scroller = nullptr;
    sgui::Box *w_fileBox = nullptr;
    sgui::Button *w_ok = nullptr, *w_cancel = nullptr;
    sgui::TextEdit *w_name = nullptr;
    std::filesystem::path newPath;
    FilePickerEntry *w_selected = nullptr;
    bool hasSelected = false;
    std::filesystem::path selected;
    std::vector<FilePickerEntry*> w_entries;

    hui::EventResult OnIdle(hui::IdleEvent &evt) override;
    void Redraw() const override;
    void OnSizeChanged() override;
    void SetDir(std::filesystem::path path);

public:

    FilePicker(hui::UI *ui, Mode mode, Callback cb = nullptr);
    void SetCallback(Callback cb) { this->cb = cb; }

};

};
