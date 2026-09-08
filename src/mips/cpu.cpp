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
}

void Cpu::step()
{
    const uint32_t instruction = bus_.read32(pc_);

    std::cout << std::hex
              << std::uppercase
              << std::setfill('0')
              << "PC=0x" << std::setw(8) << pc_
              << " INSTRUCTION=0x" << std::setw(8) << instruction
              << '\n';

    // Advance before execution to support MIPS branch delay slots later.
    pc_ = nextPc_;
    nextPc_ += 4;

    execute(instruction);

    // MIPS register $zero must always contain zero.
    registers_[0] = 0;
}

void Cpu::execute(uint32_t instruction)
{
    const uint32_t opcode = instruction >> 26;

    switch (opcode) {
        case 0x0F:
            executeLui(instruction);
            break;
        case 0x0D:
            executeOri(instruction);
            break;

        default:
            std::cerr << "Unsupported opcode: 0x"
                      << std::hex << std::uppercase
                      << opcode << '\n';
            break;
    }
}

void Cpu::executeLui(uint32_t instruction)
{
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t immediate = instruction & 0xFFFF;

    registers_[rt] = immediate << 16;

    std::cout << "  LUI r" << std::dec << rt
              << ", 0x" << std::hex << std::uppercase
              << immediate
              << " -> 0x" << registers_[rt]
              << '\n';
}

void Cpu::executeOri(uint32_t instruction)
{
    
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t immediate = instruction & 0xFFFF;

    registers_[rt] = registers_[rs] | immediate;


    std::cout << "  ORI r" << std::dec << rt
            << ", r" << rs
            << ", 0x" << std::hex << std::uppercase
            << immediate
            << " -> 0x" << registers_[rt]
            << '\n';

}


