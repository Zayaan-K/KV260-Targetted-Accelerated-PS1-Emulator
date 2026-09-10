#include "cpu.hpp"
#include "../bus/bus.hpp"
#include <iomanip>
#include <iostream>

Cpu::Cpu(Bus& bus)
    : bus_(bus)
{
    reset();
}

void Cpu::reset()
{
    registers_.fill(0);

    pc_ = 0xBFC00000;
    nextPc_ = pc_ + 4;

    hi_ = 0;
    lo_ = 0;

    pendingLoad_ = {};

}

void Cpu::step()
{
    const uint32_t instruction = bus_.read32(pc_);

    std::cout << std::hex
              << std::uppercase
              << std::setfill('0')
              << "PC=0x" << std::setw(8) << pc_
              << " INSTRUCTION=0x" << std::setw(8)
              << instruction << '\n';

    const PendingLoad previousLoad = pendingLoad_;
    pendingLoad_ = {};

    pc_ = nextPc_;
    nextPc_ += 4;

    execute(instruction);

    if (previousLoad.valid && previousLoad.destination != 0) {
        registers_[previousLoad.destination] = previousLoad.value;
    }

    registers_[0] = 0;
}