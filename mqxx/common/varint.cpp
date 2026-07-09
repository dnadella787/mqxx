#include "varint.hpp"

namespace mqxx {
using byte_buffer = std::vector<std::byte>;

byte_buffer encode_varint(std::uint64_t value) {
    std::size_t length = 0;
    std::byte prefix;

    // determining length
    // 1.4.1. table 1
    if (value <= 127) {
        length = 1;
        prefix = std::byte{0b00000000};
    } 
    else if (value <= 16383) {
        length = 2;
        prefix = std::byte{0b10000000};
    }
    else if (value <= 2097151) {
        length = 3;
        prefix = std::byte{0b11000000};
    }
    else if (value <= 268435455) {
        length = 4;
        prefix = std::byte{0b11100000};
    }
    else if (value <= 34359738367) {
        length = 5;
        prefix = std::byte{0b11110000};
    }
    else if (value <= 4398046511103) {
        length = 6;
        prefix = std::byte{0b11111000};
    }
    else if (value <= 562949953421311) {
        length = 7;
        prefix = std::byte{0b11111100};
    }
    else if (value <= 72057594037927935) {
        length = 8;
        prefix = std::byte{0b11111110};
    }
    else {
        length = 9;
        prefix = std::byte{0b11111111};
    }

    if (length == 0) throw std::out_of_range(std::to_string(length));

    byte_buffer output(length);

    // convert and add prefix bits
    // fill back to front
    for (std::size_t i = length; i > 0; i--) {
        output[i - 1] = static_cast<std::byte>(value & 0xFF);
        value >>= 8;
    }

    // OR prefix bits to top byte
    output[0] |= prefix;

    return output;

}

} // namespace mqxx

int main() {
    std::uint64_t input = 37;

    const auto encoded = mqxx::encode_varint(input);

    std::cout << "Input: " << input << "\n";
    std::cout << "Encoded: ";

    for (const auto byte : encoded) {
        std::cout
            << "0x"
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << std::to_integer<int>(byte)
            << " ";
    }

    std::cout << "\n";

    return 0;
}