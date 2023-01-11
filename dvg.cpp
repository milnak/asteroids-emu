#include "dvg.hpp"

#include "renderer.hpp"
#include "memory.hpp"
#include "mmio.hpp"
#include "bit_macros.hpp"

#include <algorithm>

// for operator<<
#include <bitset>
#include <iomanip>
#include <iostream>
#include <sstream>

// Memory Layout:
//
// The DVG reads lines from an 8K bank of memory.
// The first 4K (4000:4FFF) is RAM.
// The last 4K  (5000:5FFF) is ROM containing vector sequences for the game: asteroids, letters and numbers, ships, UFOs, etc.
//
// The "display list" is much more than a list of lines.
// It is actually an opcode language that includes the ability to jump to one of these ROM subroutines and then back to the next command.
// The main program fills the display RAM with "Move to" commands and "subroutine" commands to draw all the game object lines on the screen.
// The last command in RAM is the "Halt" command (0B) that tells the display hardware it has reached the end of the list.
//
// The DVG and main CPU share this 8K area of memory. The main CPU writes to it and the vector generator reads from it.
// The main CPU writes to this area as individual bytes. But the DVG itself reads the area as two-bytes words.
// The bytes are read little-endian -- least significant byte first.

int16_t calculate_delta(uint16_t val, int16_t shift_amount, bool negate)
{
    uint16_t result;

    if (shift_amount >= 0)
    {
        result = val >> shift_amount;
    }
    else
    {
        result = val << (-shift_amount);
    }

    if (negate)
    {
        result = -result;
    }

    return result;
}


void DVG::reset()
{
    _pc = 0;
    _x = 0;
    _y = 0;
    _sf = 0;
    std::fill(_stack.begin(), _stack.end(), 0);
    _sp = 0;
}

std::ostream &operator<<(std::ostream &os, const DVG &dvg)
{
    os
        << std::dec
        << "(" << std::setw(4) << dvg._x << ", " << std::setw(4) << dvg._y << ") "
        << "  SF: " << static_cast<int>(dvg._sf)
        << std::hex
        << "  SP: " << std::nouppercase << std::setfill('0') << std::setw(2) << static_cast<int>(dvg._sp);
    return os;
}

void DVG::render_loop(Memory &memory)
{
    // Halt            .eq     $2002                   ;Halt gives the vector state machine status
    memory.mmio.set_IN0_HALT(1);

    // DmaGo           .eq     $3000                   ;Writing this address starts the vector state machine.
    memory.mmio.set_OTHER_GODVG(0);

    reset();

    _renderer.clear();

    // Will be reset back to 0 when DVG HALT instruction is processed.
    while (memory.mmio.get_IN0_HALT() != 0)
    {
        process_instruction(memory);
    }

    _renderer.present();
}

void DVG::process_instruction(Memory &memory)
{
    if (_debug_mode)
    {
        // TODO: dump bytes (1 or 2, depending on instruction) as well.
        std::cout << "[DVG] " << *this << "  " << std::nouppercase << std::setfill('0') << std::setw(4) << _pc << ": ";
    }

    // 4000:47FF - VRAM	Vector RAM DVG word
    const uint16_t VRAM_BASE = 0x4000u;

    uint16_t word0{memory.get_word_at(VRAM_BASE + (_pc * 2))};
    ++_pc;

    // Grab instruction from top nibble, and then zero those bits for convenience.
    const uint8_t instruction{HINIBBLE(HIBYTE(word0))};
    word0 &= 0x0FFF;

    if (instruction == 0x0A)
    {
        //
        // LABS
        //

        const uint16_t word1{memory.get_word_at(VRAM_BASE + (_pc * 2))};
        ++_pc;

        const uint16_t X = word1 & 0x3ff /* 11 1111 1111 */;
        const bool Xs = IS_BIT_SET(word1, 10);

        const uint16_t Y = word0 & 0x3ff /* 11 1111 1111 */;
        const bool Ys = IS_BIT_SET(word0, 10);

        const uint8_t SF = (word1 & 0xF000 /* 1111 0000 0000 0000 */) >> 12;

        if (_debug_mode)
        {
            std::cout
                << ";LABS"
                << std::dec
                << " x=" << X
                << " y=" << Y
                << " sc=" << std::setfill('0') << std::setw(2) << static_cast<int>(SF)
                << std::endl;
        }

        // Sign-magnitude form stores the absolute value of the number in
        // the least significant bits, and the sign in the most significant bit. The sign bit on the
        // DVG is high when a number is negative.
        _x = Xs ? (0 - static_cast<int16_t>((X ^ 0x3FF) + 1)) : static_cast<int16_t>(X);
        _y = Ys ? (0 - static_cast<int16_t>((Y ^ 0x3FF) + 1)) : static_cast<int16_t>(Y);

        // Whether to SHL or SHR, and by how many bits.
        _sf = (SF < 8) ? (0 - SF) : (16 - SF);
    }
    else if (instruction <= 0x09 || instruction == 0x0f)
    {
        if (instruction <= 0x09)
        {
            //
            // VCTR
            //

            const uint16_t word1{memory.get_word_at(VRAM_BASE + (_pc * 2))};
            ++_pc;

            const uint8_t SF = (9 - instruction);
            const int16_t shift_amount = _sf + SF;

            const uint16_t X = word1 & 0x3ff /* 11 1111 1111 */;
            const bool Xs = IS_BIT_SET(word1, 10);
            const int16_t delta_x = calculate_delta(X, shift_amount, Xs);

            const uint16_t Y = word0 & 0x3ff /* 11 1111 1111 */;
            const bool Ys = IS_BIT_SET(word0, 10);
            const int16_t delta_y = calculate_delta(Y, shift_amount, Ys);

            const uint8_t Z = (word1 & 0xF000 /* 1111 0000 0000 0000 */) >> 12;
            if (Z != 0)
            {
                //  (0,0) is the lower left corner of the display. (1023,1023) is the upper right corner of the display.
                _renderer.draw_line(_x, 1023 - _y, _x + delta_x, 1023 - (_y + delta_y), Z);
            }

            if (_debug_mode)
            {
                std::cout << ";VCTR"
                          << std::dec
                          << " x=" << (Xs ? '-' : '+') << X
                          << " y=" << (Ys ? '-' : '+') << Y
                          << " sc=" << static_cast<int>(SF)
                          << " b=" << static_cast<int>(Z)
                          << std::endl;
            }

            _x += delta_x;
            _y += delta_y;
        }
        else
        {
            //
            // SVEC
            //

            const bool SF0 = IS_BIT_SET(word0, 11);
            const bool SF1 = IS_BIT_SET(word0, 3);
            const uint16_t SF = 7 - ((SF1 << 1) | static_cast<uint8_t>(SF0));

            const int16_t shift_amount = _sf + SF;

            const uint16_t X = (word0 & 0x3 /* 0011 */) << 8;
            const bool Xs = IS_BIT_SET(word0, 2);
            const int16_t delta_x = calculate_delta(X, shift_amount, Xs);

            const uint16_t Y = (word0 & 0x300 /* 11 0000 0000 */);
            const bool Ys = IS_BIT_SET(word0, 10);
            const int16_t delta_y = calculate_delta(Y, shift_amount, Ys);

            const uint8_t Z = (word0 & 0xF0 /* 1111 0000 */) >> 4;
            if (Z != 0)
            {
                //  (0,0) is the lower left corner of the display. (1023,1023) is the upper right corner of the display.
                _renderer.draw_line(_x, 1023 - _y, _x + delta_x, 1023 - (_y + delta_y), Z);
            }

            if (_debug_mode)
            {
                std::cout << ";SVEC"
                          << " x=" << (Xs ? '-' : '+') << static_cast<int>(X)
                          << " y=" << (Ys ? '-' : '+') << static_cast<int>(Y)
                          << " sc=" << static_cast<int>(SF)
                          << "  b=" << static_cast<int>(Z)
                          << std::endl;
            }

            _x += delta_x;
            _y += delta_y;
        }
    }
    else if (instruction == 0x0B)
    {
        // HALT
        // End the current drawing list.

        if (_debug_mode)
        {
            std::cout
                << ";HALT" << std::endl;
        }

        memory.mmio.set_IN0_HALT(0);
    }
    else if (instruction == 0x0C)
    {
        // JSRL (JSR)

        // Jump to a vector subroutine.
        // Note that there is room in the internal "stack" for only FOUR levels of nested subroutine calls.
        // Note that the target address is the WORD address -- not the byte address.

        // Example:
        //              ; 1100 aaaa aaaa aaaa
        // CAE4         ; 1100 1010 1110 0100
        //              ; - a is the word address of the destination

        // In this case:
        //   a = 0AE4
        //   Conversion to byte address: (0AE4-0800)*2 + 0800 = 0DC8

        if (_debug_mode)
        {
            std::cout
                << ";JSRL a=$" << std::hex << std::setfill('0') << std::setw(4) << word0 << std::endl;
        }

        // C++ exception thrown on stack overflow.
        _stack.at(_sp) = _pc;

        ++_sp;

        _pc = word0;
    }
    else if (instruction == 0x0D)
    {
        // RTSL (RTS)
        // Return from current vector subroutine.

        if (_debug_mode)
        {
            std::cout << ";RTSL" << std::endl;
        }

        --_sp;

        // C++ exception thrown on stack underflow.
        _pc = _stack.at(_sp);
    }
    else if (instruction == 0x0E)
    {
        // JMPL (JMP)
        // Jump to a new location in the vector program.

        // Note that the target address is the WORD address -- not the byte address.

        // Example:
        //              ; 1110 aaaa aaaa aaaa
        // EA0A         ; 1110 1010 0000 1010
        //              ; - a is the word address of the destination

        // In this case:
        //   a = 0A0A
        //   Conversion to byte address: (0A0A-0800)*2 + 0800 = 0C14

        if (_debug_mode)
        {
            std::cout
                << ";JMPL a=$" << std::hex << std::setfill('0') << std::setw(4) << word0 << std::endl;
        }

        _pc = word0;
    }
    else
    {
        fail_fast("Invalid instruction");
    }
}
