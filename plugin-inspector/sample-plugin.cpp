#include "cum/plugin.hpp"

class SamplePlugin : public cum::Plugin {

    virtual std::string_view GetIdentifier() const {
        return "isd.sample-plugin";
    }
    virtual std::string_view GetName() const {
        return "Sample plugin";
    }
    virtual std::string_view GetDescription() const {
        return "A sample plugin which does nothing";
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
};

extern "C" cum::Plugin *CreatePlugin() {
    return new SamplePlugin();
}
