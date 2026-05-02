#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mqxx::moqt {

using byte_string = std::vector<std::uint8_t>;
using track_namespace = std::vector<byte_string>;

struct full_track_name {
    track_namespace track_namespace;
    byte_string track_name;

    auto operator==(const full_track_name& other) const -> bool = default;
};

enum class name_parse_error {
    none,
    missing_track_separator,
    empty_namespace_field,
    invalid_escape,
    uppercase_hex_escape,
    redundant_escape,
    too_many_namespace_fields,
    full_track_name_too_long,
};

struct name_parse_result {
    std::optional<full_track_name> value;
    name_parse_error error{name_parse_error::none};

    [[nodiscard]] auto ok() const -> bool {
        return value.has_value();
    }
};

[[nodiscard]] auto render_serialized_name(const byte_string& value) -> std::string;
[[nodiscard]] auto render_serialized_full_track_name(const full_track_name& name) -> std::string;
[[nodiscard]] auto parse_serialized_full_track_name(std::string_view serialized)
    -> name_parse_result;
[[nodiscard]] auto namespace_starts_with(const track_namespace& candidate_namespace,
                                         const track_namespace& prefix) -> bool;

} // namespace mqxx::moqt
