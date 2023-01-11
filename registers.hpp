#ifndef REGISTERS_HPP
#define REGISTERS_HPP

void fail_fast(const char *msg);

#include "bit_macros.hpp"

#include <stdint.h>
#include <string>

// For operator<<
#include <ostream>

class Registers
{
public:
    Registers() {}

    friend std::ostream &operator<<(std::ostream &os, const Registers &r);

    void reset()
    {
        _a = 0;
        _x = 0;
        _y = 0;
        _pc = 0;

        // stack starts at 0x1FF
        _sp = 0xFF;

        _p = 0;
    }

    std::string p_to_string() const;

    // P (Processor Status)

    // Negative bit
    bool psr_N() const { return IS_BIT_SET(_p, 7); }
    // Overflow bit
    bool psr_V() const { return IS_BIT_SET(_p, 6); }
    // Bit 5 is not used.

    // Break bit
    bool psr_B() const { return IS_BIT_SET(_p, 4); }
    // Decimal mode bit
    bool psr_D() const { return IS_BIT_SET(_p, 3); }
    // Interrupt disable bit
    bool psr_I() const { return IS_BIT_SET(_p, 2); }
    // Zero bit
    bool psr_Z() const { return IS_BIT_SET(_p, 1); }
    // Carry bit
    bool psr_C() const { return IS_BIT_SET(_p, 0); }

    // Will set negative if bit 7 is set (indicates negative number).
    void set_psr_N_if_negative(uint8_t value)
    {
        if (IS_BIT_SET(value, 7))
        {
            SET_BIT(_p, 7);
        }
        else
        {
            CLEAR_BIT(_p, 7);
        }
    }
    // Overflow bit
    void set_psr_V(bool set) { SET_BIT_VALUE(_p, 6, set); }
    // Bit 5 is not used

    // Break bit
    void set_psr_B() { SET_BIT(_p, 4); }
    // Decimal mode bit
    void set_psr_D(bool set) { SET_BIT_VALUE(_p, 3, set); }
    // Interrupt disable bit
    void set_psr_I() { SET_BIT(_p, 2); }
    // Zero bit
    void set_psr_Z(bool set) { SET_BIT_VALUE(_p, 1, set); }
    void set_psr_Z_if_zero(uint8_t v) { set_psr_Z(!!(v == 0)); }
    // Carry bit
    void set_psr_C(bool set) { SET_BIT_VALUE(_p, 0, set); }

    uint8_t p() const { return _p; }
    void set_p(uint8_t p) { _p = p; }

    // A

    uint8_t a() const { return _a; }
    void set_a(uint8_t a) { _a = a; }

    // X

    uint8_t x() const { return _x; }
    void set_x(uint8_t x) { _x = x; }

    // Y

    uint8_t y() const { return _y; }
    void set_y(uint8_t y) { _y = y; }

    // PC

    uint16_t pc() const { return _pc; }
    void set_pc(uint16_t pc) { _pc = pc; }

    // SP

    void decrement_sp()
    {
        if (_sp == 0)
        {
            // The CPU does not detect if the stack is overflowed by excessive pushing or
            // pulling operations and will most likely result in the program crashing.

            fail_fast("Stack underflow");
        }

        --_sp;
    }

    void increment_sp()
    {
        if (_sp == 0xff)
        {
            // The CPU does not detect if the stack is overflowed by excessive pushing or
            // pulling operations and will most likely result in the program crashing.

            fail_fast("Invalid overflow");
        }

        ++_sp;
    }

    uint16_t sp() const { return _sp; }
    void set_sp(uint8_t sp) { _sp = sp; }

private:
    // Program Counter (PC)
    //
    // 16 bit register which points to the next instruction to be executed.
    // The value of program counter is modified automatically as instructions are executed.
    // The value of the program counter can be modified by executing a jump, a relative branch
    // or a subroutine call to another memory address or by returning from a subroutine or interrupt.
    // bits 15-8: PCH; 7-0: PCL

    uint16_t _pc = 0;

    // Stack Pointer (SP)
    //
    // The processor supports a 256 byte stack located between $0100 and $01FF.
    // The stack pointer is an 8 bit register and holds the low 8 bits of the next
    // free location on the stack. The location of the stack is fixed and cannot be moved.
    // Pushing bytes to the stack causes the stack pointer to be decremented.
    // Conversely pulling bytes causes it to be incremented.
    // The CPU does not detect if the stack is overflowed by excessive pushing or
    // pulling operations and will most likely result in the program crashing.

    uint8_t _sp = 0;

    // Accumulator

    uint8_t _a = 0;

    // Index Register X

    uint8_t _x = 0;

    // Index Register Y

    uint8_t _y = 0;

    // Processor Status Register (P) (7 status bits)
    //
    //  N  NEGATIVE
    //  V  OVERFLOW
    //  B  BREAK COMMAND
    //  D  DECIMAL MODE
    //  I  INTERRUPT DISABLE
    //  Z  ZERO
    //  C  CARRY

    uint8_t _p = 0;
};

#endif
