#include "cpu6502.hpp"
#include "memory.hpp"
#include "bit_macros.hpp"

// for operator<<
#include <bitset>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <array> // std::array

struct OpcodeInfo
{
    uint8_t opcode;
    Instruction instruction;
    const char *opcode_string;
    AddressingMode addressing_mode;
    uint8_t bytes;
    // NOTE: Some opcodes have cycle exceptions
    uint8_t cycles;

    std::string operand_to_string(uint16_t operand, uint16_t pc) const
    {
        std::ostringstream str;

        switch (addressing_mode)
        {
        case AddressingMode::Implied:
            break;
        case AddressingMode::Accumulator:
            str << "A";
            break;
        case AddressingMode::Immediate:
            str << "#$" << std::hex << std::nouppercase << std::setfill('0') << std::setw(2) << static_cast<int>(operand);
            break;
        case AddressingMode::ZeroPage:
            str << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(operand);
            break;
        case AddressingMode::ZeroPageX:
            str << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(operand) << ",X";
            break;
        case AddressingMode::ZeroPageY:
            str << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(operand) << ",Y";
            break;
        case AddressingMode::Relative:
            // str << "(" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(operand) << ")";
            str << std::hex << std::setfill('0') << std::setw(4) << pc + static_cast<int8_t>(operand);
            break;
        case AddressingMode::Absolute:
            str << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(operand);
            break;
        case AddressingMode::AbsoluteX:
            str << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(operand) << ",X";
            break;
        case AddressingMode::AbsoluteY:
            str << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(operand) << ",Y";
            break;
        case AddressingMode::IndirectX:
            str << "(" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(operand) << ",X)";
            break;
        case AddressingMode::IndirectY:
            str << "(" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(operand) << "),Y";
            break;
        default:
            fail_fast("Invalid addressing mode");
            break;
        }

        return str.str();
    }
};

#define INVALID_OPCODE(opcode)       \
    {                                \
        opcode, Instruction::Invalid \
    }

const std::array<OpcodeInfo, 256> OpcodeInfoTable{
    {
        {0x00, Instruction::BRK, "BRK", AddressingMode::Implied, 1, 7},
        {0x01, Instruction::ORA, "ORA", AddressingMode::IndirectX, 2, 6},
        INVALID_OPCODE(0x02),
        INVALID_OPCODE(0x03),
        INVALID_OPCODE(0x04),
        {0x05, Instruction::ORA, "ORA", AddressingMode::ZeroPage, 2, 3},
        {0x06, Instruction::ASL, "ASL", AddressingMode::ZeroPage, 2, 5},
        INVALID_OPCODE(0x07),
        {0x08, Instruction::PHP, "PHP", AddressingMode::Implied, 1, 3},
        {0x09, Instruction::ORA, "ORA", AddressingMode::Immediate, 2, 2},
        {0x0A, Instruction::ASL, "ASL", AddressingMode::Accumulator, 1, 2},
        INVALID_OPCODE(0x0B),
        INVALID_OPCODE(0x0C),
        {0x0D, Instruction::ORA, "ORA", AddressingMode::Absolute, 3, 4},
        {0x0E, Instruction::ASL, "ASL", AddressingMode::Absolute, 3, 6},
        INVALID_OPCODE(0x0F),
        {0x10, Instruction::BPL, "BPL", AddressingMode::Relative, 2, 2},
        {0x11, Instruction::ORA, "ORA", AddressingMode::IndirectY, 2, 5},
        INVALID_OPCODE(0x12),
        INVALID_OPCODE(0x13),
        INVALID_OPCODE(0x14),
        {0x15, Instruction::ORA, "ORA", AddressingMode::ZeroPageX, 2, 4},
        {0x16, Instruction::ASL, "ASL", AddressingMode::ZeroPageX, 2, 6},
        INVALID_OPCODE(0x17),
        {0x18, Instruction::CLC, "CLC", AddressingMode::Implied, 1, 2},
        {0x19, Instruction::ORA, "ORA", AddressingMode::AbsoluteY, 3, 4},
        INVALID_OPCODE(0x1A),
        INVALID_OPCODE(0x1B),
        INVALID_OPCODE(0x1C),
        {0x1D, Instruction::ORA, "ORA", AddressingMode::AbsoluteX, 3, 4},
        {0x1E, Instruction::ASL, "ASL", AddressingMode::AbsoluteX, 3, 7},
        INVALID_OPCODE(0x1F),
        {0x20, Instruction::JSR, "JSR", AddressingMode::Absolute, 3, 6},
        {0x21, Instruction::AND, "AND", AddressingMode::IndirectX, 2, 6},
        INVALID_OPCODE(0x22),
        INVALID_OPCODE(0x23),
        {0x24, Instruction::BIT, "BIT", AddressingMode::ZeroPage, 2, 3},
        {0x25, Instruction::AND, "AND", AddressingMode::ZeroPage, 2, 3},
        {0x26, Instruction::ROL, "ROL", AddressingMode::ZeroPage, 2, 5},
        INVALID_OPCODE(0x27),
        {0x28, Instruction::PLP, "PLP", AddressingMode::Implied, 1, 4},
        {0x29, Instruction::AND, "AND", AddressingMode::Immediate, 2, 2},
        {0x2A, Instruction::ROL, "ROL", AddressingMode::Accumulator, 1, 2},
        INVALID_OPCODE(0x2B),
        {0x2C, Instruction::BIT, "BIT", AddressingMode::Absolute, 3, 4},
        {0x2D, Instruction::AND, "AND", AddressingMode::Absolute, 3, 4},
        {0x2E, Instruction::ROL, "ROL", AddressingMode::Absolute, 3, 6},
        INVALID_OPCODE(0x2F),
        {0x30, Instruction::BMI, "BMI", AddressingMode::Relative, 2, 2},
        {0x31, Instruction::AND, "AND", AddressingMode::IndirectY, 2, 5},
        INVALID_OPCODE(0x32),
        INVALID_OPCODE(0x33),
        INVALID_OPCODE(0x34),
        {0x35, Instruction::AND, "AND", AddressingMode::ZeroPageX, 2, 4},
        {0x36, Instruction::ROL, "ROL", AddressingMode::ZeroPageX, 2, 6},
        INVALID_OPCODE(0x37),
        {0x38, Instruction::SEC, "SEC", AddressingMode::Implied, 1, 2},
        {0x39, Instruction::AND, "AND", AddressingMode::AbsoluteY, 3, 4},
        INVALID_OPCODE(0x3A),
        INVALID_OPCODE(0x3B),
        INVALID_OPCODE(0x3C),
        {0x3D, Instruction::AND, "AND", AddressingMode::AbsoluteX, 3, 4},
        {0x3E, Instruction::ROL, "ROL", AddressingMode::AbsoluteX, 3, 7},
        INVALID_OPCODE(0x3F),
        {0x40, Instruction::RTI, "RTI", AddressingMode::Implied, 1, 6},
        {0x41, Instruction::EOR, "EOR", AddressingMode::IndirectX, 2, 6},
        INVALID_OPCODE(0x42),
        INVALID_OPCODE(0x43),
        INVALID_OPCODE(0x44),
        {0x45, Instruction::EOR, "EOR", AddressingMode::ZeroPage, 2, 3},
        {0x46, Instruction::LSR, "LSR", AddressingMode::ZeroPage, 2, 5},
        INVALID_OPCODE(0x47),
        {0x48, Instruction::PHA, "PHA", AddressingMode::Implied, 1, 3},
        {0x49, Instruction::EOR, "EOR", AddressingMode::Immediate, 2, 2},
        {0x4A, Instruction::LSR, "LSR", AddressingMode::Accumulator, 1, 2},
        INVALID_OPCODE(0x4B),
        {0x4C, Instruction::JMP, "JMP", AddressingMode::Absolute, 3, 3},
        {0x4D, Instruction::EOR, "EOR", AddressingMode::Absolute, 3, 4},
        {0x4E, Instruction::LSR, "LSR", AddressingMode::Absolute, 3, 6},
        INVALID_OPCODE(0x4F),
        {0x50, Instruction::BVC, "BVC", AddressingMode::Relative, 2, 2},
        {0x51, Instruction::EOR, "EOR", AddressingMode::IndirectY, 2, 5},
        INVALID_OPCODE(0x52),
        INVALID_OPCODE(0x53),
        INVALID_OPCODE(0x54),
        {0x55, Instruction::EOR, "EOR", AddressingMode::ZeroPageX, 2, 4},
        {0x56, Instruction::LSR, "LSR", AddressingMode::ZeroPageX, 2, 6},
        INVALID_OPCODE(0x57),
        {0x58, Instruction::CLI, "CLI", AddressingMode::Implied, 1, 2},
        {0x59, Instruction::EOR, "EOR", AddressingMode::AbsoluteY, 3, 4},
        INVALID_OPCODE(0x5A),
        INVALID_OPCODE(0x5B),
        INVALID_OPCODE(0x5C),
        {0x5D, Instruction::EOR, "EOR", AddressingMode::AbsoluteX, 3, 4},
        {0x5E, Instruction::LSR, "LSR", AddressingMode::AbsoluteX, 3, 7},
        INVALID_OPCODE(0x5F),
        {0x60, Instruction::RTS, "RTS", AddressingMode::Implied, 1, 6},
        {0x61, Instruction::ADC, "ADC", AddressingMode::IndirectX, 2, 6},
        INVALID_OPCODE(0x62),
        INVALID_OPCODE(0x63),
        INVALID_OPCODE(0x64),
        {0x65, Instruction::ADC, "ADC", AddressingMode::ZeroPage, 2, 3},
        {0x66, Instruction::ROR, "ROR", AddressingMode::ZeroPage, 2, 5},
        INVALID_OPCODE(0x67),
        {0x68, Instruction::PLA, "PLA", AddressingMode::Implied, 1, 4},
        {0x69, Instruction::ADC, "ADC", AddressingMode::Immediate, 2, 2},
        {0x6A, Instruction::ROR, "ROR", AddressingMode::Accumulator, 1, 2},
        INVALID_OPCODE(0x6B),
        {0x6C, Instruction::JMP, "JMP", AddressingMode::Indirect, 3, 5},
        {0x6D, Instruction::ADC, "ADC", AddressingMode::Absolute, 3, 4},
        {0x6E, Instruction::ROR, "ROR", AddressingMode::Absolute, 3, 6},
        INVALID_OPCODE(0x6F),
        {0x70, Instruction::BVS, "BVS", AddressingMode::Relative, 2, 2},
        {0x71, Instruction::ADC, "ADC", AddressingMode::IndirectY, 2, 5},
        INVALID_OPCODE(0x72),
        INVALID_OPCODE(0x73),
        INVALID_OPCODE(0x74),
        {0x75, Instruction::ADC, "ADC", AddressingMode::ZeroPageX, 2, 4},
        {0x76, Instruction::ROR, "ROR", AddressingMode::ZeroPageX, 2, 6},
        INVALID_OPCODE(0x77),
        {0x78, Instruction::SEI, "SEI", AddressingMode::Implied, 1, 2},
        {0x79, Instruction::ADC, "ADC", AddressingMode::AbsoluteY, 3, 4},
        INVALID_OPCODE(0x7A),
        INVALID_OPCODE(0x7B),
        INVALID_OPCODE(0x7C),
        {0x7D, Instruction::ADC, "ADC", AddressingMode::AbsoluteX, 3, 4},
        {0x7E, Instruction::ROR, "ROR", AddressingMode::AbsoluteX, 3, 7},
        INVALID_OPCODE(0x7F),
        INVALID_OPCODE(0x80),
        {0x81, Instruction::STA, "STA", AddressingMode::IndirectX, 2, 6},
        INVALID_OPCODE(0x82),
        INVALID_OPCODE(0x83),
        {0x84, Instruction::STY, "STY", AddressingMode::ZeroPage, 2, 3},
        {0x85, Instruction::STA, "STA", AddressingMode::ZeroPage, 2, 3},
        {0x86, Instruction::STX, "STX", AddressingMode::ZeroPage, 2, 3},
        INVALID_OPCODE(0x87),
        {0x88, Instruction::DEY, "DEY", AddressingMode::Implied, 1, 2},
        INVALID_OPCODE(0x89),
        {0x8A, Instruction::TXA, "TXA", AddressingMode::Implied, 1, 2},
        INVALID_OPCODE(0x8B),
        {0x8C, Instruction::STY, "STY", AddressingMode::Absolute, 3, 4},
        {0x8D, Instruction::STA, "STA", AddressingMode::Absolute, 3, 4},
        {0x8E, Instruction::STX, "STX", AddressingMode::Absolute, 3, 4},
        INVALID_OPCODE(0x8F),
        {0x90, Instruction::BCC, "BCC", AddressingMode::Relative, 2, 2},
        {0x91, Instruction::STA, "STA", AddressingMode::IndirectY, 2, 6},
        INVALID_OPCODE(0x92),
        INVALID_OPCODE(0x93),
        {0x94, Instruction::STY, "STY", AddressingMode::ZeroPageX, 2, 4},
        {0x95, Instruction::STA, "STA", AddressingMode::ZeroPageX, 2, 4},
        {0x96, Instruction::STX, "STX", AddressingMode::ZeroPageY, 2, 4},
        INVALID_OPCODE(0x97),
        {0x98, Instruction::TYA, "TYA", AddressingMode::Implied, 1, 2},
        {0x99, Instruction::STA, "STA", AddressingMode::AbsoluteY, 3, 5},
        {0x9A, Instruction::TXS, "TXS", AddressingMode::Implied, 1, 2},
        INVALID_OPCODE(0x9B),
        INVALID_OPCODE(0x9C),
        {0x9D, Instruction::STA, "STA", AddressingMode::AbsoluteX, 3, 5},
        INVALID_OPCODE(0x9E),
        INVALID_OPCODE(0x9F),
        {0xA0, Instruction::LDY, "LDY", AddressingMode::Immediate, 2, 2},
        {0xA1, Instruction::LDA, "LDA", AddressingMode::IndirectX, 2, 6},
        {0xA2, Instruction::LDX, "LDX", AddressingMode::Immediate, 2, 2},
        INVALID_OPCODE(0xA3),
        {0xA4, Instruction::LDY, "LDY", AddressingMode::ZeroPage, 2, 3},
        {0xA5, Instruction::LDA, "LDA", AddressingMode::ZeroPage, 2, 3},
        {0xA6, Instruction::LDX, "LDX", AddressingMode::ZeroPage, 2, 3},
        INVALID_OPCODE(0xA7),
        {0xA8, Instruction::TAY, "TAY", AddressingMode::Implied, 1, 2},
        {0xA9, Instruction::LDA, "LDA", AddressingMode::Immediate, 2, 2},
        {0xAA, Instruction::TAX, "TAX", AddressingMode::Implied, 1, 2},
        INVALID_OPCODE(0xAB),
        {0xAC, Instruction::LDY, "LDY", AddressingMode::Absolute, 3, 4},
        {0xAD, Instruction::LDA, "LDA", AddressingMode::Absolute, 3, 4},
        {0xAE, Instruction::LDX, "LDX", AddressingMode::Absolute, 3, 4},
        INVALID_OPCODE(0xAF),
        {0xB0, Instruction::BCS, "BCS", AddressingMode::Relative, 2, 2},
        {0xB1, Instruction::LDA, "LDA", AddressingMode::IndirectY, 2, 5},
        INVALID_OPCODE(0xB2),
        INVALID_OPCODE(0xB3),
        {0xB4, Instruction::LDY, "LDY", AddressingMode::ZeroPageX, 2, 4},
        {0xB5, Instruction::LDA, "LDA", AddressingMode::ZeroPageX, 2, 4},
        {0xB6, Instruction::LDX, "LDX", AddressingMode::ZeroPageY, 2, 4},
        INVALID_OPCODE(0xB7),
        {0xB8, Instruction::CLV, "CLV", AddressingMode::Implied, 1, 2},
        {0xB9, Instruction::LDA, "LDA", AddressingMode::AbsoluteY, 3, 4},
        {0xBA, Instruction::TSX, "TSX", AddressingMode::Implied, 1, 2},
        INVALID_OPCODE(0xBB),
        {0xBC, Instruction::LDY, "LDY", AddressingMode::AbsoluteX, 3, 4},
        {0xBD, Instruction::LDA, "LDA", AddressingMode::AbsoluteX, 3, 4},
        {0xBE, Instruction::LDX, "LDX", AddressingMode::AbsoluteY, 3, 4},
        INVALID_OPCODE(0xBF),
        {0xC0, Instruction::CPY, "CPY", AddressingMode::Immediate, 2, 2},
        {0xC1, Instruction::CMP, "CMP", AddressingMode::IndirectX, 2, 6},
        INVALID_OPCODE(0xC2),
        INVALID_OPCODE(0xC3),
        {0xC4, Instruction::CPY, "CPY", AddressingMode::ZeroPage, 2, 3},
        {0xC5, Instruction::CMP, "CMP", AddressingMode::ZeroPage, 2, 3},
        {0xC6, Instruction::DEC, "DEC", AddressingMode::ZeroPage, 2, 5},
        INVALID_OPCODE(0xC7),
        {0xC8, Instruction::INY, "INY", AddressingMode::Implied, 1, 2},
        {0xC9, Instruction::CMP, "CMP", AddressingMode::Immediate, 2, 2},
        {0xCA, Instruction::DEX, "DEX", AddressingMode::Implied, 1, 2},
        INVALID_OPCODE(0xCB),
        {0xCC, Instruction::CPY, "CPY", AddressingMode::Absolute, 3, 4},
        {0xCD, Instruction::CMP, "CMP", AddressingMode::Absolute, 3, 4},
        {0xCE, Instruction::DEC, "DEC", AddressingMode::Absolute, 3, 6},
        INVALID_OPCODE(0xCF),
        {0xD0, Instruction::BNE, "BNE", AddressingMode::Relative, 2, 2},
        {0xD1, Instruction::CMP, "CMP", AddressingMode::IndirectY, 2, 5},
        INVALID_OPCODE(0xD2),
        INVALID_OPCODE(0xD3),
        INVALID_OPCODE(0xD4),
        {0xD5, Instruction::CMP, "CMP", AddressingMode::ZeroPageX, 2, 4},
        {0xD6, Instruction::DEC, "DEC", AddressingMode::ZeroPageX, 2, 6},
        INVALID_OPCODE(0xD7),
        {0xD8, Instruction::CLD, "CLD", AddressingMode::Implied, 1, 2},
        {0xD9, Instruction::CMP, "CMP", AddressingMode::AbsoluteY, 3, 4},
        INVALID_OPCODE(0xDA),
        INVALID_OPCODE(0xDB),
        INVALID_OPCODE(0xDC),
        {0xDD, Instruction::CMP, "CMP", AddressingMode::AbsoluteX, 3, 4},
        {0xDE, Instruction::DEC, "DEC", AddressingMode::AbsoluteX, 3, 7},
        INVALID_OPCODE(0xDF),
        {0xE0, Instruction::CPX, "CPX", AddressingMode::Immediate, 2, 2},
        {0xE1, Instruction::SBC, "SBC", AddressingMode::IndirectX, 2, 6},
        INVALID_OPCODE(0xE2),
        INVALID_OPCODE(0xE3),
        {0xE4, Instruction::CPX, "CPX", AddressingMode::ZeroPage, 2, 3},
        {0xE5, Instruction::SBC, "SBC", AddressingMode::ZeroPage, 2, 3},
        {0xE6, Instruction::INC, "INC", AddressingMode::ZeroPage, 2, 5},
        INVALID_OPCODE(0xE7),
        {0xE8, Instruction::INX, "INX", AddressingMode::Implied, 1, 2},
        {0xE9, Instruction::SBC, "SBC", AddressingMode::Immediate, 2, 2},
        {0xEA, Instruction::NOP, "NOP", AddressingMode::Implied, 1, 2},
        INVALID_OPCODE(0xEB),
        {0xEC, Instruction::CPX, "CPX", AddressingMode::Absolute, 3, 4},
        {0xED, Instruction::SBC, "SBC", AddressingMode::Absolute, 3, 4},
        {0xEE, Instruction::INC, "INC", AddressingMode::Absolute, 3, 6},
        INVALID_OPCODE(0xEF),
        {0xF0, Instruction::BEQ, "BEQ", AddressingMode::Relative, 2, 2},
        {0xF1, Instruction::SBC, "SBC", AddressingMode::IndirectY, 2, 5},
        INVALID_OPCODE(0xF2),
        INVALID_OPCODE(0xF3),
        INVALID_OPCODE(0xF4),
        {0xF5, Instruction::SBC, "SBC", AddressingMode::ZeroPageX, 2, 4},
        {0xF6, Instruction::INC, "INC", AddressingMode::ZeroPageX, 2, 6},
        INVALID_OPCODE(0xF7),
        {0xF8, Instruction::SED, "SED", AddressingMode::Implied, 1, 2},
        {0xF9, Instruction::SBC, "SBC", AddressingMode::AbsoluteY, 3, 4},
        INVALID_OPCODE(0xFA),
        INVALID_OPCODE(0xFB),
        INVALID_OPCODE(0xFC),
        {0xFD, Instruction::SBC, "SBC", AddressingMode::AbsoluteX, 3, 4},
        {0xFE, Instruction::INC, "INC", AddressingMode::AbsoluteX, 3, 7},
        INVALID_OPCODE(0xFF),
    }};

CPU6502::CPU6502(Memory &memory, bool debug, const std::vector<uint16_t> &breakpoints)
    : _memory(memory), _debug_mode{debug}, _breakpoints(breakpoints)
{
    reset();
}

// Disassemble current instruction
std::ostream &operator<<(std::ostream &os, const CPU6502 &cpu)
{
    const uint16_t pc = cpu._registers.pc();
    const uint8_t opcode = cpu._memory.get_byte_at(pc);

    const OpcodeInfo &opcode_info = OpcodeInfoTable.at(opcode);
    if (opcode_info.instruction == Instruction::Invalid)
    {
        fail_fast("Invalid opcode");
    }

    os
        << std::hex << std::nouppercase << std::setfill('0') << std::setw(4) << pc << ": "
        << std::hex << std::nouppercase << std::setfill('0') << std::setw(2) << static_cast<int>(opcode);

    if (opcode_info.bytes == 1)
    {
        os
            << "          "
            << opcode_info.opcode_string
            << "        ";
    }
    else if (opcode_info.bytes == 2)
    {
        uint8_t data;
        uint8_t value;

        data = cpu._memory.get_byte_at(pc + 1);
        value = data;

        os
            << " "
            << std::hex << std::nouppercase << std::setfill('0') << std::setw(2) << static_cast<int>(data)
            << "       "
            << opcode_info.opcode_string
            << " "
            << opcode_info.operand_to_string(value, pc + 2);
    }
    else if (opcode_info.bytes == 3)
    {
        uint8_t data1, data2;
        uint16_t value;

        data1 = cpu._memory.get_byte_at(pc + 1);
        value = data1;

        data2 = cpu._memory.get_byte_at(pc + 2);
        value |= static_cast<uint16_t>(data2 << 8);

        os
            << " "
            << std::hex << std::nouppercase << std::setfill('0') << std::setw(2) << static_cast<int>(data1)
            << " "
            << std::hex << std::nouppercase << std::setfill('0') << std::setw(2) << static_cast<int>(data2)
            << "    "
            << opcode_info.opcode_string
            << " "
            << opcode_info.operand_to_string(value, pc + 3);
    }
    else
    {
        fail_fast("Invalid number of opcode bytes");
    }

    return os;
}

void CPU6502::reset()
{
    _registers.reset();

    _registers.set_psr_I();

    // Set PC to address of power on reset location
    _registers.set_pc(_memory.get_power_on_reset_address());

    _cycle = 6;
}

void CPU6502::execute_instruction()
{
    if (_debug_mode)
    {
        std::cout << _registers << "  " << *this << std::endl;
    }

    // Get info about this opcode.

    const uint8_t opcode = _memory.get_byte_at(_registers.pc());

    const OpcodeInfo &opcode_info = OpcodeInfoTable.at(opcode);
    if (opcode_info.instruction == Instruction::Invalid)
    {
        fail_fast("Invalid opcode");
    }

    const uint16_t instruction_pc{_registers.pc()};

    // Check if breakpoint hit.
    if (std::find(_breakpoints.begin(), _breakpoints.end(), _registers.pc()) != _breakpoints.end())
    {
        fail_fast("Breakpoint hit");
    }

    if (_debug_mode)
    {
        if (_registers.pc() == _memory.get_NMI_handler_address())
        {
            // Dump info about current NMI
            std::cout
                << "[NMI] "
                << " NmiCounter: " << std::hex << std::nouppercase << std::setfill('0') << std::setw(2) << static_cast<int>(_memory.get_byte_at(0x5e))
                << " FrameCounter: " << std::hex << std::nouppercase << std::setfill('0') << std::setw(2) << static_cast<int>(_memory.get_byte_at(0x5b))
                << std::endl;
        }
    }

    // Move PC past this instruction.
    _registers.set_pc(instruction_pc + opcode_info.bytes);

    // Process instruction based on opcode addressing mode.

    switch (opcode_info.addressing_mode)
    {
    case AddressingMode::Accumulator:
        process_addressing_mode_accumulator(opcode_info.instruction);
        break;

    case AddressingMode::Implied:
        process_addressing_mode_implied(opcode_info.instruction);
        break;

    case AddressingMode::Relative:
        if (process_addressing_mode_relative(opcode_info.instruction))
        {
            const uint8_t value{_memory.get_byte_at(instruction_pc + 1)};

            // Can be a negative offset.
            // Note that PC has been moved past current instruction above.
            _registers.set_pc(_registers.pc() + static_cast<int8_t>(value));
        }
        break;

    case AddressingMode::Immediate:
    {
        const uint8_t value{_memory.get_byte_at(instruction_pc + 1)};
        process_addressing_mode_immediate(opcode_info.instruction, value);
    }
    break;

    case AddressingMode::Absolute:
    {
        const uint16_t value{_memory.get_word_at(instruction_pc + 1)};
        process_addressing_mode_absolute(opcode_info.instruction, value);
    }
    break;

    // 2 byte
    case AddressingMode::ZeroPage:
    case AddressingMode::ZeroPageX:
    case AddressingMode::ZeroPageY:
    case AddressingMode::IndirectX:
    case AddressingMode::IndirectY:
    {
        const uint8_t value{_memory.get_byte_at(instruction_pc + 1)};
        process_addressing_mode_byte(value, opcode_info.instruction, opcode_info.addressing_mode);
    }
    break;

    // 3 byte
    case AddressingMode::AbsoluteX:
    case AddressingMode::AbsoluteY:
    case AddressingMode::Indirect:
    {
        const uint16_t value{_memory.get_word_at(instruction_pc + 1)};
        process_addressing_mode_word(value, opcode_info.instruction, opcode_info.addressing_mode);
    }
    break;

    default:
        fail_fast("Invalid opcode");
        break;
    }

    // Update cycle count

    _cycle += opcode_info.cycles;
    // TODO: Some instructions have alternate cycle counts, add them here.
    // Maybe set hi bit of .cycles to indicate special case?
}

// Process instructions that use implied addressing mode.
void CPU6502::process_addressing_mode_implied(Instruction instruction)
{
    switch (instruction)
    {
    case Instruction::CLC:
        _registers.set_psr_C(false);
        break;
    case Instruction::CLD:
        _registers.set_psr_D(false);
        break;
    case Instruction::CLV:
        _registers.set_psr_V(false);
        break;
    case Instruction::DEX:
        // X,Z,N = X-1
        _registers.set_x(_registers.x() - 1);
        _registers.set_psr_Z_if_zero(_registers.x());
        _registers.set_psr_N_if_negative(_registers.x());
        break;
    case Instruction::DEY:
        // Y,Z,N = Y-1
        _registers.set_y(_registers.y() - 1);
        _registers.set_psr_Z_if_zero(_registers.y());
        _registers.set_psr_N_if_negative(_registers.y());
        break;
    case Instruction::INX:
        // X,Z,N = X+1
        _registers.set_x(_registers.x() + 1);
        _registers.set_psr_Z_if_zero(_registers.x());
        _registers.set_psr_N_if_negative(_registers.x());
        break;
    case Instruction::INY:
        // Y,Z,N = Y+1
        _registers.set_y(_registers.y() + 1);
        _registers.set_psr_Z_if_zero(_registers.y());
        _registers.set_psr_N_if_negative(_registers.y());
        break;
    case Instruction::PHA:
        push_stack_byte(_registers.a());
        break;
    case Instruction::PHP:
        push_stack_byte(_registers.p());
        break;
    case Instruction::PLA:
        _registers.set_a(pop_stack_byte());
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::PLP:
        _registers.set_p(pop_stack_byte());
        break;
    case Instruction::RTI:
        _registers.set_p(pop_stack_byte());
        _registers.set_pc(pop_stack_word());

        if (_debug_mode)
        {
            std::cout << "[RTI] "
                      << "PC: " << std::hex << std::nouppercase << std::setfill('0') << std::setw(4) << static_cast<int>(_registers.pc())
                      << " P: " << std::hex << std::nouppercase << std::setfill('0') << std::setw(2) << static_cast<int>(_registers.p())
                      << std::endl;
        }
        break;
    case Instruction::RTS:
        // Pulls the program counter (minus one) from the stack.
        _registers.set_pc(pop_stack_word() + 1);
        break;
    case Instruction::SEC:
        // C = 1
        _registers.set_psr_C(true);
        break;
    case Instruction::SED:
        // D = 1
        _registers.set_psr_D(true);
        break;
    case Instruction::TAX:
        // X = A
        _registers.set_x(_registers.a());
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::TAY:
        // Y = A
        _registers.set_y(_registers.a());
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::TXA:
        // A = X
        _registers.set_a(_registers.x());
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::TXS:
        // S = X
        _registers.set_sp(_registers.x());
        break;
    case Instruction::TYA:
        // A = Y
        _registers.set_a(_registers.y());
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    default:
        fail_fast("Invalid instruction");
        break;
    }
}

// Process instructions that accumulator addressing mode.
void CPU6502::process_addressing_mode_accumulator(Instruction instruction)
{
    uint8_t M = _registers.a();

    switch (instruction)
    {
    case Instruction::ASL:
        // A,Z,C,N = M*2

        // Set to contents of old bit 7.
        _registers.set_psr_C(IS_BIT_SET(M, 7));

        _registers.set_a(M << 1);

        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::LSR:
        // A,C,Z,N = A/2

        // Set to contents of old bit 0
        _registers.set_psr_C(IS_BIT_SET(M, 0));

        _registers.set_a(M >> 1);

        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::ROL:
    {
        const bool previous_C = _registers.psr_C();

        // Set to contents of old bit 7
        _registers.set_psr_C(IS_BIT_SET(M, 7));

        _registers.set_a((M << 1) | static_cast<uint8_t>(previous_C));

        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());

        _registers.set_a(M);
    }
    break;
    case Instruction::ROR:
    {
        const uint8_t previous_C = _registers.psr_C() << 7;

        // Set to contents of old bit 0
        _registers.set_psr_C(IS_BIT_SET(M, 0));

        _registers.set_a((M >> 1) | previous_C);

        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
    }
    break;
    default:
        fail_fast("Invalid instruction");
        break;
    }
}

// Process instructions that use relative addressing mode. Returns true if branch should be taken.
bool CPU6502::process_addressing_mode_relative(Instruction instruction)
{
    bool do_branch = false;

    switch (instruction)
    {
    case Instruction::BCC:
        do_branch = !_registers.psr_C();
        break;
    case Instruction::BCS:
        do_branch = _registers.psr_C();
        break;
    case Instruction::BEQ:
        do_branch = _registers.psr_Z();
        break;
    case Instruction::BMI:
        do_branch = _registers.psr_N();
        break;
    case Instruction::BNE:
        do_branch = !_registers.psr_Z();
        break;
    case Instruction::BPL:
        do_branch = !_registers.psr_N();
        break;
    case Instruction::BVC:
        do_branch = !_registers.psr_V();
        break;
    case Instruction::BVS:
        do_branch = _registers.psr_V();
        break;
    default:
        fail_fast("Invalid instruction");
        break;
    }

    return do_branch;
}

// Operand is the data, e.g. LDA #30 loads 30 into A.
void CPU6502::process_addressing_mode_immediate(Instruction instruction, uint8_t M)
{
    switch (instruction)
    {
    case Instruction::ADC:
        // A,Z,C,N = A+M+C

        // NOTE: Normally decimal mode is disabled and ADC/SBC perform
        // simple binary arithmetic (e.g. $99 + $01 => $9A Carry = 0),
        // but if the flag is set with a SED instruction the processor will perform
        // binary coded decimal arithmetic instead (e.g. $99 + $01 => $00 Carry = 1).

        if (_registers.psr_D())
        {
            fail_fast("Decimal mode not implemented");
        }
        else
        {
            // uint16 to allow for carry
            const uint16_t sum = _registers.a() + M + _registers.psr_C();

            _registers.set_psr_C(HIBYTE(sum) != 0);

            const uint8_t sum8 = LOBYTE(sum);

            _registers.set_a(sum8);

            _registers.set_psr_N_if_negative(_registers.a());
            _registers.set_psr_Z_if_zero(_registers.a());

            // The V bit is set to represent overflow if the
            // result exceeds +127 or -128 in two’s complement binary.
            const bool set_v =
                !IS_BIT_SET(_registers.a() ^ M, 7) &&
                IS_BIT_SET(_registers.a() ^ sum8, 7);
            _registers.set_psr_V(set_v);
        }
        break;
    case Instruction::AND:
        _registers.set_a(_registers.a() & M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::CMP:
        // Z,C,N = A-M
        _registers.set_psr_C(_registers.a() >= M);
        _registers.set_psr_Z(_registers.a() == M);
        _registers.set_psr_N_if_negative(_registers.a() - M);
        break;
    case Instruction::CPX:
        // Z,C,N = X-M
        _registers.set_psr_C(_registers.x() >= M);
        _registers.set_psr_Z(_registers.x() == M);
        _registers.set_psr_N_if_negative(_registers.x() - M);
        break;
    case Instruction::CPY:
        // Z,C,N = Y-M
        _registers.set_psr_C(_registers.y() >= M);
        _registers.set_psr_Z(_registers.y() == M);
        _registers.set_psr_N_if_negative(_registers.y() - M);
        break;
    case Instruction::EOR:
        _registers.set_a(_registers.a() ^ M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::LDA:
        _registers.set_a(M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());

        _registers.set_a(M);
        break;
    case Instruction::LDX:
        _registers.set_x(M);
        _registers.set_psr_Z_if_zero(_registers.x());
        _registers.set_psr_N_if_negative(_registers.x());
        break;
    case Instruction::LDY:
        _registers.set_y(M);
        _registers.set_psr_Z_if_zero(_registers.y());
        _registers.set_psr_N_if_negative(_registers.y());
        break;
    case Instruction::ORA:
        _registers.set_a(_registers.a() | M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::SBC:
        // A,Z,C,N = A-M-(1-C)
        // NOTE: Normally decimal mode is disabled and ADC/SBC perform
        // simple binary arithmetic (e.g. $99 + $01 => $9A Carry = 0),
        // but if the flag is set with a SED instruction the processor will perform
        // binary coded decimal arithmetic instead (e.g. $99 + $01 => $00 Carry = 1).

        if (_registers.psr_D())
        {
            fail_fast("Decimal mode not implemented");
        }
        else
        {
            // NOTE: signed int
            const int16_t diff = _registers.a() - M - (1 - _registers.psr_C());
            const uint8_t diff8 = LOBYTE(diff);

            // The V bit is set automatically to represent overflow if the
            // result exceeds (+127) or (-128) in two’s complement binary.
            _registers.set_psr_V(diff > 127 || diff < -128);
            _registers.set_psr_C(diff >= 0);

            _registers.set_a(diff8);

            _registers.set_psr_Z_if_zero(_registers.a());
            _registers.set_psr_N_if_negative(_registers.a());
        }
        break;
    default:
        fail_fast("Invalid instruction");
        break;
    }
}

void CPU6502::set_resolved_byte(uint8_t addr, AddressingMode addressing_mode, uint8_t value)
{
    switch (addressing_mode)
    {
    case AddressingMode::ZeroPage:
        _memory.set_byte_at(addr, value);
        break;
    case AddressingMode::ZeroPageX:
        // Note: Address wrap around
        _memory.set_byte_at((addr + _registers.x()) & 0xff, value);
        break;
    case AddressingMode::ZeroPageY:
        __debugbreak(); // TODO: unused?
        _memory.set_byte_at(addr + _registers.y(), value);
        break;
    case AddressingMode::IndirectX:
    {
        __debugbreak(); // TODO: unused?
        // Note: Address wrap around
        const uint16_t new_addr{_memory.get_word_at((addr + _registers.x()) & 0xFF)};
        _memory.set_byte_at(new_addr, value);
    }
    break;
    case AddressingMode::IndirectY:
    {
        // No Zero Page wrap around
        const uint16_t resolved_addr = _memory.get_word_at(addr) + _registers.y();

        // Value will loaded with the contents of memory at $4038.
        _memory.set_byte_at(resolved_addr, value);
    }
    break;
    default:
        fail_fast("Invalid addressing mode");
        break;
    }
}

// Operand is an address anywhere in 64K memory. LDA 3056 loads contents of address 3056 into A.
void CPU6502::process_addressing_mode_absolute(Instruction instruction, uint16_t addr)
{
    // NOTE: Not needed for JMP,JSR,STA,STX,STY
    uint8_t M = _memory.get_byte_at(addr);

    switch (instruction)
    {
    case Instruction::AND:
        // A,Z,N = A&M
        _registers.set_a(_registers.a() & M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::ASL:
        // M,Z,C,N = M*2

        // Set to contents of old bit 7.
        _registers.set_psr_C(IS_BIT_SET(M, 7));

        M <<= 1;

        _registers.set_psr_Z_if_zero(M);
        _registers.set_psr_N_if_negative(M);

        _memory.set_byte_at(addr, M);
        break;
    case Instruction::BIT:
        // A & M, N = M7, V = M6
        _registers.set_psr_Z((_registers.a() & M) == 0);
        _registers.set_psr_V(IS_BIT_SET(M, 6));
        _registers.set_psr_N_if_negative(M);
        break;
    case Instruction::CMP:
        // Z,C,N = A-M
        _registers.set_psr_C(_registers.a() >= M);
        _registers.set_psr_Z(_registers.a() == M);
        _registers.set_psr_N_if_negative(_registers.a() - M);
        break;
    case Instruction::DEC:
        // M,Z,N = M-1
        M -= -1;
        _registers.set_psr_Z_if_zero(M);
        _registers.set_psr_N_if_negative(M);

        _memory.set_byte_at(addr, M);
        break;
    case Instruction::INC:
        // M,Z,N = M+1
        M += 1;
        _registers.set_psr_Z(M);
        _registers.set_psr_N_if_negative(M);

        _memory.set_byte_at(addr, M);
        break;
    case Instruction::JMP:
        if (LOBYTE(addr) == 0xFF)
        {
            // An original 6502 has does not correctly fetch the target address if the indirect vector falls on a page
            // boundary (e.g. $xxFF where xx is any value from $00 to $FF).
            // In this case fetches the LSB from $xxFF as expected but takes the MSB from $xx00.
            fail_fast("JMP bug hit");
        }
        _registers.set_pc(addr);
        break;
    case Instruction::JSR:
        // Push the address (minus one) of the return point on to the stack.
        push_stack_word(_registers.pc() - 1);
        _registers.set_pc(addr);
        break;
    case Instruction::LDA:
        // A,Z,N = M
        _registers.set_a(M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::LDX:
        // X,Z,N = M
        _registers.set_x(M);
        _registers.set_psr_Z_if_zero(_registers.x());
        _registers.set_psr_N_if_negative(_registers.x());
        break;
    case Instruction::LDY:
        // Y,Z,N = M
        _registers.set_y(M);
        _registers.set_psr_Z_if_zero(_registers.y());
        _registers.set_psr_N_if_negative(_registers.y());
        break;
    case Instruction::LSR:
        // M,C,Z,N = M/2
        // Set to contents of old bit 0
        _registers.set_psr_C(IS_BIT_SET(M, 0));

        M >>= 1;

        _registers.set_psr_Z_if_zero(M);
        _registers.set_psr_N_if_negative(M);

        _memory.set_byte_at(addr, M);
        break;
    case Instruction::ORA:
        // A,Z,N = A|M
        _registers.set_a(_registers.a() | M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::ROR:
    {
        const uint8_t previous_C = _registers.psr_C() << 7;

        // Set to contents of old bit 0
        _registers.set_psr_C(IS_BIT_SET(M, 0));

        _registers.set_a((M >> 1) | previous_C);

        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
    }
    break;
    case Instruction::STA:
        // M = A
        _memory.set_byte_at(addr, _registers.a());
        break;
    case Instruction::STX:
        // M = X
        _memory.set_byte_at(addr, _registers.x());
        break;
    case Instruction::STY:
        // M = Y
        _memory.set_byte_at(addr, _registers.y());
        break;
    default:
        fail_fast("Invalid instruction");
        break;
    }
}

uint8_t CPU6502::get_resolved_byte(uint8_t value, AddressingMode addressing_mode)
{
    switch (addressing_mode)
    {
    case AddressingMode::ZeroPage:
        // LDA $00 -> return *$00
        value = _memory.get_byte_at(value);
        break;
    case AddressingMode::ZeroPageX:
        // AND $10,X -> return *($10 + X)
        // Note: Address wrap around
        value = _memory.get_byte_at((value + _registers.x()) & 0xff);
        break;
    case AddressingMode::ZeroPageY:
        // LDX $10,Y -> return *($10 + Y)
        value = _memory.get_byte_at(value + _registers.y());
        break;
    case AddressingMode::IndirectX:
    {
        // Indexed Indirect

        // Zero Page wrap around
        const uint16_t resolved_addr{_memory.get_word_at((value + _registers.x()) & 0xFF)};
        value = _memory.get_byte_at(resolved_addr);
    }
    break;
    case AddressingMode::IndirectY:
    {
        // Indirect Indexed

        // No Zero Page wrap around
        const uint16_t resolved_addr = _memory.get_word_at(value) + _registers.y();

        // Value will loaded with the contents of memory at $4038.
        value = _memory.get_byte_at(resolved_addr);
    }
    break;
    default:
        fail_fast("Invalid addressing mode");
        break;
    }

    return value;
}

// Process instructions that use byte addressing mode, aside from Immediate.
// AddressingMode::ZeroPage
// Operand is an address in page 0.  LDA 30 loads contents of address 30 into A.

// AddressingMode::ZeroPageX
// AddressingMode::ZeroPageY
// AddressingMode::IndirectX
// AddressingMode::IndirectY
void CPU6502::process_addressing_mode_byte(uint8_t operand_value, Instruction instruction, AddressingMode addressing_mode)
{
    // Value at resolved address. Returns get_byte_at(...)
    // Use set_resolved_byte(operand_value, addressing_mode, v) to write.
    uint8_t M{get_resolved_byte(operand_value, addressing_mode)};

    switch (instruction)
    {
    case Instruction::AND:
        // A,Z,N = A&M
        _registers.set_a(_registers.a() & M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::ADC:
        // A,Z,C,N = A+M+C

        // NOTE: Normally decimal mode is disabled and ADC/SBC perform
        // simple binary arithmetic (e.g. $99 + $01 => $9A Carry = 0),
        // but if the flag is set with a SED instruction the processor will perform
        // binary coded decimal arithmetic instead (e.g. $99 + $01 => $00 Carry = 1).

        if (_registers.psr_D())
        {
            fail_fast("Decimal mode not implemented");
        }
        else
        {
            const uint16_t sum = _registers.a() + M + _registers.psr_C();

            _registers.set_psr_C(HIBYTE(sum) != 0);

            const uint8_t sum8 = LOBYTE(sum);

            _registers.set_a(sum8);

            _registers.set_psr_N_if_negative(_registers.a());
            _registers.set_psr_Z_if_zero(_registers.a());

            // The V bit is set to represent overflow if the
            // result exceeds +127 or -128 in two’s complement binary.
            const bool set_v =
                !IS_BIT_SET(_registers.a() ^ M, 7) &&
                IS_BIT_SET(_registers.a() ^ sum8, 7);
            _registers.set_psr_V(set_v);
        }
        break;
    case Instruction::ASL:
        // M,Z,C,N = M*2

        // Set to contents of old bit 7.
        _registers.set_psr_C(IS_BIT_SET(M, 7));

        M <<= 1;

        _registers.set_psr_Z_if_zero(M);
        _registers.set_psr_N_if_negative(M);

        set_resolved_byte(operand_value, addressing_mode, M);
        break;
    case Instruction::BIT:
        // A & M, N = M7, V = M6
        _registers.set_psr_Z_if_zero(_registers.a() & M);
        _registers.set_psr_V(IS_BIT_SET(M, 6));
        _registers.set_psr_N_if_negative(M);
        break;
    case Instruction::CMP:
        // Z,C,N = A-M
        _registers.set_psr_C(_registers.a() >= M);
        _registers.set_psr_Z(_registers.a() == M);
        _registers.set_psr_N_if_negative(_registers.a() - M);
        break;
    case Instruction::DEC:
        // M,Z,N = M-1
        M -= 1;
        _registers.set_psr_Z_if_zero(M);
        _registers.set_psr_N_if_negative(M);

        set_resolved_byte(operand_value, addressing_mode, M);
        break;
    case Instruction::EOR:
        // A,Z,N = A^M
        _registers.set_a(_registers.a() ^ M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::INC:
        // M,Z,N = M+1
        M += 1;
        _registers.set_psr_Z_if_zero(M);
        _registers.set_psr_N_if_negative(M);

        set_resolved_byte(operand_value, addressing_mode, M);
        break;
    case Instruction::LDA:
        // A,Z,N = M
        _registers.set_a(M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::LDX:
        // X,Z,N = M
        _registers.set_x(M);
        _registers.set_psr_Z_if_zero(_registers.x());
        _registers.set_psr_N_if_negative(_registers.x());
        break;
    case Instruction::LDY:
        // Y,Z,N = M
        _registers.set_y(M);
        _registers.set_psr_Z_if_zero(_registers.y());
        _registers.set_psr_N_if_negative(_registers.y());
        break;
    case Instruction::LSR:
        // M,C,Z,N = M/2

        // Set to contents of old bit 0
        _registers.set_psr_C(IS_BIT_SET(M, 0));

        M >>= 1;

        _registers.set_psr_Z_if_zero(M);
        _registers.set_psr_N_if_negative(M);

        set_resolved_byte(operand_value, addressing_mode, M);
        break;
    case Instruction::ORA:
        // A,Z,N = A|M
        _registers.set_a(_registers.a() | M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::ROL:
    {
        const bool previous_C = _registers.psr_C();

        // Set to contents of old bit 7
        _registers.set_psr_C(IS_BIT_SET(M, 7));

        _registers.set_a((M << 1) | static_cast<uint8_t>(previous_C));

        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
    }
    break;
    case Instruction::ROR:
    {
        const uint8_t previous_C = _registers.psr_C() << 7;

        // Set to contents of old bit 0
        _registers.set_psr_C(IS_BIT_SET(M, 0));

        _registers.set_a((M >> 1) | previous_C);

        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
    }
    break;
    case Instruction::SBC:
        // A,Z,C,N = A-M-(1-C)

        // NOTE: Normally decimal mode is disabled and ADC/SBC perform
        // simple binary arithmetic (e.g. $99 + $01 => $9A Carry = 0),
        // but if the flag is set with a SED instruction the processor will perform
        // binary coded decimal arithmetic instead (e.g. $99 + $01 => $00 Carry = 1).

        if (_registers.psr_D())
        {
            fail_fast("Decimal mode not implemented");
        }
        else
        {
            // NOTE: signed int
            const int16_t diff = _registers.a() - M - (1 - _registers.psr_C());
            const uint8_t diff8 = LOBYTE(diff);

            // The V bit is set automatically to represent overflow if the
            // result exceeds (+127) or (-128) in two’s complement binary.
            _registers.set_psr_V(diff > 127 || diff < -128);
            _registers.set_psr_C(diff >= 0);

            _registers.set_a(diff8);

            _registers.set_psr_Z_if_zero(_registers.a());
            _registers.set_psr_N_if_negative(_registers.a());
        }
        break;
    case Instruction::STA:
        // M = A
        set_resolved_byte(operand_value, addressing_mode, _registers.a());
        break;
    case Instruction::STX:
        // M = X
        set_resolved_byte(operand_value, addressing_mode, _registers.x());
        break;
    case Instruction::STY:
        // M = Y
        set_resolved_byte(operand_value, addressing_mode, _registers.x());
        break;
    default:
        fail_fast("Invalid instruction");
        break;
    }
}

// AddressingMode::AbsoluteX
// AddressingMode::AbsoluteY
// AddressingMode::Indirect
uint16_t CPU6502::resolve_address(uint16_t value, AddressingMode addressing_mode)
{
    switch (addressing_mode)
    {
    case AddressingMode::AbsoluteX:
        // LTA $3000,X -> return 3000 + X
        value += _registers.x();
        break;
    case AddressingMode::AbsoluteY:
        // AND $4000,Y -> return 4000 + Y
        value += _registers.y();
        break;
    case AddressingMode::Indirect:
        // JMP ($FFFC) -> return *$FFFC
        value = _memory.get_word_at(value);
        break;
    default:
        fail_fast("Invalid addressing mode");
        break;
    }

    return value;
}

// Process instructions that use word addressing mode, aside from Absolute
// AddressingMode::AbsoluteX
// AddressingMode::AbsoluteY
// AddressingMode::Indirect
void CPU6502::process_addressing_mode_word(uint16_t operand_value, Instruction instruction, AddressingMode addressing_mode)
{
    // Value at resolved address.
    const uint16_t addr{resolve_address(operand_value, addressing_mode)};

    // NOTE: Not needed for STA,STX,STY
    const uint8_t M = _memory.get_byte_at(addr);

    switch (instruction)
    {
    case Instruction::ADC:
        // A,Z,C,N = A+M+C

        // NOTE: Normally decimal mode is disabled and ADC/SBC perform
        // simple binary arithmetic (e.g. $99 + $01 => $9A Carry = 0),
        // but if the flag is set with a SED instruction the processor will perform
        // binary coded decimal arithmetic instead (e.g. $99 + $01 => $00 Carry = 1).

        if (_registers.psr_D())
        {
            fail_fast("Decimal mode not implemented");
        }
        else
        {
            const uint16_t sum = _registers.a() + M + _registers.psr_C();

            _registers.set_psr_C(HIBYTE(sum) != 0);

            const uint8_t sum8 = LOBYTE(sum);

            _registers.set_a(sum8);

            _registers.set_psr_N_if_negative(_registers.a());
            _registers.set_psr_Z_if_zero(_registers.a());

            // The V bit is set to represent overflow if the
            // result exceeds +127 or -128 in two’s complement binary.
            const bool set_v =
                !IS_BIT_SET(_registers.a() ^ M, 7) &&
                IS_BIT_SET(_registers.a() ^ sum8, 7);
            _registers.set_psr_V(set_v);
        }
        break;
    case Instruction::EOR:
        // A,Z,N = A^M
        _registers.set_a(_registers.a() ^ M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::LDA:
        // A,Z,N = M
        _registers.set_a(M);
        _registers.set_psr_Z_if_zero(_registers.a());
        _registers.set_psr_N_if_negative(_registers.a());
        break;
    case Instruction::LDX:
        // X,Z,N = M
        _registers.set_x(M);
        _registers.set_psr_Z_if_zero(_registers.x());
        _registers.set_psr_N_if_negative(_registers.x());
        break;
    case Instruction::SBC:
        // A,Z,C,N = A-M-(1-C)
        // NOTE: Normally decimal mode is disabled and ADC/SBC perform
        // simple binary arithmetic (e.g. $99 + $01 => $9A Carry = 0),
        // but if the flag is set with a SED instruction the processor will perform
        // binary coded decimal arithmetic instead (e.g. $99 + $01 => $00 Carry = 1).

        if (_registers.psr_D())
        {
            fail_fast("Decimal mode not implemented");
        }
        else
        {
            // NOTE: signed int
            const int16_t diff = _registers.a() - M - (1 - _registers.psr_C());
            const uint8_t diff8 = LOBYTE(diff);

            // The V bit is set automatically to represent overflow if the
            // result exceeds (+127) or (-128) in two’s complement binary.
            _registers.set_psr_V(diff > 127 || diff < -128);
            _registers.set_psr_C(diff >= 0);

            _registers.set_a(diff8);

            _registers.set_psr_Z_if_zero(_registers.a());
            _registers.set_psr_N_if_negative(_registers.a());
        }
        break;

    case Instruction::STA:
        _memory.set_byte_at(addr, _registers.a());
        break;
    default:
        fail_fast("Invalid instruction");
        break;
    }
}

void CPU6502::initiate_nmi()
{
    if (_debug_mode)
    {
        std::cout
            << "[NMI] "
            << "PC: " << std::hex << std::nouppercase << std::setfill('0') << std::setw(4) << _registers.pc()
            << "  P: " << std::hex << std::nouppercase << std::setfill('0') << std::setw(2) << static_cast<int>(_registers.p())
            << "  JMP: " << std::hex << std::nouppercase << std::setfill('0') << std::setw(4) << _memory.get_NMI_handler_address()
            << std::endl;
    }

    push_stack_word(_registers.pc());
    push_stack_byte(_registers.p());

    // Set PC to address of non-maskable interrupt (NMI) handler
    //
    //                    ;
    //                    ; NMI handler.  These arrive at 3000/12 = 250Hz.
    //                    ;
    // 7b65: 48           NMI             pha                             ;Push A, Y and X onto the stack

    _registers.set_pc(_memory.get_NMI_handler_address());

    _cycle += 7;
}

void CPU6502::push_stack_byte(uint8_t store)
{
    // stack at 0x100:0x1FF
    _memory.set_byte_at(0x100 + _registers.sp(), store);

    // Pushing bytes to the stack causes the stack pointer to be decremented.
    _registers.decrement_sp();
}

uint8_t CPU6502::pop_stack_byte()
{
    // Pulling bytes from the stack causes the stack pointer to be incremented.
    _registers.increment_sp();

    return _memory.get_byte_at(0x100 + _registers.sp());
}

void CPU6502::push_stack_word(uint16_t store)
{
    push_stack_byte(HIBYTE(store));
    push_stack_byte(LOBYTE(store));
}

uint16_t CPU6502::pop_stack_word()
{
    _registers.increment_sp();

    const uint16_t val = _memory.get_word_at(0x100 + _registers.sp());

    _registers.increment_sp();

    return val;
}
