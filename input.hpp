#ifndef INPUT_HPP
#define INPUT_HPP

#include <SDL_keycode.h> // SDL_Keycode

class Memory;

namespace Input
{
    enum class KeyState
    {
        Up,
        Down,
    };

    bool process_sdl_events(Memory &memory);
};

#endif // INPUT_HPP
