#ifndef I_SGUI_EVENT
#define I_SGUI_EVENT

#include <functional>
#include <vector>
namespace sgui {

template<typename ...Args>
class Event {

public:

    using HandlerType = std::function<void(Args...)>;

private:

    std::vector<HandlerType> m_handlers;

public:

    void addListener(HandlerType handler) {
        m_handlers.push_back(handler);
    }

    void trigger(Args... args) {
        for (auto &i : m_handlers)
            i(args...);
    }
};


};

#endif
