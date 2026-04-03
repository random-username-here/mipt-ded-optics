#include "dr4/math/color.hpp"
#include "dr4/math/rect.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/texture.hpp"
#include "./drawables.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <assert.h>
#include <climits>

using dr4::Vec2f;
using dr4::Color;

inline void SDL_SetRenderDrawColor(SDL_Renderer *renderer, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);    
}

// image

Image::Image(Window *win) :window(win), tex(nullptr), w(0), h(0) {}
Image::~Image() { SDL_DestroyTexture(tex); }

void Image::SetPixel(size_t x, size_t y, Color color) {
    if (!tex || x >= w || y >= h) return;
    Uint8 *place = pixels + x * 4 + y * pitch;
    place[3] = color.r;
    place[2] = color.g;
    place[1] = color.b;
    place[0] = color.a;
}

Color Image::GetPixel(size_t x, size_t y) const {
    if (!pixels || x >= w || y >= h) // locking will just give garbage
        return Color(0, 0, 0, 0); 
    Uint8 *place = pixels + x * 4 + y * pitch;
    return Color(place[3], place[2], place[1], place[0]);
}

void Image::SetSize(Vec2f size) {
    SDL_DestroyTexture(tex);
    pixels = nullptr;

    tex = SDL_CreateTexture(window->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, (int) size.x, (int) size.y);
    SDL_SetTextureBlendMode(tex, window->blendNonPremult);
    w = (int) size.x; h = (int) size.y;
    SDL_LockTexture(tex, NULL, (void**) &pixels, &pitch);
    assert(pixels);
}

float Image::GetWidth() const { return w; }
float Image::GetHeight() const { return h; }
Vec2f Image::GetSize() const { return { GetWidth(), GetHeight() }; }

void Image::DrawOn(dr4::Texture &dst) const {
    if (!tex) return;

    Texture &dstImpl = dynamic_cast<Texture&>(dst);
    SDL_UnlockTexture(tex);
    pixels = nullptr;

    SDL_Texture *target = dstImpl.GetTex();
    if (target) {
        SDL_SetRenderTarget(window->renderer, target);
        dstImpl.UseClip();
        Vec2f off = dstImpl.GetZero();

        SDL_Rect rect = { (int) (Pos_.x + off.x), (int) (Pos_.y + off.y), w, h };
        SDL_RenderCopy(window->renderer, tex, NULL, &rect);
    }

    SDL_LockTexture(tex, NULL, (void**) &pixels, &pitch);
    assert(pixels);

}

// texture

Texture::Texture(Window *win) :window(win), tex(nullptr), w(0), h(0) {}
Texture::~Texture() { SDL_DestroyTexture(tex); }

void Texture::SetSize(Vec2f size) {
    SDL_DestroyTexture(tex);

    tex = SDL_CreateTexture(window->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, (int) size.x, (int) size.y);
    SDL_SetTextureBlendMode(tex, window->blendPremult);
    w = (int) size.x; h = (int) size.y;
}

float Texture::GetWidth() const { return w; }
float Texture::GetHeight() const { return h; }
Vec2f Texture::GetSize() const { return { GetWidth(), GetHeight() }; }

void Texture::SetClipRect(dr4::Rect2f rect) {
    this->rect = rect;
    clip = true;
}

void Texture::RemoveClipRect() {
    clip = false;
}

dr4::Rect2f Texture::GetClipRect() const {
    if (clip) return rect;
    else return dr4::Rect2f(
        -Zero_, Vec2f(w, h)
    );
}

void Texture::UseClip() {
    if (clip) {
        SDL_Rect cr = { 
            .x = (int) (rect.pos.x + Zero_.x), .y = (int) (rect.pos.y + Zero_.y),
            .w = (int) rect.size.x, .h = (int) rect.size.y
        };
        SDL_RenderSetClipRect(window->renderer, &cr);
    } else {
        SDL_RenderSetClipRect(window->renderer, NULL);
    }
}

void Texture::Clear(Color color) {
    if (!tex) return;
    SDL_SetRenderTarget(window->renderer, tex);
    UseClip();
    SDL_SetRenderDrawColor(window->renderer, color);
    SDL_RenderClear(window->renderer);
}

void Texture::DrawOn(dr4::Texture &dst) const {
    if (!tex) return;

    Texture &dstImpl = dynamic_cast<Texture&>(dst);

    SDL_Texture *target = dstImpl.GetTex();
    if (!target) return;
    SDL_SetRenderTarget(window->renderer, target);
    dstImpl.UseClip();
    Vec2f off = dstImpl.GetZero();

    SDL_Rect rect = { (int) (Pos_.x + off.x), (int) (Pos_.y + off.y), w, h };
    SDL_RenderCopy(window->renderer, tex, NULL, &rect);
}

// font

#define FONTSIZE 15

void Font::LoadFromFile(const std::string &path) {
    printf("loading font `%s`\n", path.c_str());
    font = TTF_OpenFont(path.c_str(), FONTSIZE);
    assert(font);
}

void Font::LoadFromBuffer(const void *buf, size_t size) {
    SDL_RWops *data = SDL_RWFromConstMem(buf, size);
    font = TTF_OpenFontRW(data, true, FONTSIZE);
}

float Font::GetAscent(float size) const {
    assert(font);
    return TTF_FontAscent(font);
}

float Font::GetDescent(float size) const {
    assert(font);
    return TTF_FontDescent(font);
}

float Font::GetWidth(const char *text) const {
    assert(font);
    int width = 0;
    TTF_MeasureUTF8(font, text, INT_MAX, &width, nullptr);
    return width;
}

Font::~Font() {
    TTF_CloseFont(font);
}

void Line::DrawOn(dr4::Texture& dst) const {
    Texture &dstImpl = dynamic_cast<Texture&>(dst);

    SDL_Texture *target = dstImpl.GetTex();
    if (!target) return;
    SDL_SetRenderTarget(dstImpl.GetWindow()->renderer, target);
    dstImpl.UseClip();
    Vec2f off = dstImpl.GetZero() + Pos_;

    thickLineRGBA(
            dstImpl.GetWindow()->renderer,
            Start_.x + off.x, Start_.y + off.y,
            End_.x + off.x, End_.y + off.y, 
            Thickness_,
            Color_.r, Color_.g, Color_.b, Color_.a
    );
}

void Circle::DrawOn(dr4::Texture& dst) const {
    Texture &dstImpl = dynamic_cast<Texture&>(dst);

    SDL_Texture *target = dstImpl.GetTex();
    if (!target) return;
    SDL_SetRenderTarget(dstImpl.GetWindow()->renderer, target);
    dstImpl.UseClip();
    Vec2f off = dstImpl.GetZero();

    filledEllipseRGBA(
            dstImpl.GetWindow()->renderer,
            Center_.x + off.x, Center_.y + off.y,
            Radius_.x, Radius_.y,
            FillColor_.r, FillColor_.g, FillColor_.b, FillColor_.a
    );
    ellipseRGBA(
            dstImpl.GetWindow()->renderer,
            Center_.x + off.x, Center_.y + off.y,
            Radius_.x, Radius_.y,
            BorderColor_.r, BorderColor_.g, BorderColor_.b, BorderColor_.a
    ); // TODO: border width
}

void Rectangle::DrawOn(dr4::Texture& dst) const {
    Texture &dstImpl = dynamic_cast<Texture&>(dst);

    SDL_Texture *target = dstImpl.GetTex();
    if (!target) return;
    SDL_SetRenderTarget(dstImpl.GetWindow()->renderer, target);
    dstImpl.UseClip();
    Vec2f base = dstImpl.GetZero() + Pos_;

    boxRGBA(
            dstImpl.GetWindow()->renderer,
            base.x, base.y, base.x + Size_.x, base.y + Size_.y,
            FillColor_.r, FillColor_.g, FillColor_.b, FillColor_.a
    );
    rectangleRGBA(
            dstImpl.GetWindow()->renderer,
            base.x, base.y, base.x + Size_.x, base.y + Size_.y,
            BorderColor_.r, BorderColor_.g, BorderColor_.b, BorderColor_.a
    ); // TODO: border width
}

void Text::Refresh() const {
    if (tex) SDL_DestroyTexture(tex);

    const Font *font = dynamic_cast<const Font*>(Font_);
    assert(font);
    assert(font->GetSDLFont());

    SDL_Surface *surf = TTF_RenderUTF8_Blended_Wrapped(
            font->GetSDLFont(), text.c_str(),
            SDL_Color { .r = color.r, .g = color.g, .b = color.b, .a = color.a },
            10000
    );
    if (surf) {
        assert(surf);
        tex = SDL_CreateTextureFromSurface(win->renderer, surf);
        tex_w = surf->w;
        tex_h = surf->h;
        SDL_FreeSurface(surf);
    } else {
        tex = nullptr;
    }
    changed = false;

}

Vec2f Text::GetBounds() const {
    assert(Font_);
    const Font *font = dynamic_cast<const Font*>(Font_);
    assert(font);

    return Vec2f(
        font->GetWidth(text.c_str()),
        font->GetAscent(0) // in case someone will use this
    );
}

void Text::DrawOn(dr4::Texture& dst) const {
    assert(Font_);
    Texture &dstImpl = dynamic_cast<Texture&>(dst);

    SDL_Texture *target = dstImpl.GetTex();
    if (!target) return;
    SDL_SetRenderTarget(dstImpl.GetWindow()->renderer, target);
    dstImpl.UseClip();
    Vec2f base = dstImpl.GetZero() + Pos_;
    const Font *font = dynamic_cast<const Font*>(Font_);
    assert(font);

    switch(VAlign_) {
        case VAlign::TOP:      base.y -= 0; break;
        case VAlign::MIDDLE:   base.y -= (font->GetAscent(0) - font->GetDescent(0)) / 2; break;
        case VAlign::BASELINE: base.y -= font->GetAscent(0); break;
        case VAlign::BOTTOM:   base.y -= (font->GetAscent(0) - font->GetDescent(0)); break;
        case VAlign::UNKNOWN: default: assert(false);
    }

    if (changed)
        Refresh();

    if (tex) {
        SDL_Rect rect = { (int) base.x, (int) base.y, tex_w, tex_h };
        SDL_RenderCopy(dstImpl.GetWindow()->renderer, tex, NULL, &rect);
    }

}


