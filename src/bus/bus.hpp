#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Bus {
public:
    explicit Bus(const std::string& biosPath);

    uint8_t read8(uint32_t address) const;
    uint16_t read16(uint32_t address) const;
    uint32_t read32(uint32_t address) const;

    void write8(uint32_t address, uint8_t value);
    void write16(uint32_t address, uint16_t value);
    void write32(uint32_t address, uint32_t value);

private:
    static constexpr uint32_t RAM_SIZE = 2 * 1024 * 1024;
    static constexpr uint32_t RAM_MIRROR_END = 0x00800000;

    static constexpr uint32_t BIOS_BASE = 0x1FC00000;
    static constexpr uint32_t BIOS_SIZE = 512 * 1024;

    std::vector<uint8_t> ram_;
    std::vector<uint8_t> bios_;

    static uint32_t virtualToPhysical(uint32_t address);
};
