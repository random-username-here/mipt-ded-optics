#pragma once
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "dr4/window.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>

using dr4::Vec2f;
using dr4::Color;

class Window : public dr4::Window {
public:

    SDL_Window *window;
    SDL_Renderer *renderer;
    Uint64 prevTicksMs;
    bool open;
    std::string title;
    std::string text;
    const dr4::Font *defFont;
    Vec2f prevPos = Vec2f(NAN, NAN);

    SDL_BlendMode blendPremult, blendNonPremult;

    Window();
	~Window();

    void SetTitle(const std::string &title) override;
    const std::string &GetTitle() const override;

    Vec2f GetSize() const override;
    void SetSize(Vec2f size) override;

    void Open() override;
    bool IsOpen() const override;
    void Close() override;

    void Clear(Color color) override;
    void Draw(const dr4::Texture &texture) override;
    void Display() override;

    // Monotonic time in seconds
    double GetTime() override;
    void Sleep(double t) override;

    dr4::Texture   *CreateTexture()   override;
    dr4::Image     *CreateImage()     override;
    dr4::Font      *CreateFont()      override;
    dr4::Line      *CreateLine()      override;
    dr4::Circle    *CreateCircle()    override;
    dr4::Rectangle *CreateRectangle() override;
    dr4::Text      *CreateText()      override;

    void StartTextInput() override;
    void StopTextInput() override;

    std::optional<dr4::Event> PollEvent() override;

    virtual void SetDefaultFont( const dr4::Font* font) override { defFont = font; }
    virtual const dr4::Font* GetDefaultFont() override { return defFont; }

    virtual void SetClipboard( const std::string& string ) override {}
    virtual std::string GetClipboard() override { return ""; }


};
