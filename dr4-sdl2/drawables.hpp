#pragma once
#include "dr4/math/color.hpp"
#include "dr4/math/rect.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "./window.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

using dr4::Vec2f;
using dr4::Color;

#define FIELD_BASE(name, t_storage, t_in, t_out, init)\
    t_storage name##_ = init;\
    inline t_out Get##name() const override { return name##_; }\
    inline void Set##name(t_in v) override { name##_ = v; }

#define FIELD(name, type, init)\
    FIELD_BASE(name, type, type, type, init)

#define V0 Vec2f(0, 0)
#define C0 Color(0, 0, 0, 0)
#define C_RED Color(255, 0, 0)

class Image: public dr4::Image {

    SDL_Texture *tex;
    mutable Uint8 *pixels = nullptr;
    mutable int pitch;
    int w, h;

    Window *window;

public:

    Image(Window *win);

    ~Image();

    void SetPixel(size_t x, size_t y, Color color) override;
    Color GetPixel(size_t x, size_t y) const override;

    void SetSize(Vec2f size) override;

    Vec2f GetSize() const override;
    float GetWidth() const override;
    float GetHeight() const override;

    FIELD(Pos, Vec2f, V0);
    
    void DrawOn(dr4::Texture& texture) const override;
};

class Texture : public dr4::Texture {

    Window *window;
    SDL_Texture *tex;
    int w, h;
    dr4::Rect2f rect;
    bool clip = false;

public:

    Texture(Window *win);
    ~Texture();

    FIELD(Zero, Vec2f, V0);
    FIELD(Pos, Vec2f, V0);

    void SetSize(Vec2f size) override;
    Vec2f GetSize() const override;
    float GetWidth() const override;
    float GetHeight() const override;

    void SetClipRect(dr4::Rect2f rect) override;
    void RemoveClipRect() override;
    dr4::Rect2f GetClipRect() const override;
    void UseClip();

    void Clear(Color color) override;
    void DrawOn(dr4::Texture& texture) const override;

    inline SDL_Texture *GetTex() const { return tex; }
    inline Window *GetWindow() const { return window; }
    
    dr4::Image* GetImage() const override { return nullptr; }

};

class Font : public dr4::Font {

    TTF_Font *font = nullptr;

public:

    virtual ~Font();

    void LoadFromFile(const std::string &path) override;
    void LoadFromBuffer(const void *buffer, size_t size) override;
    float GetAscent(float fontSize) const override;
    float GetDescent(float fontSize) const override;
    float GetWidth(const char *text) const;

    inline TTF_Font *GetSDLFont() const { return font; }
};

class Line : public dr4::Line {

    FIELD(Start, Vec2f, V0);
    FIELD(End, Vec2f, Vec2f(100, 100));
    FIELD(Color, Color, C_RED);
    FIELD(Thickness, float, 0);
    FIELD(Pos, Vec2f, V0);
    
    void DrawOn(dr4::Texture& texture) const override;
};

class Circle : public dr4::Circle {

    FIELD(FillColor, Color, C0);
    FIELD(BorderColor, Color, C0);
    FIELD(BorderThickness, float, 0);
    FIELD(Center, Vec2f, V0)
    FIELD(Radius, Vec2f, Vec2f(50, 50));

    Vec2f GetPos() const override { return Center_ - Radius_; }
    void SetPos(Vec2f pos) override { Center_ = pos + Radius_; }

    void DrawOn(dr4::Texture& texture) const override;
};

class Rectangle : public dr4::Rectangle {

    FIELD(Size, Vec2f, Vec2f(100, 100));
    FIELD(FillColor, Color, C0);
    FIELD(BorderThickness, float, 0);
    FIELD(BorderColor, Color, C0);
    FIELD(Pos, Vec2f, V0);

    void DrawOn(dr4::Texture& texture) const override;
};

class Text : public dr4::Text {

    mutable SDL_Texture *tex;
    mutable int tex_w, tex_h;
    mutable bool changed = false;
    std::string text;
    Color color;
    Window *win;


    const std::string &GetText() const override { return text; }
    void SetText(const std::string &t) override { text = t; changed = true; }

    Color GetColor() const override { return color; }
    void SetColor(Color c) override { color = c; changed = true; }

    void Refresh() const;

    FIELD(FontSize, float, 0);
    FIELD(VAlign, VAlign, VAlign::BASELINE);
    FIELD(Font, const dr4::Font*, nullptr);
    FIELD(Pos, Vec2f, V0);

    Vec2f GetBounds() const override;
    void DrawOn(dr4::Texture& texture) const override;
    
public:
    Text(Window *win) :win(win) {}
};
