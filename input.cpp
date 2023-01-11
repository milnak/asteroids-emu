#include "input.hpp"
#include "memory.hpp"
#include "mmio.hpp"

#include <SDL.h>
#include <SDL_opengl.h>

#include <stdint.h>

namespace Input
{
    static void update_from_input(SDL_Keycode keycode, KeyState keystate, Memory &memory)
    {
        const uint8_t mem_val = (keystate == KeyState::Down) ? 0xFF : 0;

        switch (keycode)
        {
        case SDLK_LEFT:
            // P1 Button 1
            memory.mmio.set_IN1_SWROTLEFT(mem_val);
            break;

        case SDLK_RIGHT:
            // P1 Button 2
            memory.mmio.set_IN1_SWROTRGHT(mem_val);
            break;

        case SDLK_LCTRL:
            // P1 Button 3
            memory.mmio.set_IN0_SWFIRE(mem_val);
            break;

        case SDLK_LALT:
            // P1 Button 4
            memory.mmio.set_IN1_SWTHRUST(mem_val);
            break;

        case SDLK_SPACE:
            // P1 Button 5
            memory.mmio.set_IN0_SWHYPER(mem_val);
            break;

        case SDLK_1:
            // 1 Player Start
            memory.mmio.set_IN1_SW1START(mem_val);
            break;

        case SDLK_2:
            // 1 Player Start
            memory.mmio.set_IN1_SW2START(mem_val);
            break;

        case SDLK_5:
            // Coin 1
            memory.mmio.set_IN1_SWLCOIN(mem_val);
            break;
        case SDLK_6:
            // Coin 2
            memory.mmio.set_IN1_SWCCOIN(mem_val);
            break;
        case SDLK_7:
            // Coin 3
            memory.mmio.set_IN1_SWRCOIN(mem_val);
            break;
        case SDLK_9:
            // Diagnostic Step
            memory.mmio.set_IN0_SWDIAGST(mem_val);
            break;
        case SDLK_t:
            // Tilt
            memory.mmio.set_IN0_SWSLAM(mem_val);
            break;
        case SDLK_F1:
            memory.mmio.set_IN0_SWTEST(mem_val);
            break;
        }
    }

    bool process_sdl_events(Memory &memory)
    {
        bool running = true;

        SDL_Event Event;
        while (running && SDL_PollEvent(&Event))
        {
            switch (Event.type)
            {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                if (Event.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = false;
                }
                else
                {
                    Input::update_from_input(Event.key.keysym.sym, Input::KeyState::Down, memory);
                }
                break;

            case SDL_KEYUP:
                Input::update_from_input(Event.key.keysym.sym, Input::KeyState::Up, memory);
                break;

            case SDL_WINDOWEVENT:
                break;
            }
        }

        return running;
    }
};
