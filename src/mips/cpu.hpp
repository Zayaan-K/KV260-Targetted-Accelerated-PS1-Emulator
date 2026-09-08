#pragma once

#include <array>
#include <cstdint>

class Bus;

class Cpu {
public:
    explicit Cpu(Bus& bus);

    void reset();
    void step();

private:
    void execute(uint32_t instruction);
    void executeLui(uint32_t instruction);
    void executeOri(uint32_t instruction);

    Bus& bus_;

    std::array<uint32_t, 32> registers_{};

    uint32_t pc_ = 0;
    uint32_t nextPc_ = 0;
    uint32_t hi_ = 0;
    uint32_t lo_ = 0;
};
