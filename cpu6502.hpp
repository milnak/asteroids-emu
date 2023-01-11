#ifndef CPU6502_HPP
#define CPU6502_HPP

void fail_fast(const char *msg);

#include "registers.hpp"

#include <stdint.h>
#include <string>
#include <vector>

// For operator<<
#include <ostream>

class Memory;
class CPU6502;

enum /*class*/ Instruction
{
    Invalid,

    // Load/Store Operations
    LDA,
    LDX,
    LDY,
    STA,
    STX,
    STY,

    // Register Transfers
    TAX,
    TAY,
    TXA,
    TYA,

    // Stack Operations
    TSX,
    TXS,
    PHA,
    PHP,
    PLA,
    PLP,

    // Logical
    AND,
    EOR,
    ORA,
    BIT,

    // Arithmetic
    ADC,
    SBC,
    CMP,
    CPX,
    CPY,

    // Increments & Decrements
    INC,
    INX,
    INY,
    DEC,
    DEX,
    DEY,

    // Shifts
    ASL,
    LSR,
    ROL,
    ROR,

    // Jumps & Calls
    JMP,
    JSR,
    RTS,

    // Branches
    BCC,
    BCS,
    BEQ,
    BMI,
    BNE,
    BPL,
    BVC,
    BVS,

    // Status Flag Changes
    CLC,
    CLD,
    CLI,
    CLV,
    SEC,
    SED,
    SEI,

    // System Functions
    BRK,
    NOP,
    RTI,
};

enum class AddressingMode
{
    Invalid,
    Implied,         // aka Implicit; e.g. TXS, CLC, RTS
    Accumulator,     // e.g. LSR A, ROR A
    Immediate,       // e.g. LDA #10
    ZeroPage,        // e.g. LDA $00
    ZeroPageX,       // e.g. STY $10,X
    ZeroPageY,       // e.g. LDX $10,Y
    Relative,        // e.g. BEQ LABEL, BNE *+4
    Absolute,        // e.g. JMP $1234
    AbsoluteX,       // e.g. STA $3000,X
    AbsoluteY,       // e.g. AND $4000,Y
    Indirect,        // e.g. JMP ($FFFC)
    IndirectX, // e.g. LDA ($40,X)
    IndirectY, // e.g. LDA ($40),Y
};

class CPU6502
{
public:
    CPU6502(Memory &memory, bool debug, const std::vector<uint16_t>& breakpoints);

    friend std::ostream &operator<<(std::ostream &os, const CPU6502 &r);

    void run_until_tick();

    void reset();

    void execute_instruction();

    void initiate_nmi();

    void push_stack_byte(uint8_t store);
    uint8_t pop_stack_byte();

    void push_stack_word(uint16_t store);
    uint16_t pop_stack_word();

    uint64_t get_cycle() const
    {
        return _cycle;
    }

private:
    uint8_t get_resolved_byte(uint8_t value, AddressingMode addressing_mode);
    void set_resolved_byte(uint8_t addr, AddressingMode addressing_mode, uint8_t value);

    uint16_t resolve_address(uint16_t value, AddressingMode addressing_mode);

    void process_addressing_mode_accumulator(Instruction instruction);
    void process_addressing_mode_implied(Instruction instruction);
    bool process_addressing_mode_relative(Instruction instruction);
    void process_addressing_mode_immediate(Instruction instruction, uint8_t M);
    void process_addressing_mode_absolute(Instruction instruction, uint16_t M);
    void process_addressing_mode_byte(uint8_t operand_value, Instruction instruction, AddressingMode addressing_mode);
    void process_addressing_mode_word(uint16_t operand_value, Instruction instruction, AddressingMode addressing_mode);

    Memory &_memory;

    bool _debug_mode = false;
    std::vector<uint16_t> _breakpoints;

    Registers _registers;

    // Cycle count to determine when to perform NMI, etc.
    uint64_t _cycle = 0;
};

#endif // CPU6502_HPP
