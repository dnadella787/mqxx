#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace saltpepper::moqt {

using ByteString = std::vector<std::uint8_t>;
using TrackNamespace = std::vector<ByteString>;

struct FullTrackName {
    TrackNamespace track_namespace;
    ByteString track_name;

    auto operator==(const FullTrackName& other) const -> bool = default;
};

enum class NameParseError {
    none,
    missing_track_separator,
    empty_namespace_field,
    invalid_escape,
    uppercase_hex_escape,
    redundant_escape,
    too_many_namespace_fields,
    full_track_name_too_long,
};

struct NameParseResult {
    std::optional<FullTrackName> value;
    NameParseError error{NameParseError::none};

    [[nodiscard]] auto ok() const -> bool {
        return value.has_value();
    }
};

[[nodiscard]] auto render_serialized_name(const ByteString& value) -> std::string;
[[nodiscard]] auto render_serialized_full_track_name(const FullTrackName& name) -> std::string;
[[nodiscard]] auto parse_serialized_full_track_name(std::string_view serialized) -> NameParseResult;
[[nodiscard]] auto namespace_starts_with(const TrackNamespace& track_namespace,
                                         const TrackNamespace& prefix) -> bool;

} // namespace saltpepper::moqt
