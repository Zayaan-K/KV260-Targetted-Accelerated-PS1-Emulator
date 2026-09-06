#include "bus/bus.hpp"
#include "mips/cpu.hpp"

#include <exception>
#include <iostream>

int main()
{
    try {
        Bus bus{"bios/SCPH1001.BIN"};
        Cpu cpu{bus};

        cpu.step();
    }
    catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
