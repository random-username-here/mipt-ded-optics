#include "sgui/box/box.hpp"
#include "dr4/math/color.hpp"
#include "hui/event.hpp"
#include "hui/ui.hpp"
#include "hui/widget.hpp"
#include "sgui/widget.hpp"

namespace sgui {

Box::Box(hui::UI *ui) :Container(ui) {}

void Box::Redraw() const {
    GetTexture().Clear(dr4::Color(0, 0, 0, 0));
    for (size_t i = children.size() - 1; i != (size_t) -1; --i) {
        if (children[i])
            GetTexture().Draw(children[i]->GetFreshTexture());
    }
}
    
hui::EventResult Box::PropagateToChildren(hui::Event &event) {
    for (auto &i : children)
        if (i && event.Apply(*i.get()) == hui::EventResult::HANDLED)
            return hui::EventResult::HANDLED;
    return hui::EventResult::UNHANDLED;
}

void Box::AddChild(hui::Widget *win) {
    children.push_back(std::unique_ptr<hui::Widget>(win));
    BecomeParentOf(win);
    ForceRedraw();
}

hui::Widget *Box::ExtractChild(hui::Widget *win) {
    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i].get() == win) {
            needsClear = true;
            hui::Widget *ret = children[i].release();
            UnbecomeParentOf(win);
            ForceRedraw();
            return ret;
        }
    }
    return nullptr;
}

void Box::DelChild(hui::Widget *wgt) {
    delete ExtractChild(wgt);
}


void Box::BringToFront(hui::Widget *win) {
    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i].get() == win) {
            auto el = children[i].release();
            children.erase(children.begin() + i);
            children.insert(children.begin(), std::unique_ptr<hui::Widget>(el));
            return;
        }
    }
}

hui::EventResult Box::OnIdle(hui::IdleEvent &event) {
    Container::OnIdle(event);
    if (needsClear) {
        for (int i = children.size()-1; i >= 0; --i) {
            if (!children[i])
                children.erase(children.begin() + i);
        }
        needsClear = false;
    }
    return hui::EventResult::UNHANDLED;
}

};
