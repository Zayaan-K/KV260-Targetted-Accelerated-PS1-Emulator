#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

constexpr uint32_t BIOS_PHYSICAL_BASE = 0x1FC00000;
constexpr uint32_t RESET_VECTOR       = 0xBFC00000;
constexpr std::size_t BIOS_SIZE       = 512 * 1024;

uint32_t virtualToPhysical(uint32_t address)
{
    // Converts the PS1 KSEG0/KSEG1 address to a physical address.
    return address & 0x1FFFFFFF;
}

uint32_t read32(const std::vector<uint8_t>& bios, uint32_t address)
{
    const uint32_t physicalAddress = virtualToPhysical(address);

    if (physicalAddress < BIOS_PHYSICAL_BASE ||
        physicalAddress + 3 >= BIOS_PHYSICAL_BASE + bios.size()) {
        throw std::out_of_range("Address is outside the BIOS");
    }

    const uint32_t offset = physicalAddress - BIOS_PHYSICAL_BASE;

    // The PS1 MIPS CPU is little-endian.
    return static_cast<uint32_t>(bios[offset]) |
           (static_cast<uint32_t>(bios[offset + 1]) << 8) |
           (static_cast<uint32_t>(bios[offset + 2]) << 16) |
           (static_cast<uint32_t>(bios[offset + 3]) << 24);
}

int main()
{
    std::ifstream file("bios/SCPH1001.BIN", std::ios::binary);

    if (!file) {
        std::cerr << "Could not open BIOS\n";
        return 1;
    }

    std::vector<uint8_t> bios{
        std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}
    };

    if (bios.size() != BIOS_SIZE) {
        std::cerr << "Incorrect BIOS size: "
                  << bios.size() << " bytes\n";
        return 1;
    }

    const uint32_t instruction = read32(bios, RESET_VECTOR);

    std::cout << std::hex << std::uppercase << std::setfill('0');
    std::cout << "Reset PC:          0x"
              << std::setw(8) << RESET_VECTOR << '\n';
    std::cout << "First instruction: 0x"
              << std::setw(8) << instruction << '\n';
}
