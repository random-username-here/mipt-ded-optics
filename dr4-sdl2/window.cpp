#include "./window.hpp"
#include "./drawables.hpp"
#include "dr4/mouse_buttons.hpp"
#include <SDL3/SDL_timer.h>
#include <cassert>
#include <cmath>

Window::Window() {
    blendPremult = SDL_ComposeCustomBlendMode(
            SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD,
            SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD
    );
    blendNonPremult = SDL_ComposeCustomBlendMode(
            SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD,
            SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD
    );
}

Window::~Window() {
    if (open)
        Close();
}

void Window::SetTitle(const std::string &title) {
    assert(open);

    this->title = title;
    SDL_SetWindowTitle(window, title.c_str());
}

const std::string &Window::GetTitle() const {
    return title;
}

dr4::Vec2f Window::GetSize() const {
    assert(open);
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return Vec2f(w, h);
}

void Window::SetSize(dr4::Vec2f size) {
    assert(open);
    SDL_SetWindowSize(window, size.x, size.y);
}

void Window::Open() {
    assert(!open);
    SDL_CreateWindowAndRenderer(1000, 800, 0, &window, &renderer);
    SDL_SetRenderDrawBlendMode(renderer, blendNonPremult);
    SDL_StartTextInput();
    open = true;
}

bool Window::IsOpen() const {
    return open;
}

void Window::Close() {
    assert(open);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    renderer = nullptr;
    window = nullptr;
}

void Window::Clear(dr4::Color color) {
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

void Window::Draw(const dr4::Texture &texture) {
    SDL_SetRenderTarget(renderer, nullptr);

    const Texture &texImpl = dynamic_cast<const Texture&>(texture);

    SDL_Texture *target = texImpl.GetTex();
    if (!target) return;

    SDL_Rect rect = {
        (int) texImpl.Pos_.x, (int) texImpl.Pos_.y,
        (int) texImpl.GetWidth(), (int) texImpl.GetHeight()
    };
    SDL_RenderCopy(renderer, texImpl.GetTex(), NULL, &rect);
}

void Window::Display() {
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderPresent(renderer);
}

double Window::GetTime() {
    return SDL_GetTicks64() / 1000.0;
}
void Window::Sleep(double t) {
    SDL_Delay(t / 1000.0);
}

dr4::Texture   *Window::CreateTexture()   { return new Texture(this); }
dr4::Image     *Window::CreateImage()     { return new Image(this); }
dr4::Font      *Window::CreateFont()      { return new Font(); }
dr4::Line      *Window::CreateLine()      { return new Line(); }
dr4::Circle    *Window::CreateCircle()    { return new Circle(); }
dr4::Rectangle *Window::CreateRectangle() { return new Rectangle(); }
dr4::Text      *Window::CreateText()      { return new Text(this); }

void Window::StartTextInput() {
    SDL_StartTextInput();
}

void Window::StopTextInput() {
    SDL_StopTextInput();
}

static dr4::KeyMode remapMods(int sdlMods) {
    int mods = 0;
    if (sdlMods & KMOD_CTRL)
        mods |= dr4::KeyMode::KEYMOD_CTRL;
    if (sdlMods & KMOD_ALT)
        mods |= dr4::KeyMode::KEYMOD_ALT;
    if (sdlMods & KMOD_SHIFT)
        mods |= dr4::KeyMode::KEYMOD_SHIFT;
    return (dr4::KeyMode) mods;
}

static dr4::KeyCode remapKeys(int sdlKey) {
    switch(sdlKey) {

        case SDLK_a: return dr4::KeyCode::KEYCODE_A;
        case SDLK_b: return dr4::KeyCode::KEYCODE_B;
        case SDLK_c: return dr4::KeyCode::KEYCODE_C;
        case SDLK_d: return dr4::KeyCode::KEYCODE_D;
        case SDLK_e: return dr4::KeyCode::KEYCODE_E;
        case SDLK_f: return dr4::KeyCode::KEYCODE_F;
        case SDLK_g: return dr4::KeyCode::KEYCODE_G;
        case SDLK_h: return dr4::KeyCode::KEYCODE_H;
        case SDLK_i: return dr4::KeyCode::KEYCODE_I;
        case SDLK_j: return dr4::KeyCode::KEYCODE_J;
        case SDLK_k: return dr4::KeyCode::KEYCODE_K;
        case SDLK_l: return dr4::KeyCode::KEYCODE_L;
        case SDLK_m: return dr4::KeyCode::KEYCODE_M;
        case SDLK_n: return dr4::KeyCode::KEYCODE_N;
        case SDLK_o: return dr4::KeyCode::KEYCODE_O;
        case SDLK_p: return dr4::KeyCode::KEYCODE_P;
        case SDLK_q: return dr4::KeyCode::KEYCODE_Q;
        case SDLK_r: return dr4::KeyCode::KEYCODE_R;
        case SDLK_s: return dr4::KeyCode::KEYCODE_S;
        case SDLK_t: return dr4::KeyCode::KEYCODE_T;
        case SDLK_u: return dr4::KeyCode::KEYCODE_U;
        case SDLK_v: return dr4::KeyCode::KEYCODE_V;
        case SDLK_w: return dr4::KeyCode::KEYCODE_W;
        case SDLK_x: return dr4::KeyCode::KEYCODE_X;
        case SDLK_y: return dr4::KeyCode::KEYCODE_Y;
        case SDLK_z: return dr4::KeyCode::KEYCODE_Z;
        case SDLK_0: return dr4::KeyCode::KEYCODE_NUM0;
        case SDLK_1: return dr4::KeyCode::KEYCODE_NUM1;
        case SDLK_2: return dr4::KeyCode::KEYCODE_NUM2;
        case SDLK_3: return dr4::KeyCode::KEYCODE_NUM3;
        case SDLK_4: return dr4::KeyCode::KEYCODE_NUM4;
        case SDLK_5: return dr4::KeyCode::KEYCODE_NUM5;
        case SDLK_6: return dr4::KeyCode::KEYCODE_NUM6;
        case SDLK_7: return dr4::KeyCode::KEYCODE_NUM7;
        case SDLK_8: return dr4::KeyCode::KEYCODE_NUM8;
        case SDLK_9: return dr4::KeyCode::KEYCODE_NUM9;
        case SDLK_ESCAPE: return dr4::KeyCode::KEYCODE_ESCAPE;
        case SDLK_LCTRL: return dr4::KeyCode::KEYCODE_LCONTROL;
        case SDLK_LSHIFT: return dr4::KeyCode::KEYCODE_LSHIFT;
        case SDLK_LALT: return dr4::KeyCode::KEYCODE_LALT;
        //case SDLK_LMETA: return dr4::KeyCode::KEYCODE_LSYSTEM;
        case SDLK_RCTRL: return dr4::KeyCode::KEYCODE_RCONTROL;
        case SDLK_RSHIFT: return dr4::KeyCode::KEYCODE_RSHIFT;
        case SDLK_RALT: return dr4::KeyCode::KEYCODE_RALT;
        //case SDLK_RMETA: return dr4::KeyCode::KEYCODE_RSYSTEM;
        case SDLK_MENU: return dr4::KeyCode::KEYCODE_MENU;
        case SDLK_LEFTBRACKET: return dr4::KeyCode::KEYCODE_LBRACKET;
        case SDLK_RIGHTBRACKET: return dr4::KeyCode::KEYCODE_RBRACKET;
        case SDLK_SEMICOLON: return dr4::KeyCode::KEYCODE_SEMICOLON;
        case SDLK_COMMA: return dr4::KeyCode::KEYCODE_COMMA;
        case SDLK_PERIOD: return dr4::KeyCode::KEYCODE_PERIOD;
        //case SDLK_QUOTE: return dr4::KeyCode::KEYCODE_QUOTE;
        case SDLK_SLASH: return dr4::KeyCode::KEYCODE_SLASH;
        case SDLK_BACKSLASH: return dr4::KeyCode::KEYCODE_BACKSLASH;
        case SDLK_BACKQUOTE: return dr4::KeyCode::KEYCODE_TILDE;
        //case SDLK_EQUAL: return dr4::KeyCode::KEYCODE_EQUAL;
        //case SDLK_HYPHEN: return dr4::KeyCode::KEYCODE_HYPHEN;
        case SDLK_SPACE: return dr4::KeyCode::KEYCODE_SPACE;
        //case SDLK_ENTER: return dr4::KeyCode::KEYCODE_ENTER;
        case SDLK_BACKSPACE: return dr4::KeyCode::KEYCODE_BACKSPACE;
        case SDLK_TAB: return dr4::KeyCode::KEYCODE_TAB;
        case SDLK_PAGEUP: return dr4::KeyCode::KEYCODE_PAGEUP;
        case SDLK_PAGEDOWN: return dr4::KeyCode::KEYCODE_PAGEDOWN;
        case SDLK_END: return dr4::KeyCode::KEYCODE_END;
        case SDLK_HOME: return dr4::KeyCode::KEYCODE_HOME;
        case SDLK_INSERT: return dr4::KeyCode::KEYCODE_INSERT;
        case SDLK_DELETE: return dr4::KeyCode::KEYCODE_DELETE;
        //case SDLK_ADD: return dr4::KeyCode::KEYCODE_ADD;
        //case SDLK_SUBTRACT: return dr4::KeyCode::KEYCODE_SUBTRACT;
        //case SDLK_MULTIPLY: return dr4::KeyCode::KEYCODE_MULTIPLY;
        //case SDLK_DIVIDE: return dr4::KeyCode::KEYCODE_DIVIDE;
        case SDLK_LEFT: return dr4::KeyCode::KEYCODE_LEFT;
        case SDLK_RIGHT: return dr4::KeyCode::KEYCODE_RIGHT;
        case SDLK_UP: return dr4::KeyCode::KEYCODE_UP;
        case SDLK_DOWN: return dr4::KeyCode::KEYCODE_DOWN;
        /*case SDLK_NUMPAD0: return dr4::KeyCode::KEYCODE_NUMPAD0;
        case SDLK_NUMPAD1: return dr4::KeyCode::KEYCODE_NUMPAD1;
        case SDLK_NUMPAD2: return dr4::KeyCode::KEYCODE_NUMPAD2;
        case SDLK_NUMPAD3: return dr4::KeyCode::KEYCODE_NUMPAD3;
        case SDLK_NUMPAD4: return dr4::KeyCode::KEYCODE_NUMPAD4;
        case SDLK_NUMPAD5: return dr4::KeyCode::KEYCODE_NUMPAD5;
        case SDLK_NUMPAD6: return dr4::KeyCode::KEYCODE_NUMPAD6;
        case SDLK_NUMPAD7: return dr4::KeyCode::KEYCODE_NUMPAD7;
        case SDLK_NUMPAD8: return dr4::KeyCode::KEYCODE_NUMPAD8;
        case SDLK_NUMPAD9: return dr4::KeyCode::KEYCODE_NUMPAD9;*/
        case SDLK_F1: return dr4::KeyCode::KEYCODE_F1;
        case SDLK_F2: return dr4::KeyCode::KEYCODE_F2;
        case SDLK_F3: return dr4::KeyCode::KEYCODE_F3;
        case SDLK_F4: return dr4::KeyCode::KEYCODE_F4;
        case SDLK_F5: return dr4::KeyCode::KEYCODE_F5;
        case SDLK_F6: return dr4::KeyCode::KEYCODE_F6;
        case SDLK_F7: return dr4::KeyCode::KEYCODE_F7;
        case SDLK_F8: return dr4::KeyCode::KEYCODE_F8;
        case SDLK_F9: return dr4::KeyCode::KEYCODE_F9;
        case SDLK_F10: return dr4::KeyCode::KEYCODE_F10;
        case SDLK_F11: return dr4::KeyCode::KEYCODE_F11;
        case SDLK_F12: return dr4::KeyCode::KEYCODE_F12;
        case SDLK_F13: return dr4::KeyCode::KEYCODE_F13;
        case SDLK_F14: return dr4::KeyCode::KEYCODE_F14;
        case SDLK_F15: return dr4::KeyCode::KEYCODE_F15;
        case SDLK_PAUSE: return dr4::KeyCode::KEYCODE_PAUSE;
        default: return dr4::KEYCODE_UNKNOWN;
    }
}

std::optional<dr4::Event> Window::PollEvent() {

    SDL_Event evt;

    while (1) {
        int res = SDL_PollEvent(&evt);
        if (!res) return std::nullopt;

        dr4::Event ret;
        switch(evt.type) {
            case SDL_QUIT:
                ret.type = dr4::Event::Type::QUIT;
                return ret;

            case SDL_MOUSEMOTION: {
                Vec2f pos = Vec2f(evt.motion.x, evt.motion.y);
                if (std::isnan(prevPos.x))
                    prevPos = pos;
                ret.type = dr4::Event::Type::MOUSE_MOVE;
                ret.mouseMove = {
                    .pos = pos,
                    .rel = pos - prevPos
                };
                prevPos = pos;
                return ret;
            }

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                dr4::MouseButtonType btn;
                switch (evt.button.button) {
                    case SDL_BUTTON_LEFT: btn = dr4::MouseButtonType::LEFT; break;
                    case SDL_BUTTON_RIGHT: btn = dr4::MouseButtonType::RIGHT; break;
                    case SDL_BUTTON_MIDDLE: btn = dr4::MouseButtonType::MIDDLE; break;
                    default: btn = dr4::MouseButtonType::UNKNOWN; break;
                }
                ret.type = evt.type == SDL_MOUSEBUTTONDOWN 
                        ? dr4::Event::Type::MOUSE_DOWN : dr4::Event::Type::MOUSE_UP;
                ret.mouseButton = {
                    .button = btn,
                    .pos = dr4::Vec2f(evt.button.x, evt.button.y),
                };
                return ret;
            }

            case SDL_MOUSEWHEEL:
                ret.type = dr4::Event::Type::MOUSE_WHEEL,
                ret.mouseWheel = {
                    .delta = Vec2f((float) evt.wheel.x, (float) evt.wheel.y),
                    .pos = dr4::Vec2f(evt.wheel.mouseX, evt.wheel.mouseY),
                };
                return ret;

            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                ret.type = evt.type == SDL_KEYDOWN ? dr4::Event::Type::KEY_DOWN : dr4::Event::Type::KEY_UP;
                ret.key = {
                    .sym = remapKeys(evt.key.keysym.sym),
                    .mods = remapMods(evt.key.keysym.mod)
                };
                return ret;
            }

            case SDL_TEXTINPUT: {
                ret.type = dr4::Event::Type::TEXT_EVENT;
                text = evt.text.text;
                ret.text = { .unicode = text.data() };
                return ret;
            }

            default:
                // process another one
                break;
        }
    }
}


