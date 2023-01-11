#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <stdint.h>

struct SDL_Window;
struct SDL_Renderer;

class Renderer
{
public:
    Renderer(bool debug_mode);
    ~Renderer();

    void clear();

    void present();

    void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t intensity);

private:
    bool create_window();
    void destroy_window();

    bool _debug_mode;

    SDL_Window *_window{nullptr};
    SDL_Renderer *_renderer{nullptr};
};

#endif RENDERER_HPP
