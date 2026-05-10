#include "mqxx/moqt/full_track_name.hpp"

#include <algorithm>
#include <array>
#include <ranges>
#include <sstream>
#include <utility>

namespace mqxx::moqt {
namespace {

constexpr std::size_t kMaxNamespaceFields = 32U;
constexpr std::size_t kMaxFullTrackNameLength = 4096U;

[[nodiscard]] auto is_literal_byte(common::byte value) -> bool {
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

[[nodiscard]] auto append_parsed_component(std::string_view serialized, byte_string& output)
    -> common::result<common::unit, name_parse_error> {
    for (std::size_t index = 0; index < serialized.size(); ++index) {
        const char current = serialized[index];
        if (current != '.') {
            output.push_back(static_cast<std::uint8_t>(current));
            continue;
        }

        if (index + 2U >= serialized.size()) {
            return common::result<common::unit, name_parse_error>::failure(
                name_parse_error::invalid_escape);
        }

        const char high = serialized[index + 1U];
        const char low = serialized[index + 2U];
        if (std::isupper(static_cast<unsigned char>(high)) != 0 ||
            std::isupper(static_cast<unsigned char>(low)) != 0) {
            return common::result<common::unit, name_parse_error>::failure(
                name_parse_error::uppercase_hex_escape);
        }

        const int high_value = hex_digit_to_value(high);
        const int low_value = hex_digit_to_value(low);
        if (high_value < 0 || low_value < 0) {
            return common::result<common::unit, name_parse_error>::failure(
                name_parse_error::invalid_escape);
        }

        const auto decoded = static_cast<common::byte>((high_value << 4) | low_value);
        if (is_literal_byte(decoded)) {
            return common::result<common::unit, name_parse_error>::failure(
                name_parse_error::redundant_escape);
        }

        output.push_back(decoded);
        index += 2U;
    }

    return common::result<common::unit, name_parse_error>::success(common::unit{});
}

[[nodiscard]] auto total_binary_length(const full_track_name& name) -> std::size_t {
    std::size_t length = name.track_name.size();
    for (const byte_string& field : name.track_namespace) {
        length += field.size();
    }

    return length;
}

} // namespace

auto render_serialized_name(const byte_string& value) -> std::string {
    static constexpr auto kHex = std::to_array("0123456789abcdef");

    std::string rendered;
    for (const common::byte byte : value) {
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

auto render_serialized_full_track_name(const full_track_name& name) -> std::string {
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

auto parse_serialized_full_track_name(std::string_view serialized) -> name_parse_result {
    const std::size_t separator = serialized.find("--");
    if (separator == std::string_view::npos) {
        return name_parse_result::failure(name_parse_error::missing_track_separator);
    }

    full_track_name parsed_name;

    const std::string_view namespace_part = serialized.substr(0, separator);
    const std::string_view track_part = serialized.substr(separator + 2U);

    if (!namespace_part.empty()) {
        std::size_t begin = 0;
        while (begin <= namespace_part.size()) {
            const std::size_t end = namespace_part.find('-', begin);
            const std::string_view field = namespace_part.substr(
                begin, end == std::string_view::npos ? namespace_part.size() - begin : end - begin);

            if (field.empty()) {
                return name_parse_result::failure(name_parse_error::empty_namespace_field);
            }

            byte_string parsed_field;
            const auto parsed_field_result = append_parsed_component(field, parsed_field);
            if (!parsed_field_result.ok()) {
                return name_parse_result::failure(parsed_field_result.error());
            }

            parsed_name.track_namespace.push_back(std::move(parsed_field));
            if (parsed_name.track_namespace.size() > kMaxNamespaceFields) {
                return name_parse_result::failure(name_parse_error::too_many_namespace_fields);
            }

            if (end == std::string_view::npos) {
                break;
            }

            begin = end + 1U;
        }
    }

    const auto parsed_track_result = append_parsed_component(track_part, parsed_name.track_name);
    if (!parsed_track_result.ok()) {
        return name_parse_result::failure(parsed_track_result.error());
    }

    if (total_binary_length(parsed_name) > kMaxFullTrackNameLength) {
        return name_parse_result::failure(name_parse_error::full_track_name_too_long);
    }

    return name_parse_result::success(std::move(parsed_name));
}

auto namespace_starts_with(const track_namespace& candidate_namespace,
                           const track_namespace& prefix) -> bool {
    if (prefix.size() > candidate_namespace.size()) {
        return false;
    }

    return std::ranges::equal(prefix, candidate_namespace | std::views::take(prefix.size()));
}

} // namespace mqxx::moqt
