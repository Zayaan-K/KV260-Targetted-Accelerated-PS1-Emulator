#include "bus.hpp"

#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>

Bus::Bus(const std::string& biosPath)
    : ram_(RAM_SIZE, 0)
{
    std::ifstream file(biosPath, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Could not open BIOS: " + biosPath);
    }

    bios_ = std::vector<uint8_t>{
        std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}
    };

    if (bios_.size() != BIOS_SIZE) {
        throw std::runtime_error(
            "BIOS must be exactly 524288 bytes"
        );
    }
}

uint32_t Bus::virtualToPhysical(uint32_t address)
{
    // KSEG0: cached address space.
    if (address >= 0x80000000 && address <= 0x9FFFFFFF) {
        return address - 0x80000000;
    }

    // KSEG1: uncached address space. The BIOS reset vector uses this.
    if (address >= 0xA0000000 && address <= 0xBFFFFFFF) {
        return address - 0xA0000000;
    }

    // KUSEG addresses are already physical for this early implementation.
    if (address < 0x80000000) {
        return address;
    }

    throw std::out_of_range("Unsupported virtual address");
}

uint8_t Bus::read8(uint32_t address) const
{
    const uint32_t physical = virtualToPhysical(address);

    // Main RAM is mirrored throughout the first 8 MiB.
    if (physical < RAM_MIRROR_END) {
        return ram_[physical % RAM_SIZE];
    }

    if (physical >= BIOS_BASE &&
        physical < BIOS_BASE + BIOS_SIZE) {
        return bios_[physical - BIOS_BASE];
    }

    std::ostringstream message;
    message << "Read from unmapped address: 0x"
            << std::hex << address;

    throw std::out_of_range(message.str());
}

uint16_t Bus::read16(uint32_t address) const
{
    return static_cast<uint16_t>(read8(address)) |
           (static_cast<uint16_t>(read8(address + 1)) << 8);
}

uint32_t Bus::read32(uint32_t address) const
{
    return static_cast<uint32_t>(read8(address)) |
           (static_cast<uint32_t>(read8(address + 1)) << 8) |
           (static_cast<uint32_t>(read8(address + 2)) << 16) |
           (static_cast<uint32_t>(read8(address + 3)) << 24);
}

void Bus::write8(uint32_t address, uint8_t value)
{
    const uint32_t physical = virtualToPhysical(address);

    if (physical < RAM_MIRROR_END) {
        ram_[physical % RAM_SIZE] = value;
        return;
    }

    std::ostringstream message;
    message << "Write to unmapped or read-only address: 0x"
            << std::hex << address;

    throw std::out_of_range(message.str());
}

void Bus::write16(uint32_t address, uint16_t value)
{
    write8(address, static_cast<uint8_t>(value));
    write8(address + 1, static_cast<uint8_t>(value >> 8));
}

void Bus::write32(uint32_t address, uint32_t value)
{
    write8(address, static_cast<uint8_t>(value));
    write8(address + 1, static_cast<uint8_t>(value >> 8));
    write8(address + 2, static_cast<uint8_t>(value >> 16));
    write8(address + 3, static_cast<uint8_t>(value >> 24));
}
