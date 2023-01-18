﻿#include <time.h>
#include <stdint.h>
#include "getopt/getopt.h"

#include <iostream>  // std::cout
#include <algorithm> // std::copy
#include <iomanip>   // std::setfill
#include <vector>

#include "renderer.hpp"
#include "cpu6502.hpp"
#include "dvg.hpp"
#include "input.hpp"
#include "memory.hpp"
#include "mmio.hpp"

// Generated by roms/generate-romcode.ps1
#include "build/romcode.h"

// CPU tracing
#define DEBUG_FLAG_CPU (1 << 0)
// DVG tracing
#define DEBUG_FLAG_DVG (1 << 1)
// Renderer tracing
#define DEBUG_FLAG_RENDERER (1 << 2)
// Memory tracing
#define DEBUG_FLAG_MEMORY (1 << 3)
// Misc tracing
#define DEBUG_FLAG_MISC (1 << 4)

void fail_fast(const char *msg)
{
    std::cout << "FATAL: " << msg << std::endl;
    __debugbreak();
}

#if 0
static const uint32_t TICKS_PER_SLEEP = 20;

struct timespec
{
    long tv_sec;
    long tv_nsec;
}; // header part

void clock_gettime(struct timespec *spec)
{
    __int64 wintime;
    GetSystemTimeAsFileTime((FILETIME *)&wintime);
    wintime -= 116444736000000000i64;            // 1jan1601 to 1jan1970
    spec->tv_sec = wintime / 10000000i64;        // seconds
    spec->tv_nsec = wintime % 10000000i64 * 100; // nano-seconds
}
#endif

// Processes instructions until next tick is reached.
static void run_until_tick(CPU6502 &cpu, Memory &memory, DVG &dvg, bool debug_mode)
{
    // CPU execution loop

    // The CPU runs at 1.5MHz (1500000 Hz)
    // The MMI interrupt is clocked at 250Hz.
    const uint64_t NMI_CYCLES = (1500000 / 250); // 6000

    // The CPU runs at 1.5MHz (1500000 Hz)
    // Set the 3 KHz Clock (3000 Hz)
    const uint16_t CLOCK_3KHZ = (1500000 / 3000); // 500

    static uint64_t next_nmi = NMI_CYCLES;

    // Next CLOCK_3KHZ multiple, e.g. 500, 1000, ...
    const uint64_t next_3khz_cycle = ((cpu.get_cycle() / CLOCK_3KHZ) + 1) * CLOCK_3KHZ;

    while (cpu.get_cycle() < next_3khz_cycle)
    {
        while (cpu.get_cycle() < next_3khz_cycle && memory.mmio.get_OTHER_GODVG() == 0)
        {
            cpu.execute_instruction();

            // If enough cycles have passed, initiate NMI.

            if (cpu.get_cycle() >= next_nmi)
            {
                cpu.initiate_nmi();

                next_nmi += NMI_CYCLES;
            }
        }

        // If GODVG was set, call DVG to render.

        if (memory.mmio.get_OTHER_GODVG() != 0)
        {
            // DVG render resets GODVG
            dvg.render_loop(memory);
        }

        // TODO: Play any sounds.

        // Sounds_Play(memory);

        // Update 3KHz clock
        //
        // Example code:
        // Waits for Clk3Khz to go negative then positive again
        //
        //                    :SelfTestWaitLoop
        // Sets negative flag set if bit 7 of A is set.
        // 7e75: ad 01 20                     lda     Clk3Khz
        // If the negative flag is clear then branch
        // 7e78: 10 fb                        bpl     SelfTestWaitLoop
        // 7e7a: ad 01 20     :Loop           lda     Clk3Khz
        // If the negative flag is set then branch
        // 7e7d: 30 fb                        bmi     :Loop

        // This counter is incremented once every 3KHz (500 cycles).
        const uint8_t clck3khz{(cpu.get_cycle() / CLOCK_3KHZ) & 0xFF};
        memory.mmio.set_IN0_CLCK3KHZ(clck3khz);

        if (debug_mode)
        {
            std::cout << "[CLK3KHZ] "
                      << std::hex << std::nouppercase << std::setfill('0') << std::setw(2) << static_cast<int>(clck3khz)
                      << std::endl;
        }
    }
}

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
    (byte & 0x80 ? '1' : '0'),     \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')

// e.g. -b 0x7b7b -t 1
int main(int argc, char **argv)
{
    bool running = true;
    uint8_t debug_flags = 0;
    std::vector<uint16_t> breakpoints;

    int trace_bits = 0;

    while (1)
    {
        static struct option long_options[] =
            {
                {"breakpoint", required_argument, 0, 'b'},
                {"trace", required_argument, 0, 't'},
                {0, 0, 0, 0}};

        int option_index = 0;
        const int c = getopt_long(argc, argv, "b:t:", long_options, &option_index);
        if (c == -1)
        {
            break;
        }

        switch (c)
        {
            uint16_t v;

        case 'b':
            // --breakpoint 0x736a (can be used multiple times)
            v = static_cast<uint16_t>(std::stoul(optarg, nullptr, 16));
            printf("Added breakpoint at $%X\n", v);
            breakpoints.push_back(v);
            break;
        case 't':
            // --trace 111
            v = static_cast<uint16_t>(std::stoul(optarg, nullptr, 2));
            debug_flags = static_cast<uint8_t>(v);
            printf("Debug flags: " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(v));
            break;
        case '?':
            // '?' returned in case of help or invalid parameter.
            return 0;
        }
    }

#if 0
    // In nanoseconds
    int tick_time = 1000000000 / 3000 * TICKS_PER_SLEEP; // 6666667
#endif

    // Load in ROM images.

    std::array<uint8_t, 2 * 1024> dvg_rom_array;
    std::copy(std::begin(dvg_rom), std::end(dvg_rom), dvg_rom_array.begin());

    std::array<uint8_t, 6 * 1024> game_rom_array;
    std::copy(std::begin(game_rom), std::end(game_rom), game_rom_array.begin());

    // Construct system objects.

    Memory memory(game_rom_array, dvg_rom_array, debug_flags & DEBUG_FLAG_MEMORY);
    Renderer renderer(debug_flags & DEBUG_FLAG_RENDERER);
    DVG dvg(renderer, debug_flags & DEBUG_FLAG_DVG);
    CPU6502 cpu(memory, debug_flags & DEBUG_FLAG_CPU, breakpoints);

    //
    // Set DIP switches
    //

    // 2800 SWCOINAGE Coinage 0 = Free Play, 1 = 1 Coin 2 Credits, 2 = 1 Coin 1 Credit, 3 = 2 Coins 1 Credit
    memory.set_byte_at(MMIO::DSW1::SWCOINAGE, 2);
    // 2801 SWCNRMULT Right Coin Multiplier 0 = 1x, 1 = 4x, 2 = 5x, 3 = 6x
    memory.set_byte_at(MMIO::DSW1::SWCNRMULT, 0);
    // 2802 SWCNCMULT Center Coin Multiplier & Starting Lives 0 = 1x & 4, 1 = 1x & 3, 2 = 2x & 4, 3 = 2x & 3
    memory.set_byte_at(MMIO::DSW1::SWCNCMULT, 3);
    // 2803 SWLANGUAGE Language 0 = English, 1 = German, 2 = French, 3 = Spanish
    memory.set_byte_at(MMIO::DSW1::SWLANGUAGE, 0);

    while (running)
    {
#if 0
        struct timespec now;
        clock_gettime(&now);
#endif

#if 0
        for (uint64_t i = 0; running && (i < TICKS_PER_SLEEP); ++i)
        {
#endif
        running = Input::process_sdl_events(memory);
#if 0
        }         // for i
#endif
        if (!running)
        {
            break;
        }

        run_until_tick(cpu, memory, dvg, debug_flags & DEBUG_FLAG_MISC);

#if 0
        delta = now.elapsed();
        if (delta < tick_time)
        {
            sleep(tick_time - delta);
        }
#endif
    } // while Running

    return 0;
}
