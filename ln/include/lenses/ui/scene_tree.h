#ifndef I_LENSES_UI_SCENETREE
#define I_LENSES_UI_SCENETREE

#include "hui/event.hpp"
#include "rtr/obj/obj.h"
#include "rtr/renderer.h"
#include "sgui/box/box.hpp"
#include "sgui/box/scrollbox.hpp"
#include "sgui/input/button.hpp"

namespace ln {

class SceneTreeItem : public sgui::Button {

    rtr::Object *m_obj;
    bool m_selected = false; // if we have selected state texture drawn

public:

    inline SceneTreeItem(hui::UI *app, rtr::Object *obj)
        :Button(app, ""), m_obj(obj) {}

    void Redraw() const override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &evt) override;
    void updateSelection();

};
    
class SceneTree : public sgui::Box {
    
    rtr::Scene *m_scene;

    void refresh();
    void refreshSelected();
    void refreshNames();
    void OnSizeChanged() override;

public:

    SceneTree(hui::UI *app, rtr::Scene *scene);

};

};

#endif
