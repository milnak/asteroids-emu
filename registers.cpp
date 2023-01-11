#include "registers.hpp"

#include <sstream>
#include <iomanip>

std::ostream &operator<<(std::ostream &os, const Registers &r)
{
    // https://stackoverflow.com/questions/28257957/c-stdcout-in-hexadecimal
    return os
           << "A:" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(r._a) << " "
           << "Y:" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(r._y) << " "
           << "X:" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(r._x) << " "
        //    << "PC: " << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << r._pc << " "
           << "SP: " << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(r._sp) << " "
           << r.p_to_string();
}

std::string Registers::p_to_string() const
{
    std::stringstream str;

    str << (psr_N() ? 'N' : '_')
        << (psr_V() ? 'O' : '_')
        << (psr_B() ? 'B' : '_')
        << (psr_D() ? 'D' : '_')
        << (psr_I() ? 'I' : '_')
        << (psr_Z() ? 'Z' : '_')
        << (psr_C() ? 'C' : '_');
    return str.str();
}
