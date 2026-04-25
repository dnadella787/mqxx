#include "saltpepper/moqt/full_track_name.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>

namespace saltpepper::moqt {
namespace {

constexpr std::size_t kMaxNamespaceFields = 32U;
constexpr std::size_t kMaxFullTrackNameLength = 4096U;

[[nodiscard]] auto is_literal_byte(std::uint8_t value) -> bool {
    return std::isalnum(static_cast<unsigned char>(value)) != 0 ||
           value == static_cast<std::uint8_t>('_');
}

[[nodiscard]] auto hex_digit_to_value(char value) -> int {
    if (value >= '0' && value <= '9') {
        return value - '0';
    }

    if (value >= 'a' && value <= 'f') {
        return 10 + (value - 'a');
    }

    if (value >= 'A' && value <= 'F') {
        return 10 + (value - 'A');
    }

    return -1;
}

[[nodiscard]] auto append_parsed_component(std::string_view serialized, ByteString& output)
    -> std::optional<NameParseError> {
    for (std::size_t index = 0; index < serialized.size(); ++index) {
        const char current = serialized[index];
        if (current != '.') {
            output.push_back(static_cast<std::uint8_t>(current));
            continue;
        }

        if (index + 2U >= serialized.size()) {
            return NameParseError::invalid_escape;
        }

        const char high = serialized[index + 1U];
        const char low = serialized[index + 2U];
        if (std::isupper(static_cast<unsigned char>(high)) != 0 ||
            std::isupper(static_cast<unsigned char>(low)) != 0) {
            return NameParseError::uppercase_hex_escape;
        }

        const int high_value = hex_digit_to_value(high);
        const int low_value = hex_digit_to_value(low);
        if (high_value < 0 || low_value < 0) {
            return NameParseError::invalid_escape;
        }

        const auto decoded = static_cast<std::uint8_t>((high_value << 4) | low_value);
        if (is_literal_byte(decoded)) {
            return NameParseError::redundant_escape;
        }

        output.push_back(decoded);
        index += 2U;
    }

    return std::nullopt;
}

[[nodiscard]] auto total_binary_length(const FullTrackName& name) -> std::size_t {
    std::size_t length = name.track_name.size();
    for (const ByteString& field : name.track_namespace) {
        length += field.size();
    }

    return length;
}

} // namespace

auto render_serialized_name(const ByteString& value) -> std::string {
    static constexpr auto kHex = std::to_array("0123456789abcdef");

    std::string rendered;
    for (const std::uint8_t byte : value) {
        if (is_literal_byte(byte)) {
            rendered.push_back(static_cast<char>(byte));
            continue;
        }

        rendered.push_back('.');
        rendered.push_back(kHex[(byte >> 4U) & 0x0fU]);
        rendered.push_back(kHex[byte & 0x0fU]);
    }

    return rendered;
}

auto render_serialized_full_track_name(const FullTrackName& name) -> std::string {
    std::ostringstream output;
    for (std::size_t index = 0; index < name.track_namespace.size(); ++index) {
        if (index != 0U) {
            output << '-';
        }

        output << render_serialized_name(name.track_namespace[index]);
    }

    output << "--" << render_serialized_name(name.track_name);
    return output.str();
}

auto parse_serialized_full_track_name(std::string_view serialized) -> NameParseResult {
    const std::size_t separator = serialized.find("--");
    if (separator == std::string_view::npos) {
        return {.value = std::nullopt, .error = NameParseError::missing_track_separator};
    }

    FullTrackName parsed_name;

    std::string_view namespace_part = serialized.substr(0, separator);
    std::string_view track_part = serialized.substr(separator + 2U);

    if (!namespace_part.empty()) {
        std::size_t begin = 0;
        while (begin <= namespace_part.size()) {
            const std::size_t end = namespace_part.find('-', begin);
            const std::string_view field = namespace_part.substr(
                begin, end == std::string_view::npos ? namespace_part.size() - begin : end - begin);

            if (field.empty()) {
                return {.value = std::nullopt, .error = NameParseError::empty_namespace_field};
            }

            ByteString parsed_field;
            if (const auto error = append_parsed_component(field, parsed_field);
                error.has_value()) {
                return {.value = std::nullopt, .error = *error};
            }

            parsed_name.track_namespace.push_back(std::move(parsed_field));
            if (parsed_name.track_namespace.size() > kMaxNamespaceFields) {
                return {.value = std::nullopt, .error = NameParseError::too_many_namespace_fields};
            }

            if (end == std::string_view::npos) {
                break;
            }

            begin = end + 1U;
        }
    }

    if (const auto error = append_parsed_component(track_part, parsed_name.track_name);
        error.has_value()) {
        return {.value = std::nullopt, .error = *error};
    }

    if (total_binary_length(parsed_name) > kMaxFullTrackNameLength) {
        return {.value = std::nullopt, .error = NameParseError::full_track_name_too_long};
    }

    return {.value = std::move(parsed_name), .error = NameParseError::none};
}

auto namespace_starts_with(const TrackNamespace& track_namespace, const TrackNamespace& prefix)
    -> bool {
    if (prefix.size() > track_namespace.size()) {
        return false;
    }

    return std::equal(prefix.begin(), prefix.end(), track_namespace.begin());
}

} // namespace saltpepper::moqt
