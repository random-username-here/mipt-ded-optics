#include "rtr/obj/mesh.h"
#include "rtr/renderer.h"
#include "lenses/ui/tool.h"
#include "sgui/theme.hpp"
#include <stdexcept>
#include <string>

namespace ln {

class MeshTool : public Tool {

public:
    MeshTool(hui::UI *app, rtr::Mesh *mesh) :Tool(app) {
        
        auto filename = addField(
                "Filename",
                mesh->filename()
        );
        auto error = addText("No error", sgui::theme::theme::textColor);
        filename.edit->SetHandler([error, mesh](const std::string &val){
                error->SetText("Loading...");
                try {
                    mesh->loadFromOBJFile(val);
                    error->SetText("No error");
                } catch(std::runtime_error &err) {
                    error->SetText(err.what());
                }
                mesh->scene()->onObjectFieldsChange.trigger(mesh);
        });
    }
};

REGISTER_TOOL(rtr::Mesh, MeshTool);

};
