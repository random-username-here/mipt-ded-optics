#pragma once

#include "hui/event.hpp"
#include "sgui/widget.hpp"
#include <functional>
namespace sgui {

class TextEdit : public Widget {

public:

    using ValidatorType = std::function<bool(const std::string&)>;
    using HandlerType = std::function<void(const std::string&)>;

    static bool NoValidator(const std::string& value);
    static bool NumberValidator(const std::string& value);

private:
    
    std::string m_value = "";
    size_t m_pos = 0;
    ValidatorType m_validator = NoValidator;
    HandlerType m_handler;
    bool focused = false;
    bool disabled = false;

    void Redraw() const override;
    hui::EventResult OnKeyDown(hui::KeyEvent &) override;
    hui::EventResult OnText(hui::TextEvent &) override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &) override;
    void OnFocusGained() override;
    void OnFocusLost() override;

public:

    using Widget::Widget;

    void SetDisabled(bool dis) { disabled = dis; }
    inline void SetValidator(ValidatorType validator) { m_validator = validator; };
    inline void SetHandler(HandlerType handler) { m_handler = handler; };
    inline void SetValue(std::string_view value) { m_value = value; ForceRedraw(); }

};


};
