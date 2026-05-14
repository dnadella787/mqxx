#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace mqxx {

using byte = std::uint8_t;
using byte_string = std::vector<byte>;
using byte_view = std::span<const byte>;

} // namespace mqxx::common
