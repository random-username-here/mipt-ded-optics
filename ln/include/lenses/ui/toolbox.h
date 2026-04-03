#ifndef I_LENSES_UI_TOOLBOX
#define I_LENSES_UI_TOOLBOX

#include "sgui/box/box.hpp"
#include "rtr/renderer.h"
#include <typeinfo>
#include <unordered_set>

namespace ln {

class Toolbox : public sgui::Box {
    rtr::Scene *m_scene;
    void refresh();

    void createToolsFor(rtr::Object* obj, float &ypos, const std::type_info *ti, std::unordered_set<void*> &used);
    void OnSizeChanged() override;

public:

    Toolbox(hui::UI *app, rtr::Scene *scene);
};

};

#endif
