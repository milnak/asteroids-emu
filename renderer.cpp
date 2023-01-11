#include "renderer.hpp"

#include <iostream>

#include <SDL.h>
#include <windows.h> // AdjustWindowsRect

Renderer::Renderer(bool debug_mode) : _debug_mode(debug_mode)
{
    // TODO: Throw on fail
    create_window();
}

Renderer::~Renderer()
{
    destroy_window();
}

bool Renderer::create_window()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        return false;
    }

    // DVG renders 0..1023
    // Want a 1024x1024 client area.
    RECT rc{0, 0, 1024, 1024};

    // TODO: Review Window Styles here.
    AdjustWindowRect(&rc,
                     WS_MINIMIZEBOX | WS_SYSMENU | WS_DLGFRAME | WS_BORDER,
                     FALSE);

    // SDL_WINDOW_MAXIMIZED
    _window = SDL_CreateWindow("AsteroidsEmu by JeffMill",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               rc.right, rc.bottom,
                               0);
    if (_window == nullptr)
    {
        return false;
    }

    // SDL_GLContext Context{SDL_GL_CreateContext(window)};

    _renderer = SDL_CreateRenderer(_window, -1, 0);
    if (_renderer == nullptr)
    {
        return false;
    }

    return true;
}

void Renderer::destroy_window()
{
    SDL_DestroyRenderer(_renderer);
    _renderer = nullptr;

    SDL_DestroyWindow(_window);
    _window = nullptr;

    SDL_Quit();
}

void Renderer::clear()
{
    if (_debug_mode)
    {
        std::cout << "[Renderer] clear" << std::endl;
    }
    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(_renderer);
}

void Renderer::present()
{
    if (_debug_mode)
    {
        std::cout << "[Renderer] present" << std::endl;
    }

    SDL_RenderPresent(_renderer);
}

void Renderer::draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t intensity)
{
    if (_debug_mode)
    {
        std::cout
            << "[Renderer] draw_line"
            << std::dec
            << " (" << x1 << ", " << y1 << ")-"
            << "(" << x2 << ", " << y2 << ")"
            << " int: " << static_cast<int>(intensity)
            << std::endl;
    }

    SDL_SetRenderDrawColor(_renderer, 255, 255, 255, intensity * 17 /* Convert to 0..255 */);

    SDL_RenderDrawLine(_renderer, x1, y1, x2, y2);
}
