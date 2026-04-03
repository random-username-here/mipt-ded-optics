#include "circle.hpp"
#include "line.hpp"
#include "cum/ifc/pp.hpp"
#include "rectlike.hpp"

class SamplePlugin : public cum::PPToolPlugin {

    virtual std::string_view GetIdentifier() const {
        return "isd.pp-tools";
    }
    virtual std::string_view GetName() const {
        return "PP tools by i-s-d";
    }
    virtual std::string_view GetDescription() const {
        return "Contains circles, lines, rectangles, text fields";
    }
    virtual std::vector<std::string_view> GetDependencies() const {
        return {};
    }
    virtual std::vector<std::string_view> GetConflicts() const {
        return {};
    }
    virtual void AfterLoad() {
        // do nothing
    }

    virtual std::vector<std::unique_ptr<pp::Tool>> CreateTools(pp::Canvas *cvs) {
        std::vector<std::unique_ptr<pp::Tool>> tools;
        tools.push_back(std::make_unique<CircleTool>(cvs));
        tools.push_back(std::make_unique<PolyTool>(cvs));
        tools.push_back(std::make_unique<RectTool>(cvs, "󰗆", "Rectangle", [](auto cvs) { return new Rect(cvs); }));
        return tools;
    }
};

extern "C" cum::Plugin *CreatePlugin() {
    return new SamplePlugin();
}
