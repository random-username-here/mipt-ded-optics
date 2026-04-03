#ifndef I_LENSES_UI_TOOL
#define I_LENSES_UI_TOOL

#include "hui/ui.hpp"
#include "hui/widget.hpp"
#include "rtr/obj/obj.h"
#include "sgui/box/box.hpp"
#include "sgui/input/textedit.hpp"
#include "sgui/theme.hpp"
#include "sgui/view/text.hpp"
#include <cassert>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace ln {

class ToolRegistrationBase {
public:
    virtual const std::type_info &type() const = 0;
    virtual hui::Widget *createTool(hui::UI *app, rtr::Object *object) const = 0;
};

class ToolRegistry {

    std::unordered_map<std::type_index, const ToolRegistrationBase*> m_registered;
public:

    void registerTool(const ToolRegistrationBase *registration);
    const ToolRegistrationBase* getToolFor(const std::type_info &type) const;
};

ToolRegistry& toolRegistry();

template<typename For, typename Tool>
class ToolRegistration : public ToolRegistrationBase {
public:
    ToolRegistration() {
        toolRegistry().registerTool(this);
    }

    const std::type_info &type() const override {
        return typeid(For);
    }

    hui::Widget *createTool(hui::UI *app, rtr::Object *obj) const override {
        For* cast = dynamic_cast<For*>(obj);
        assert(cast != nullptr);
        return new Tool(app, cast);
    }
};

#define REGISTER_TOOL(type, tool) static ln::ToolRegistration<type, tool> _tool;

class Tool : public sgui::Box {

    void OnSizeChanged() override;

    int ecount = 0;

public:

    struct Entry {
        sgui::Text *text;
        sgui::TextEdit *edit;
    };


    using Box::Box;

    Entry addField(
            std::string_view name,
            std::string_view initial = "",
            sgui::TextEdit::ValidatorType validator = sgui::TextEdit::NoValidator
    );

    sgui::Text *addText(std::string_view str, dr4::Color color = sgui::theme::theme::titleColor2);



};


};

#endif
