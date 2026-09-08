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
    void executeSpecial(uint32_t instruction);
    


    //ALU

    void executeAdd(uint32_t instruction);
    void executeAddu(uint32_t instruction);
    void executeSub(uint32_t instruction);
    void executeSubu(uint32_t instruction);
    void executeAddi(uint32_t instruction);
    void executeAddiu(uint32_t instruction);

    void executeSlt(uint32_t instruction);
    void executeSltu(uint32_t instruction);
    void executeSlti(uint32_t instruction);
    void executeSltiu(uint32_t instruction);
    


    void executeAnd(uint32_t instruction);
    void executeOr(uint32_t instruction);
    void executeXor(uint32_t instruction);
    void executeNor(uint32_t instruction);
    void executeAndi(uint32_t instruction);
    void executeOri(uint32_t instruction);
    void executeXori(uint32_t instruction);

    void executeSllv(uint32_t instruction);
    void executeSrlv(uint32_t instruction);
    void executeSrav(uint32_t instruction);
    void executeSll(uint32_t instruction);
    void executeSrl(uint32_t instruction);
    void executeSra(uint32_t instruction);
    void executeLui(uint32_t instruction);


    Bus& bus_;

    std::array<uint32_t, 32> registers_{};

    uint32_t pc_ = 0;
    uint32_t nextPc_ = 0;
    uint32_t hi_ = 0;
    uint32_t lo_ = 0;
};
