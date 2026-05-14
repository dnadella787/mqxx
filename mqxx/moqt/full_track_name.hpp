#pragma once

#include "mqxx/common/byte_buffer.hpp"
#include "mqxx/common/result.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace mqxx::moqt {

using byte_string = byte_string;
using track_namespace = std::vector<byte_string>;

struct full_track_name {
    track_namespace track_namespace;
    byte_string track_name;

    bool operator==(const full_track_name& other) const = default;
};

enum class name_parse_error {
    missing_track_separator,
    empty_namespace_field,
    invalid_escape,
    uppercase_hex_escape,
    redundant_escape,
    too_many_namespace_fields,
    full_track_name_too_long,
};

using name_parse_result = result<full_track_name, name_parse_error>;

[[nodiscard]] std::string render_serialized_name(const byte_string& value);
[[nodiscard]] std::string render_serialized_full_track_name(const full_track_name& name);
[[nodiscard]] name_parse_result parse_serialized_full_track_name(std::string_view serialized);
[[nodiscard]] bool namespace_starts_with(const track_namespace& candidate_namespace,
                                         const track_namespace& prefix);

} // namespace mqxx::moqt
