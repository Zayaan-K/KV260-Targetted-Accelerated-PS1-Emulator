#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Bus {
public:
    explicit Bus(const std::string& biosPath);

    uint8_t  read8(uint32_t address) const;
    uint16_t read16(uint32_t address) const;
    uint32_t read32(uint32_t address) const;

    void write8(uint32_t address, uint8_t value);
    void write16(uint32_t address, uint16_t value);
    void write32(uint32_t address, uint32_t value);

private:
    static constexpr uint32_t RAM_SIZE  = 2 * 1024 * 1024;
    static constexpr uint32_t BIOS_SIZE = 512 * 1024;

    static constexpr uint32_t BIOS_START = 0x1FC00000;
    static constexpr uint32_t MMIO_START = 0x1F801000;
    static constexpr uint32_t MMIO_END   = 0x1F802FFF;

    std::vector<uint8_t> ram_;
    std::vector<uint8_t> bios_;

    // Memory-control registers
    uint32_t biosRomDelaySize_ = 0;
    uint32_t ramSizeRegister_  = 0;
    uint32_t commonDelayRegister_ = 0;
    uint32_t expansion1BaseAddress_ = 0;
    uint32_t expansion1DelaySize_ = 0;
    uint32_t expansion2BaseAddress_ = 0;
    uint32_t spuDelaySize_ = 0;
    uint32_t expansion3DelaySize_ = 0;
    uint32_t cdromDelaySize_      = 0;
    uint32_t expansion2DelaySize_ = 0;

    static constexpr uint32_t CACHE_CONTROL_ADDRESS = 0xFFFE0130;
    uint32_t cacheControlRegister_ = 0;

    static uint32_t virtualToPhysical(uint32_t address);

    uint32_t readMmio32(uint32_t address) const;
    void writeMmio32(uint32_t address, uint32_t value);
};