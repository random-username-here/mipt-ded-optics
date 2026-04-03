#include "./window.hpp"
#include "cum/ifc/dr4.hpp"
#include <SDL2/SDL.h>
#include <SDL_ttf.h>

class Plugin : public cum::DR4BackendPlugin {

    virtual std::string_view GetIdentifier() const {
        return "isd.dr4-sdl2-backend";
    }
    virtual std::string_view GetName() const {
        return "DR4 SDL2 backend";
    }
    virtual std::string_view GetDescription() const {
        return "A backend for DR4 based on SDL2.\n"
               "Requires SDL2 and SDL2_gfx to be present.";
    }
    virtual std::vector<std::string_view> GetDependencies() const {
        return {};
    }
    virtual std::vector<std::string_view> GetConflicts() const {
        return {};
    }
    virtual void AfterLoad() {
        SDL_Init(SDL_INIT_VIDEO);
        TTF_Init();
    }

    virtual dr4::Window *CreateWindow() {
        return new Window();
    }
};

extern "C" cum::Plugin *CreatePlugin() {
    return new Plugin();
}
