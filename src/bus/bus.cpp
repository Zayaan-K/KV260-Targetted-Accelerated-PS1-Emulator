#include "bus.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

Bus::Bus(const std::string& biosPath)
    : ram_(RAM_SIZE, 0)
{
    std::ifstream file(biosPath, std::ios::binary);

    if (!file) {
        throw std::runtime_error(
            "Could not open BIOS: " + biosPath
        );
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
    // KSEG0: cached kernel address space.
    if (address >= 0x80000000 && address <= 0x9FFFFFFF) {
        return address - 0x80000000;
    }

    // KSEG1: uncached kernel address space.
    if (address >= 0xA0000000 && address <= 0xBFFFFFFF) {
        return address - 0xA0000000;
    }

    return address;
}

uint8_t Bus::read8(uint32_t address) const
{
    const uint32_t physical = virtualToPhysical(address);

    // The 2 MiB of RAM is mirrored throughout the first 8 MiB.
    if (physical < 0x00800000) {
        return ram_[physical % RAM_SIZE];
    }

    if (physical >= BIOS_START && physical < BIOS_START + BIOS_SIZE) {
        return bios_[physical - BIOS_START];
    }

    std::ostringstream message;

    message << "Read8 from unmapped address: 0x"
            << std::hex
            << std::uppercase
            << physical;

    throw std::runtime_error(message.str());
}

uint16_t Bus::read16(uint32_t address) const
{
    const uint16_t byte0 = static_cast<uint16_t>(read8(address));
    const uint16_t byte1 = static_cast<uint16_t>(read8(address + 1));

    return static_cast<uint16_t>(
        byte0 |
        (byte1 << 8)
    );
}

uint32_t Bus::read32(uint32_t address) const
{
    const uint32_t physical = virtualToPhysical(address);

    if ((physical >= MMIO_START && physical <= MMIO_END) || physical == CACHE_CONTROL_ADDRESS) {
        return readMmio32(physical);
    }

    const uint32_t byte0 = static_cast<uint32_t>(read8(address));
    const uint32_t byte1 = static_cast<uint32_t>(read8(address + 1));
    const uint32_t byte2 = static_cast<uint32_t>(read8(address + 2));
    const uint32_t byte3 = static_cast<uint32_t>(read8(address + 3));

    return byte0 | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);
}

void Bus::write8(uint32_t address, uint8_t value)
{
    const uint32_t physical = virtualToPhysical(address);

    if (physical < 0x00800000) {
        ram_[physical % RAM_SIZE] = value;
        return;
    }

    std::ostringstream message;

    message << "Write8 to unmapped or read-only address: 0x"
            << std::hex
            << std::uppercase
            << physical;

    throw std::runtime_error(message.str());
}

void Bus::write16(uint32_t address, uint16_t value)
{
    write8(address,static_cast<uint8_t>(value & 0xFF));
    write8(address + 1, static_cast<uint8_t>((value >> 8) & 0xFF));
}

void Bus::write32(uint32_t address, uint32_t value)
{
    const uint32_t physical = virtualToPhysical(address);

    if ((physical >= MMIO_START && physical <= MMIO_END) || physical == CACHE_CONTROL_ADDRESS) {
        writeMmio32(physical, value);
        return;
    }

    write8(address,static_cast<uint8_t>(value & 0xFF));
    write8(address + 1, static_cast<uint8_t>((value >> 8) & 0xFF));
    write8(address + 2, static_cast<uint8_t>((value >> 16) & 0xFF));
    write8(address + 3,static_cast<uint8_t>((value >> 24) & 0xFF) );
}

uint32_t Bus::readMmio32(uint32_t address) const
{
    switch (address) {
        case 0x1F801010:
            return biosRomDelaySize_;
        case 0x1F801060:
            return ramSizeRegister_;
        case 0x1F801020:
            return commonDelayRegister_;
        case 0x1F801000:
            return expansion1BaseAddress_;
        case 0x1F801004:
            return expansion2BaseAddress_;
        case 0x1F801008:
            return expansion1DelaySize_;
        case 0x1F801014:
            return spuDelaySize_;
        case 0x1F80100C:
            return expansion3DelaySize_;
        case 0x1F801018:
            return cdromDelaySize_;
        case 0x1F80101C:
            return expansion2DelaySize_;
        case CACHE_CONTROL_ADDRESS:
            return cacheControlRegister_;

        default:
            break;
    }

    std::ostringstream message;

    message << "Unhandled 32-bit MMIO read at 0x"
            << std::hex
            << std::uppercase
            << address;

    throw std::runtime_error(message.str());
}

void Bus::writeMmio32(uint32_t address, uint32_t value)
{
        
    switch (address) {
        case 0x1F801010:
            biosRomDelaySize_ = value;

            std::cout
                << "  MMIO BIOS ROM delay/size <- 0x"
                << std::hex
                << std::uppercase
                << std::setw(8)
                << std::setfill('0')
                << value
                << '\n';

            return;

        case 0x1F801060:
            ramSizeRegister_ = value;

            std::cout
                << "  MMIO RAM size/config <- 0x"
                << std::hex
                << std::uppercase
                << std::setw(8)
                << std::setfill('0')
                << value
                << '\n';

            return;

        case 0x1F801020:
            commonDelayRegister_ = value;

            std::cout
                << "  MMIO Common delay <- 0x"
                << std::hex
                << std::uppercase
                << std::setw(8)
                << std::setfill('0')
                << value
                << '\n';

            return;    

        case 0x1F801000:
            expansion1BaseAddress_ = value;

            std::cout
                << "  MMIO Expansion 1 base address <- 0x"
                << std::hex
                << std::uppercase
                << std::setw(8)
                << std::setfill('0')
                << value
                << '\n';

            return;

        case 0x1F801004:
            expansion2BaseAddress_ = value;

            std::cout
                << "  MMIO Expansion 2 base address <- 0x"
                << std::hex
                << std::uppercase
                << std::setw(8)
                << std::setfill('0')
                << value
                << '\n';

            return;
            
        case 0x1F801008:
            expansion1DelaySize_ = value;

            std::cout
                << "  MMIO Expansion 1 delay/size <- 0x"
                << std::hex
                << std::uppercase
                << std::setw(8)
                << std::setfill('0')
                << value
                << '\n';

            return; 
        case 0x1F801014:
            spuDelaySize_ = value;

            std::cout
                << "  MMIO SPU delay/size <- 0x"
                << std::hex
                << std::uppercase
                << std::setw(8)
                << std::setfill('0')
                << value
                << '\n';

            return;

        case 0x1F80100C:
            expansion3DelaySize_ = value;

            std::cout
                << "  MMIO Expansion 3 delay/size <- 0x"
                << std::hex
                << std::uppercase
                << std::setw(8)
                << std::setfill('0')
                << value
                << '\n';

            return;

        case 0x1F801018:
            cdromDelaySize_ = value;

            std::cout
                << "  MMIO CD-ROM delay/size <- 0x"
                << std::hex
                << std::uppercase
                << std::setw(8)
                << std::setfill('0')
                << value
                << '\n';

            return;

        case 0x1F80101C:
            expansion2DelaySize_ = value;

            std::cout
                << "  MMIO Expansion 2 delay/size <- 0x"
                << std::hex
                << std::uppercase
                << std::setw(8)
                << std::setfill('0')
                << value
                << '\n';

            return;

        case CACHE_CONTROL_ADDRESS:
            cacheControlRegister_ = value;

            std::cout
                << "  Cache control <- 0x"
                << std::hex
                << std::uppercase
                << std::setw(8)
                << std::setfill('0')
                << value
                << '\n';

            return;
            
        default:
            break;
    }

    std::ostringstream message;

    message << "Unhandled 32-bit MMIO write at 0x"
            << std::hex
            << std::uppercase
            << address
            << " value=0x"
            << value;

    throw std::runtime_error(message.str());
}

