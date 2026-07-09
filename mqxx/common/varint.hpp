#pragma once

#include <cstddef>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <vector>

namespace mqxx {

std::vector<std::byte> encode_varint(std::uint64_t value);

}