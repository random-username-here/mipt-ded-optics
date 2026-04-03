#pragma once

#include "hui/event.hpp"
#include "sgui/widget.hpp"
#include <memory>
#include <vector>

namespace sgui {

class Box : public Container {

protected:
    std::vector<std::unique_ptr<hui::Widget>> children;
    bool needsClear = false;
    
    void Redraw() const override;
    hui::EventResult PropagateToChildren(hui::Event &event) override;
    hui::EventResult OnIdle(hui::IdleEvent &event) override;

public:
    Box(hui::UI *ui);

    void DelChild(hui::Widget *wgt);
    void AddChild(hui::Widget *wgt);
    hui::Widget *ExtractChild(hui::Widget *wgt);
    void Clear() {
        for (auto &i : children)
            UnbecomeParentOf(i.get());
        children.clear();
    }

    void BringToFront(hui::Widget *wgt);

};

};
