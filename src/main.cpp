#include "bus/bus.hpp"
#include "mips/cpu.hpp"

#include <exception>
#include <iostream>

int main()
{
    try {
        Bus bus{"bios/SCPH1001.BIN"};
        Cpu cpu{bus};

        for (int i = 0; i < 10; ++i) {
            cpu.step();
        }
    }
    catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}