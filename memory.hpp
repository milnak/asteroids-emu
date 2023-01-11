#ifndef MEMORY_HPP
#define MEMORY_HPP

void fail_fast(const char *msg);

#include <array>

#include "mmio.hpp"

//
// Memory
//

class Memory
{
public:
    Memory(
        const std::array<uint8_t, 6 * 1024> &game_rom,
        const std::array<uint8_t, 2 * 1024> &vector_rom,
        bool debug_mode);

    MMIO mmio;

    uint8_t get_byte_at(uint16_t addr) const;

    uint16_t get_word_at(uint16_t addr) const;

    void set_byte_at(uint16_t addr, uint8_t byte);

    // FFFA:FFFB = addresses of the non-maskable interrupt handler
    uint16_t get_NMI_handler_address() const
    {
        return get_word_at(0xFFFA);
    }

    // FFFC:FFFD = the power on reset location
    uint16_t get_power_on_reset_address() const
    {
        return get_word_at(0xFFFC);
    }

    // FFFE:FFFF = BRK/interrupt request handler
    uint16_t get_brk_interrupt_request_handler_address() const
    {
        return get_word_at(0xFFFE);
    }

private:
    bool _debug_mode;

    // 0000:00FF - zero page
    std::array<uint8_t, 256> _zero_page{};

    // 0100:01FF - system stack
    std::array<uint8_t, 256> _stack{};

    // 0200:03FF - Game RAM
    std::array<uint8_t, 512> _game_ram{} ;

    // 2000:3FFF - Mapped IO
    std::array<uint8_t, 8 * 1024> _mapped_io{};

    // 4000:5FFF (8K bytes): Shared DVG and CPU  memory space.
    // The DVG reads this as 4K words.
    // Only half of the RAM/ROM space is actually populated.
    // There is room for expansion.

    // 4000:47FF - VRAM	- Vector RAM DVG word
    // 2K vector RAM (+ 2K unused)

    std::array<uint8_t, 2 * 1024 + 2 * 1024> _dvg_ram{};

    // 4800:4FFF - VRAMunusd	Unused VRAM DVG word

    // 5000:47FF - VROM	- Vector ROM DVG word

    // 2K vector ROM
    std::array<uint8_t, 2 * 1024> _dvg_rom{};

    // 5800:5FFF- VROMunusd	- Unused VROM DVG word

    // 6000:7FFF - Game ROM (6K)
    std::array<uint8_t, 6 * 1024> _game_rom{};
};

#endif // MEMORY_HPP
