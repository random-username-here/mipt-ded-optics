#include "rtr/obj/gnd.h"
#include "lenses/ui/tool.h"
#include "rtr/renderer.h"
#include <string>

namespace ln {

class GroundTool : public Tool {

public:
    GroundTool(hui::UI *app, rtr::GroundPlane *ground) :Tool(app) {
        auto z = addField("Z", std::to_string(ground->z()), sgui::TextEdit::NumberValidator);
        z.edit->SetHandler([ground](const std::string& val) {
                ground->setZ(std::stof(val));
                ground->scene()->onObjectFieldsChange.trigger(ground);
        });
    }
};

REGISTER_TOOL(rtr::GroundPlane, GroundTool);

};
