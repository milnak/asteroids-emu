#ifndef MMIO_HPP
#define MMIO_HPP

#include <stdint.h>

class Memory;

//
// Memory Mapped IO
// Specific to Asteroids hardware.
//

class MMIO
{
public:
    MMIO(Memory &memory);

    //
    // IN0
    //

    enum IN0 : uint16_t
    {
        // 2001 CLCK3KHZ - 3 KHz Clock
        CLCK3KHZ = 0x2001,
        // 2002 HALT - HALT
        HALT = 0x2002,
        // 2003 SWHYPER - Hyperspace Switch
        SWHYPER = 0x2003,
        // 2004 SWFIRE - Fire Switch
        SWFIRE = 0x2004,
        // 2005 SWDIAGST - Diagnostic Step
        SWDIAGST = 0x2005,
        // 2006 SWSLAM - Slam Switch
        SWSLAM = 0x2006,
        // 2007 SWTEST - Self Test Switch
        SWTEST = 0x2007,
    };

    void set_IN0_CLCK3KHZ(uint8_t value) { set_IN0(MMIO::IN0::CLCK3KHZ, value); }

    // Vector state machine status. 1=busy, 0=idle
    uint8_t get_IN0_HALT() const { return get_IN0(MMIO::IN0::HALT); }
    void set_IN0_HALT(uint8_t value) { set_IN0(MMIO::IN0::HALT, value); }

    void set_IN0_SWHYPER(uint8_t value) { set_IN0(MMIO::IN0::SWHYPER, value); }

    void set_IN0_SWFIRE(uint8_t value) { set_IN0(MMIO::IN0::SWFIRE, value); }

    void set_IN0_SWDIAGST(uint8_t value) { set_IN0(MMIO::IN0::SWDIAGST, value); }
    void set_IN0_SWSLAM(uint8_t value) { set_IN0(MMIO::IN0::SWSLAM, value); }
    void set_IN0_SWTEST(uint8_t value) { set_IN0(MMIO::IN0::SWTEST, value); }

    //
    // IN1
    //

    enum IN1 : uint16_t
    {
        // 2400 SWLCOIN Left Coin Switch
        SWLCOIN = 0x2400,
        // 2401 SWCCOIN Center Coin Switch
        SWCCOIN = 0x2401,
        // 2402 SWRCOIN Right Coin Switch
        SWRCOIN = 0x2402,
        // 2403 SW1START 1 Player Start Switch
        SW1START = 0x2403,
        // 2404 SW2START 2 Player Start Switch
        SW2START = 0x2404,
        // 2405 SWTHRUST Thrust Switch
        SWTHRUST = 0x2405,
        // 2406 SWROTRGHT Rotate Right Switch
        SWROTRGHT = 0x2406,
        // 2407 SWROTLEFT Rotate Left Switch
        SWROTLEFT = 0x2407,
    };

    void set_IN1_SWLCOIN(uint8_t value) { set_IN1(MMIO::IN1::SWLCOIN, value); }
    void set_IN1_SWCCOIN(uint8_t value) { set_IN1(MMIO::IN1::SWCCOIN, value); }
    void set_IN1_SWRCOIN(uint8_t value) { set_IN1(MMIO::IN1::SWRCOIN, value); }
    void set_IN1_SW1START(uint8_t value) { set_IN1(MMIO::IN1::SW1START, value); }
    void set_IN1_SW2START(uint8_t value) { set_IN1(MMIO::IN1::SW2START, value); }
    void set_IN1_SWTHRUST(uint8_t value) { set_IN1(MMIO::IN1::SWTHRUST, value); }
    void set_IN1_SWROTRGHT(uint8_t value) { set_IN1(MMIO::IN1::SWROTRGHT, value); }
    void set_IN1_SWROTLEFT(uint8_t value) { set_IN1(MMIO::IN1::SWROTLEFT, value); }

    //
    // DSW1
    //

    enum DSW1 : uint16_t
    {
        // 2800 SWCOINAGE Coinage 0 = Free Play, 1 = 1 Coin 2 Credits, 2 = 1 Coin 1 Credit, 3 = 2 Coins 1 Credit
        SWCOINAGE = 0x2800,
        // 2801 SWCNRMULT Right Coin Multiplier 0 = 1x, 1 = 4x, 2 = 5x, 3 = 6x
        SWCNRMULT = 0x2801,
        // 2802 SWCNCMULT Center Coin Multiplier & Starting Lives 0 = 1x & 4, 1 = 1x & 3, 2 = 2x & 4, 3 = 2x & 3
        SWCNCMULT = 0x2802,
        // 2803 SWLANGUAGE Language 0 = English, 1 = German, 2 = French, 3 = Spanish
        SWLANGUAGE = 0x2803,
    };

    //
    // OTHER
    //

    enum OTHER : uint16_t
    {
        // 3000 GODVG AVG/DVG Go
        GODVG = 0x3000,

        // 3200 LMPSCNS Bit 1 = 2 Player Start Lamp, Bit 2 = 1 Player Start Lamp, Bit 3 = RAMSEL, Bit 4 = Left Coin Counter, Bit 5 = Center Coin Counter, Bit 6 = Right Coin Counter
        LMPSCNS = 0x3200,

        // 3400 WATCHDOG Watchdog
        WATCHDOG = 0x3400,

        // 3600 SNDEXP Sound (explosion)
        SNDEXP = 0x3600,

        // 3A00 SNDTHUMP Sound (thump)
        SNDTHUMP = 0x3A00,

        // 3C00 SNDSAUCR Sound (saucer)
        SNDSAUCR = 0x3C00,
        // 3C01 SNDSFIRE Sound (saucer fire)
        SNDSFIRE = 0x3C01,
        // 3C02 SNDSELSAU Sound Select (large/small saucer)
        SNDSELSAU = 0x3C02,
        // 3C03 SNDTHRUST Sound (ship thrust)
        SNDTHRUST = 0x3C03,
        // 3C04 SNDFIRE Sound (ship fire)
        SNDFIRE = 0x3C04,
        // 3C05 SNDBONUS Sound (bonus life)
        SNDBONUS = 0x3C05,

        // 3E00 SNDRESET Sound (noise reset)
        SNDRESET = 0x3E00,
    };

    uint8_t get_OTHER_GODVG() const { return get_OTHER(MMIO::OTHER::GODVG); }

    void set_OTHER_GODVG(uint8_t value) { set_OTHER(MMIO::OTHER::GODVG, value); }

private:
    uint8_t get_IN0(MMIO::IN0 addr) const;
    void set_IN0(MMIO::IN0 addr, uint8_t value);
    void set_IN1(MMIO::IN1 addr, uint8_t value);
    void set_DSW1(MMIO::DSW1 addr, uint8_t value);
    uint8_t get_OTHER(MMIO::OTHER addr) const;
    void set_OTHER(MMIO::OTHER addr, uint8_t value);

    Memory &_memory;
};

#endif
