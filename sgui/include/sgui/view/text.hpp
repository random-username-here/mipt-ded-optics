#pragma once

#include "hui/ui.hpp"
#include "sgui/widget.hpp"
#include <string_view>
#include "sgui/theme.hpp"
namespace sgui {

class Text : public Widget {

    std::string text;
    Color color = theme::textColor;
    float size = theme::fontSize;

    void Redraw() const override;

public:

    Text(hui::UI *ui, std::string_view text = "Some text") 
        :Widget(ui), text(text) {}

    void SetColor(Color c) { color = c; ForceRedraw(); }
    void SetText(std::string_view s) { text = s; ForceRedraw(); }
    
};

};
