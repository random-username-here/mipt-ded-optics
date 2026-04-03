#include "rtr/obj/obj.h"
#include "lenses/ui/tool.h"
#include "rtr/renderer.h"
#include "sgui/theme.hpp"
#include <string>

namespace ln {

class ObjectTool : public Tool {

public:
    ObjectTool(hui::UI *app, rtr::Object *obj) :Tool(app) {

        auto name = addField("Name", obj->name());
        name.edit->SetHandler([obj](const std::string &name){
            obj->setName(name);
            obj->scene()->onObjectFieldsChange.trigger(obj);
        });
        addText("Resource name: " + obj->id(), sgui::theme::textColor);

        addText("Phong");
        auto &mat = obj->getCommonMaterial();

        #define GEN_FLOAT_FIELD_V(name, field, var)\
            auto field = addField(name, std::to_string(mat.field), sgui::TextEdit::NumberValidator);\
            field.edit->SetHandler([&mat, obj](const std::string &val) {\
                mat.field = std::stof(val);\
                obj->scene()->onObjectFieldsChange.trigger(obj);\
            });

        #define GEN_FLOAT_FIELD(name, field)\
            GEN_FLOAT_FIELD_V(name, field, field)

        GEN_FLOAT_FIELD("Ambient", ambient);
        GEN_FLOAT_FIELD("Diffuse", diffuse);
        GEN_FLOAT_FIELD("Specular", specular);
        GEN_FLOAT_FIELD("Shinyness", shinyness);
        GEN_FLOAT_FIELD("Emission", emission);

        addText("Reflection");
        GEN_FLOAT_FIELD("Reflectivity", reflection);

        addText("Refraction");
        GEN_FLOAT_FIELD("Refraction", refraction);
        GEN_FLOAT_FIELD("Refractive index", refractiveIndex);
    }
};
REGISTER_TOOL(rtr::Object, ObjectTool);

};
