#include "memory.hpp"

Memory::Memory(
    const std::array<uint8_t, 6 * 1024> &game_rom,
    const std::array<uint8_t, 2 * 1024> &dvg_rom,
    bool debug_mode)
    : _game_rom{game_rom}, _dvg_rom{dvg_rom}, _debug_mode(debug_mode), mmio{*this}
{
}

uint8_t Memory::get_byte_at(uint16_t addr) const
{
    uint8_t byte;

    // The upper address line is ignored.
    // Thus the interrupt vectors beginning at FFFA map to the ROM space at 7FFA.
    addr &= 0x7FFF;

    // 0000:3FFF RAM (1KB) - Zero Page (256b), Stack (256b), Game RAM

    if (addr <= 0xFF)
    {
        byte = _zero_page.at(addr);
    }
    else if (addr <= 0x1FF)
    {
        byte = _stack.at(addr - 0x100);
    }
    else if (addr <= 0x3ff)
    {
        byte = _game_ram.at(addr - 0x200);
    }

    // 0x400:0x1FFF unused.

    // 2000:3FFF Various I/O

    else if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        byte = _mapped_io.at(addr - 0x2000);
    }

    // 4000:47FF Vector RAM

    else if (addr >= 0x4000 && addr <= 0x4FFF)
    {
        byte = _dvg_ram.at(addr - 0x4000);
    }

    // 5000:57FF Vector ROM

    else if (addr >= 0x5000 && addr <= 0x57FF)
    {
        byte = _dvg_rom.at(addr - 0x5000);
    }

    // 5800:0x67FF unused.

    // 6800:7FFF Game ROM

    else if (addr >= 0x6800 /* && addr <= 0x7FFF */)
    {
        byte = _game_rom.at(addr - 0x6800);
    }
    else
    {
        fail_fast("Invalid address");
    }

    return byte;
}

// Returns *addr | *(addr+1) << 8
uint16_t Memory::get_word_at(uint16_t addr) const
{
    return static_cast<uint16_t>(get_byte_at(addr)) | static_cast<uint16_t>(get_byte_at(addr + 1) << 8);
}

// for operator<<
#include <bitset>
#include <iomanip>
#include <iostream>

void Memory::set_byte_at(uint16_t addr, uint8_t byte)
{
    // The upper address line is ignored.
    // Thus the interrupt vectors beginning at FFFA map to the ROM space at 7FFA.
    addr &= 0x7FFF;

    if (addr <= 0xFF)
    {
        // 0000:00FF - zero page
        _zero_page.at(addr) = byte;

        if (_debug_mode)
        {
            if (addr == 0x02 || addr == 0x03)
            {
                std::cout
                    << "[VecRamPtr] " << std::hex << std::nouppercase << std::setfill('0') << std::setw(4) << get_word_at(2)
                    << std::endl;
            }
        }
    }
    else if (addr <= 0x1FF)
    {
        // 0100:01FF -  stack
        _stack.at(addr - 0x100) = byte;
    }
    else if (addr <= 0x3ff)
    {
        // 0200:03FF - Game RAM
        _game_ram.at(addr - 0x200) = byte;
    }
    // 0x400 - 0x1FFF unused.
    else if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        _mapped_io.at(addr - 0x2000) = byte;
    }
    else if (addr >= 0x4000 && addr <= 0x4FFF)
    {
        // 4000-4FFF / C000-CFFF
        _dvg_ram.at(addr - 0x4000) = byte;
    }
    else
    {
        // 2K vector ROM (5000:57FF): read-only
        // 6K game ROM (6800:7FFF): read-only
        fail_fast("Attempt to set ROM memory");
    }
}
