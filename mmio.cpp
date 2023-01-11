#include "mmio.hpp"
#include "memory.hpp"

MMIO::MMIO(Memory &memory) : _memory(memory)
{
}

uint8_t MMIO::get_IN0(MMIO::IN0 addr) const
{
    return _memory.get_byte_at(addr);
}

void MMIO::set_IN0(MMIO::IN0 addr, uint8_t value)
{
    _memory.set_byte_at(addr, value);
}

void MMIO::set_IN1(MMIO::IN1 addr, uint8_t value)
{
    _memory.set_byte_at(addr, value);
}

void MMIO::set_DSW1(MMIO::DSW1 addr, uint8_t value)
{
    _memory.set_byte_at(addr, value);
}

uint8_t MMIO::get_OTHER(MMIO::OTHER addr) const
{
    return _memory.get_byte_at(addr);
}

void MMIO::set_OTHER(MMIO::OTHER addr, uint8_t value)
{
    _memory.set_byte_at(addr, value);
}
