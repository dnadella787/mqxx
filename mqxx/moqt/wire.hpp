#pragma once

#include "mqxx/common/byte_buffer.hpp"
#include "mqxx/moqt/track_identity.hpp"

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace mqxx::moqt {

template <typename T> struct decoded {
    T value;
    std::size_t encoded_size;

    [[nodiscard]] bool operator==(const decoded& other) const = default;
};

using decoded_varint = decoded<std::uint64_t>;

struct encoded_varint {
    std::array<std::byte, 9> bytes{};
    std::size_t size = 0;

    [[nodiscard]] std::span<const std::byte> view() const noexcept;
};

struct location {
    std::uint64_t group;
    std::uint64_t object;

    auto operator<=>(const location&) const = default;
};

struct key_value_pair {
    std::uint64_t type;
    std::variant<std::uint64_t, byte_buffer> value;

    [[nodiscard]] bool operator==(const key_value_pair& other) const = default;
};

struct reason_phrase {
    static constexpr std::size_t max_byte_count = 1024;

    std::string text;

    [[nodiscard]] bool operator==(const reason_phrase& other) const = default;
};

enum class wire_error : std::uint8_t {
    truncated_input,
    invalid_varint_length,
    key_type_overflow,
    key_value_length_too_large,
    invalid_utf8,
    reason_phrase_too_large,
    namespace_field_count_too_large,
    empty_namespace_field_on_wire,
    namespace_too_large,
    full_track_name_too_large,
    invalid_type,
};

[[nodiscard]] std::expected<decoded_varint, wire_error>
decode_varint(std::span<const std::byte> encoded);

[[nodiscard]] encoded_varint encode_varint(std::uint64_t value);

[[nodiscard]] std::expected<decoded<location>, wire_error>
decode_location(std::span<const std::byte> encoded);

[[nodiscard]] byte_buffer encode_location(const location& value);

[[nodiscard]] std::expected<std::vector<key_value_pair>, wire_error>
decode_key_value_pairs(std::span<const std::byte> encoded);

[[nodiscard]] std::expected<byte_buffer, wire_error>
encode_key_value_pairs(std::span<const key_value_pair> pairs);

[[nodiscard]] std::expected<decoded<reason_phrase>, wire_error>
decode_reason_phrase(std::span<const std::byte> encoded);

[[nodiscard]] std::expected<byte_buffer, wire_error>
encode_reason_phrase(const reason_phrase& phrase);

[[nodiscard]] std::expected<decoded<track_namespace>, wire_error>
decode_track_namespace(std::span<const std::byte> encoded);

[[nodiscard]] byte_buffer encode_track_namespace(const track_namespace& name_space);

[[nodiscard]] std::expected<decoded<full_track_name>, wire_error>
decode_full_track_name(std::span<const std::byte> encoded);

[[nodiscard]] byte_buffer encode_full_track_name(const full_track_name& name);

[[nodiscard]] std::expected<byte_buffer, wire_error>
encode_setup(std::span<const key_value_pair> setup_payload);

[[nodiscard]] std::expected<std::vector<key_value_pair>, wire_error>
decode_setup(std::span<const std::byte> encoded);

} // namespace mqxx::moqt
