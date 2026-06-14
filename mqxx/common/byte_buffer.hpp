#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace mqxx {

using byte_buffer = std::vector<std::byte>;
using byte_buffer_view = std::span<const std::byte>;

} // namespace mqxx
