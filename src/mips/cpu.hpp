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
    void executeRegimm(uint32_t instruction);

    void executeSyscall(uint32_t instruction);
    void executeBreak(uint32_t instruction);

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

    void executeMult(uint32_t instruction);
    void executeMultu(uint32_t instruction);
    void executeDiv(uint32_t instruction);
    void executeDivu(uint32_t instruction);
    void executeMfhi(uint32_t instruction);
    void executeMflo(uint32_t instruction);
    void executeMthi(uint32_t instruction);
    void executeMtlo(uint32_t instruction);


    void executeJ(uint32_t instruction);
    void executeJal(uint32_t instruction);
    void executeJr(uint32_t instruction);
    void executeJalr(uint32_t instruction);

    void executeBeq(uint32_t instruction);
    void executeBne(uint32_t instruction);
    void executeBltz(uint32_t instruction);
    void executeBgez(uint32_t instruction);
    void executeBgtz(uint32_t instruction);
    void executeBlez(uint32_t instruction);
    void executeBltzal(uint32_t instruction);
    void executeBgezal(uint32_t instruction);

    void executeLb(uint32_t instruction);
    void executeLbu(uint32_t instruction);
    void executeLh(uint32_t instruction);
    void executeLhu(uint32_t instruction);
    void executeLw(uint32_t instruction);

    void executeSb(uint32_t instruction);
    void executeSh(uint32_t instruction);
    void executeSw(uint32_t instruction);

    void scheduleLoad(uint32_t destination, uint32_t value);


    Bus& bus_;

    struct PendingLoad {
    bool valid = false;
    uint32_t destination = 0;
    uint32_t value = 0;
    };



    PendingLoad pendingLoad_{};

    std::array<uint32_t, 32> registers_{};

    uint32_t pc_ = 0;
    uint32_t nextPc_ = 0;
    uint32_t hi_ = 0;
    uint32_t lo_ = 0;
};
