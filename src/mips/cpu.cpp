#include "cpu.hpp"
#include "../bus/bus.hpp"
#include <iomanip>
#include <iostream>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

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
    const uint32_t opcode = (instruction >> 26) & 0x3F;

    switch (opcode)
    {

        case 0x00:
            executeSpecial(instruction);
            break;
        case 0x02:
            executeJ(instruction);
            break;
        case 0x03:
            executeJal(instruction);
            break;
        case 0x08:
            executeAddi(instruction);
            break;
        case 0x09:
            executeAddiu(instruction);
            break;
        case 0x0A:
            executeSlti(instruction);
            break;
        case 0x0B:
            executeSltiu(instruction);
            break;
        case 0x0C:
            executeAndi(instruction);
            break;
        case 0x0D:
            executeOri(instruction);
            break;
        case 0x0E:
            executeXori(instruction);
            break;
        case 0x0F:
            executeLui(instruction);
            break;

        default:
            std::cerr << "Unsupported opcode: 0x"
                      << std::hex
                      << std::uppercase
                      << opcode
                      << '\n';
            break;
    }
}


void Cpu::executeSpecial(uint32_t instruction)
{
    const uint32_t funct = instruction & 0x3F;

    switch (funct) {
        case 0x00: executeSll(instruction);     break;
        case 0x02: executeSrl(instruction);     break;
        case 0x03: executeSra(instruction);     break;
        case 0x04: executeSllv(instruction);    break;
        case 0x06: executeSrlv(instruction);    break;
        case 0x07: executeSrav(instruction);    break;

        case 0x08: executeJr(instruction);      break;
        case 0x09: executeJalr(instruction);    break;

        case 0x0C: executeSyscall(instruction); break;
        case 0x0D: executeBreak(instruction);   break;

        case 0x10: executeMfhi(instruction);    break;
        case 0x11: executeMthi(instruction);    break;
        case 0x12: executeMflo(instruction);    break;
        case 0x13: executeMtlo(instruction);    break;

        case 0x18: executeMult(instruction);    break;
        case 0x19: executeMultu(instruction);   break;
        case 0x1A: executeDiv(instruction);     break;
        case 0x1B: executeDivu(instruction);    break;

        case 0x20: executeAdd(instruction);     break;
        case 0x21: executeAddu(instruction);    break;
        case 0x22: executeSub(instruction);     break;
        case 0x23: executeSubu(instruction);    break;
        case 0x24: executeAnd(instruction);     break;
        case 0x25: executeOr(instruction);      break;
        case 0x26: executeXor(instruction);     break;
        case 0x27: executeNor(instruction);     break;
        case 0x2A: executeSlt(instruction);     break;
        case 0x2B: executeSltu(instruction);    break;

        default:
            std::cerr << "Unsupported SPECIAL function: 0x"
                      << std::hex << std::uppercase
                      << funct << '\n';
            break;
    }
}



//  31..26 |25..21|20..16|15..11|10..6 |  5..0  |
//   6bit  | 5bit | 5bit | 5bit | 5bit |  6bit  |
//  -------+------+------+------+------+--------+------------
// 000000 | rs   | rt   | rd   | N/A  | 10xxxx | alu-reg

void Cpu::executeAdd(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t a = registers_[rs];
    const uint32_t b = registers_[rt];
    const uint32_t result = a + b;

    const bool overflow =
        ((~(a ^ b) & (a ^ result)) & 0x80000000U) != 0;

    if (overflow) {
        throw std::overflow_error("MIPS ADD signed overflow");
    }

    registers_[rd] = result;


    std::cout << "  ADD r" << std::dec << rd
            << ", r" << rs
            << ", r" << rt
            << " -> 0x" << std::hex << std::uppercase
            << registers_[rd]
            << '\n';
}

void Cpu::executeSub(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t a = registers_[rs];
    const uint32_t b = registers_[rt];
    const uint32_t result = a - b;

    const bool overflow =
        (((a ^ b) & (a ^ result)) & 0x80000000U) != 0;

    if (overflow) {
        throw std::overflow_error("MIPS SUB signed overflow");
    }

    registers_[rd] = result;

    std::cout << "  SUB r" << std::dec << rd
            << ", r" << rs
            << ", r" << rt
            << " -> 0x" << std::hex << std::uppercase
            << registers_[rd]
            << '\n';
}


void Cpu::executeAddu (uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t a = registers_[rs];
    const uint32_t b = registers_[rt];
    const uint32_t result = a + b;

    registers_[rd] = result;

    std::cout << "  ADDU r" << std::dec << rd
            << ", r" << rs
            << ", r" << rt
            << " -> 0x" << std::hex << std::uppercase
            << registers_[rd]
            << '\n';

}

void Cpu::executeSubu(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t a = registers_[rs];
    const uint32_t b = registers_[rt];
    const uint32_t result = a - b;

    registers_[rd] = result;

    std::cout << "  SUBU r" << std::dec << rd
            << ", r" << rs
            << ", r" << rt
            << " -> 0x" << std::hex << std::uppercase
            << registers_[rd]
            << '\n';
    }


//31........26 25.....21 20.....16 15................0
//   opcode        rs        rt          immediate
//   001000       source destination    signed 16-bit

void Cpu::executeAddi (uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;


    const int32_t signedImmediate = static_cast<int16_t>(instruction & 0xFFFF);


    const uint32_t a = registers_[rs];
    const uint32_t b = static_cast<uint32_t>(signedImmediate);
    const uint32_t result = a + b;

    const bool overflow =
        ((~(a ^ b) & (a ^ result)) & 0x80000000U) != 0;

    if (overflow) {
        throw std::overflow_error("MIPS ADDI signed overflow");
    }

    registers_[rt] = result;

    std::cout << "  ADDI r" << std::dec << rt
              << ", r" << rs
              << ", " << signedImmediate
              << " -> 0x" << std::hex << std::uppercase
              << registers_[rt]
              << '\n';
}


void Cpu::executeAddiu(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const int32_t signedImmediate = static_cast<int16_t>(instruction & 0xFFFF);

    registers_[rt] = registers_[rs] + static_cast<uint32_t>(signedImmediate);

    std::cout << "  ADDIU r" << std::dec << rt
              << ", r" << rs
              << ", " << signedImmediate
              << " -> 0x" << std::hex << std::uppercase
              << registers_[rt]
              << '\n';
}


void Cpu::executeSlt(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const int32_t a = static_cast<int32_t>(registers_[rs]);
    const int32_t b = static_cast<int32_t>(registers_[rt]);

    registers_[rd] = (a < b) ? 1U : 0U;

    std::cout << "  SLT r" << std::dec << rd
              << ", r" << rs
              << ", r" << rt
              << " -> " << registers_[rd]
              << '\n';
}

void Cpu::executeSltu(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t a = registers_[rs];
    const uint32_t b = registers_[rt];

    registers_[rd] = (a < b) ? 1U : 0U;

    std::cout << "  SLTU r" << std::dec << rd
              << ", r" << rs
              << ", r" << rt
              << " -> " << registers_[rd]
              << '\n';
}

void Cpu::executeSlti(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;

    const int32_t immediate =
        static_cast<int16_t>(instruction & 0xFFFF);

    const int32_t source =
        static_cast<int32_t>(registers_[rs]);

    registers_[rt] = (source < immediate) ? 1U : 0U;

    std::cout << "  SLTI r" << std::dec << rt
              << ", r" << rs
              << ", " << immediate
              << " -> " << registers_[rt]
              << '\n';
}


void Cpu::executeSltiu(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;

    const int32_t signedImmediate =
        static_cast<int16_t>(instruction & 0xFFFF);

    const uint32_t immediate =
        static_cast<uint32_t>(signedImmediate);

    registers_[rt] =
        (registers_[rs] < immediate) ? 1U : 0U;

    std::cout << "  SLTIU r" << std::dec << rt
              << ", r" << rs
              << ", " << signedImmediate
              << " -> " << registers_[rt]
              << '\n';
}


void Cpu::executeAnd(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t result = registers_[rs] & registers_[rt];

    if (rd != 0)
        registers_[rd] = result;

    std::cout << "  AND r" << std::dec << rd
              << ", r" << rs
              << ", r" << rt
              << " -> 0x" << std::hex
              << std::uppercase << result << '\n';
}

void Cpu::executeOr(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t result = registers_[rs] | registers_[rt];

    if (rd != 0)
        registers_[rd] = result;

    std::cout << "  OR r" << std::dec << rd
              << ", r" << rs
              << ", r" << rt
              << " -> 0x" << std::hex
              << std::uppercase << result << '\n';
}

void Cpu::executeXor(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t result = registers_[rs] ^ registers_[rt];

    if (rd != 0)
        registers_[rd] = result;

    std::cout << "  XOR r" << std::dec << rd
              << ", r" << rs
              << ", r" << rt
              << " -> 0x" << std::hex
              << std::uppercase << result << '\n';
}

void Cpu::executeNor(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t result = ~(registers_[rs] | registers_[rt]);

    if (rd != 0)
        registers_[rd] = result;

    std::cout << "  NOR r" << std::dec << rd
              << ", r" << rs
              << ", r" << rt
              << " -> 0x" << std::hex
              << std::uppercase << result << '\n';
}

void Cpu::executeAndi(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t immediate = instruction & 0xFFFF;

    const uint32_t result = registers_[rs] & immediate;

    if (rt != 0)
        registers_[rt] = result;

    std::cout << "  ANDI r" << std::dec << rt
              << ", r" << rs
              << ", 0x" << std::hex
              << std::uppercase << immediate
              << " -> 0x" << result << '\n';
}


void Cpu::executeXori(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t immediate = instruction & 0xFFFF;

    const uint32_t result = registers_[rs] ^ immediate;

    if (rt != 0)
        registers_[rt] = result;

    std::cout << "  XORI r" << std::dec << rt
              << ", r" << rs
              << ", 0x" << std::hex
              << std::uppercase << immediate
              << " -> 0x" << result << '\n';
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


void Cpu::executeSll(uint32_t instruction)
{
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;
    const uint32_t shiftAmount = (instruction >> 6) & 0x1F;

    const uint32_t result = registers_[rt] << shiftAmount;

    if (rd != 0) registers_[rd] = result;

    std::cout << "  SLL r" << std::dec << rd
              << ", r" << rt
              << ", " << shiftAmount
              << " -> 0x" << std::hex
              << std::uppercase << result << '\n';
}

void Cpu::executeSrl(uint32_t instruction)
{
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;
    const uint32_t shiftAmount = (instruction >> 6) & 0x1F;

    const uint32_t result = registers_[rt] >> shiftAmount;

    if (rd != 0) registers_[rd] = result;

    std::cout << "  SRL r" << std::dec << rd
              << ", r" << rt
              << ", " << shiftAmount
              << " -> 0x" << std::hex
              << std::uppercase << result << '\n';
}

void Cpu::executeSra(uint32_t instruction)
{
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;
    const uint32_t shiftAmount = (instruction >> 6) & 0x1F;

    const int32_t signedValue = static_cast<int32_t>(registers_[rt]);

    const uint32_t result = static_cast<uint32_t>(signedValue >> shiftAmount);

    if (rd != 0)
        registers_[rd] = result;

    std::cout << "  SRA r" << std::dec << rd
              << ", r" << rt
              << ", " << shiftAmount
              << " -> 0x" << std::hex
              << std::uppercase << result << '\n';
}

void Cpu::executeSllv(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t shiftAmount = registers_[rs] & 0x1F;
    const uint32_t result = registers_[rt] << shiftAmount;

    if (rd != 0) registers_[rd] = result;

    std::cout << "  SLLV r" << std::dec << rd
              << ", r" << rt
              << ", r" << rs
              << " -> 0x" << std::hex
              << std::uppercase << result << '\n';
}

void Cpu::executeSrlv(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t shiftAmount = registers_[rs] & 0x1F;
    const uint32_t result = registers_[rt] >> shiftAmount;

    if (rd != 0) registers_[rd] = result;

    std::cout << "  SRLV r" << std::dec << rd
              << ", r" << rt
              << ", r" << rs
              << " -> 0x" << std::hex
              << std::uppercase << result << '\n';
}

void Cpu::executeSrav(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;
    const uint32_t rd = (instruction >> 11) & 0x1F;

    const uint32_t shiftAmount = registers_[rs] & 0x1F;

    const int32_t signedValue =
        static_cast<int32_t>(registers_[rt]);

    const uint32_t result =
        static_cast<uint32_t>(signedValue >> shiftAmount);

    if (rd != 0)
        registers_[rd] = result;

    std::cout << "  SRAV r" << std::dec << rd
              << ", r" << rt
              << ", r" << rs
              << " -> 0x" << std::hex
              << std::uppercase << result << '\n';
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

void Cpu::executeMult(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;

    const int64_t lhs =
        static_cast<int64_t>(static_cast<int32_t>(registers_[rs]));

    const int64_t rhs =
        static_cast<int64_t>(static_cast<int32_t>(registers_[rt]));

    const int64_t signedResult = lhs * rhs;
    const uint64_t result = static_cast<uint64_t>(signedResult);

    lo_ = static_cast<uint32_t>(result);
    hi_ = static_cast<uint32_t>(result >> 32);

    std::cout << "  MULT r" << std::dec << rs
              << ", r" << rt
              << " -> HI=0x" << std::hex << std::uppercase << hi_
              << " LO=0x" << lo_ << '\n';
}


void Cpu::executeMultu(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;

    const uint64_t lhs = registers_[rs];
    const uint64_t rhs = registers_[rt];
    const uint64_t result = lhs * rhs;

    lo_ = static_cast<uint32_t>(result);
    hi_ = static_cast<uint32_t>(result >> 32);

    std::cout << "  MULTU r" << std::dec << rs
              << ", r" << rt
              << " -> HI=0x" << std::hex << std::uppercase << hi_
              << " LO=0x" << lo_ << '\n';
}

void Cpu::executeDiv(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;

    const int32_t dividend = static_cast<int32_t>(registers_[rs]);
    const int32_t divisor  = static_cast<int32_t>(registers_[rt]);

    if (divisor == 0)
    {
        // R3000A division-by-zero behavior:
        // Negative dividend -> quotient 1
        // Non-negative dividend -> quotient -1
        lo_ = (dividend < 0) ? 1U : 0xFFFFFFFFU;
        hi_ = static_cast<uint32_t>(dividend);
    }
    else if (
        dividend == std::numeric_limits<int32_t>::min() &&
        divisor == -1)
    {
        // Avoid signed division overflow:
        // -2147483648 / -1 cannot be represented by int32_t.
        lo_ = 0x80000000U;
        hi_ = 0;
    }
    else
    {
        const int32_t quotient  = dividend / divisor;
        const int32_t remainder = dividend % divisor;

        lo_ = static_cast<uint32_t>(quotient);
        hi_ = static_cast<uint32_t>(remainder);
    }

    std::cout << "  DIV r" << std::dec << rs
              << ", r" << rt
              << " -> HI=0x" << std::hex << std::uppercase << hi_
              << " LO=0x" << lo_ << '\n';
}


void Cpu::executeDivu(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;
    const uint32_t rt = (instruction >> 16) & 0x1F;

    const uint32_t dividend = registers_[rs];
    const uint32_t divisor  = registers_[rt];

    if (divisor == 0)
    {
        lo_ = 0xFFFFFFFFU;
        hi_ = dividend;
    }
    else
    {
        lo_ = dividend / divisor;
        hi_ = dividend % divisor;
    }

    std::cout << "  DIVU r" << std::dec << rs
              << ", r" << rt
              << " -> HI=0x" << std::hex << std::uppercase << hi_
              << " LO=0x" << lo_ << '\n';
}

void Cpu::executeMfhi(uint32_t instruction)
{
    const uint32_t rd = (instruction >> 11) & 0x1F;

    if (rd != 0)
        registers_[rd] = hi_;

    std::cout << "  MFHI r" << std::dec << rd
              << " -> 0x" << std::hex
              << std::uppercase << hi_ << '\n';
}

void Cpu::executeMflo(uint32_t instruction)
{
    const uint32_t rd = (instruction >> 11) & 0x1F;

    if (rd != 0)
        registers_[rd] = lo_;

    std::cout << "  MFLO r" << std::dec << rd
              << " -> 0x" << std::hex
              << std::uppercase << lo_ << '\n';
}

void Cpu::executeMthi(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;

    hi_ = registers_[rs];

    std::cout << "  MTHI r" << std::dec << rs
              << " -> HI=0x" << std::hex
              << std::uppercase << hi_ << '\n';
}

void Cpu::executeMtlo(uint32_t instruction)
{
    const uint32_t rs = (instruction >> 21) & 0x1F;

    lo_ = registers_[rs];

    std::cout << "  MTLO r" << std::dec << rs
              << " -> LO=0x" << std::hex
              << std::uppercase << lo_ << '\n';
}

//31          26 25                              0
//+--------------+--------------------------------+
//| opcode       |       26-bit target index      |
//+--------------+--------------------------------+
     6 bits                 26 bits
void Cpu::executeJ(uint32_t instruction)
{
    const uint32_t opcode      = (instruction >> 26) & 0x3F;
    const uint32_t targetIndex = instruction & 0x03FFFFFF;

}

