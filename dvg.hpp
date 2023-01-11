#ifndef DVG_HPP
#define DVG_HPP

void fail_fast(const char *msg);

#include <stdint.h>
#include <array>

// For operator<<
#include <ostream>

#include "renderer.hpp"
class Memory;

// Atari Digital Vector Generator

class DVG
{
public:
    DVG(Renderer &renderer, bool debug) : _renderer(renderer), _debug_mode{debug}
    {
        reset();
    }

    friend std::ostream &operator<<(std::ostream &os, const DVG &d);

    void reset();

    void render_loop(Memory &memory);

private:
    void process_instruction(Memory &memory);

    // _pc is index into Vector RAM, an array of WORDs.
    uint16_t _pc = 0;

    // The DVG keeps up with a current (x,y) cursor coordinate.
    int16_t _x = 0;
    int16_t _y = 0;

    // Global scaling factor.
    // Negative SF indicates divide (>>n), positive indicates multiply (<<n)
    int8_t _sf = 0;

    // Stack (4 WORDs) and stack pointer.
    std::array<uint16_t, 4> _stack{};
    uint8_t _sp = 0;

    bool _debug_mode = false;
    Renderer _renderer;
};

#endif // DVG_HPP
