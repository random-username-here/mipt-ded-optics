#include "rtr/obj/sphere.h"
#include "lenses/ui/tool.h"
#include "rtr/renderer.h"
#include <string>

namespace ln {

class SphereTool : public Tool {

public:
    SphereTool(hui::UI *app, rtr::Sphere *sphere) :Tool(app) {
        auto x = addField("X", std::to_string(sphere->getCenter().x()), sgui::TextEdit::NumberValidator);
        x.edit->SetHandler([sphere](const std::string& val) {
                vec3f c = sphere->getCenter();
                sphere->setCenter(vec3f(atof(val.c_str()), c.y(), c.z()));
                sphere->scene()->onObjectFieldsChange.trigger(sphere);
        });
        auto y = addField("Y", std::to_string(sphere->getCenter().y()), sgui::TextEdit::NumberValidator);
        y.edit->SetHandler([sphere](const std::string& val) {
                vec3f c = sphere->getCenter();
                sphere->setCenter(vec3f(c.x(), atof(val.c_str()), c.z()));
                sphere->scene()->onObjectFieldsChange.trigger(sphere);
        });
        auto z = addField("Z", std::to_string(sphere->getCenter().z()), sgui::TextEdit::NumberValidator);
        z.edit->SetHandler([sphere](const std::string& val) {
                vec3f c = sphere->getCenter();
                sphere->setCenter(vec3f(c.x(), c.y(), atof(val.c_str())));
                sphere->scene()->onObjectFieldsChange.trigger(sphere);
        });
        auto r = addField("R", std::to_string(sphere->getRadius()), sgui::TextEdit::NumberValidator);
        r.edit->SetHandler([sphere](const std::string& val) {
                sphere->setRadius(atof(val.c_str()));
                sphere->scene()->onObjectFieldsChange.trigger(sphere);
        });
    }
};

REGISTER_TOOL(rtr::Sphere, SphereTool);

};
