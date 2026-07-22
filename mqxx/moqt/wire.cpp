#include "mqxx/moqt/wire.hpp"

#include <bit>
#include <limits>
#include <utility>

namespace mqxx::moqt {
namespace {

constexpr std::uint64_t max_key_value_byte_count = 65535;

std::size_t varint_size_for_value(const std::uint64_t value) noexcept {
    if (value <= ((std::uint64_t{1} << 7) - 1)) {
        return 1;
    }
    if (value <= ((std::uint64_t{1} << 14) - 1)) {
        return 2;
    }
    if (value <= ((std::uint64_t{1} << 21) - 1)) {
        return 3;
    }
    if (value <= ((std::uint64_t{1} << 28) - 1)) {
        return 4;
    }
    if (value <= ((std::uint64_t{1} << 35) - 1)) {
        return 5;
    }
    if (value <= ((std::uint64_t{1} << 42) - 1)) {
        return 6;
    }
    if (value <= ((std::uint64_t{1} << 49) - 1)) {
        return 7;
    }
    if (value <= ((std::uint64_t{1} << 56) - 1)) {
        return 8;
    }
    return 9;
}

void append_bytes(byte_buffer& out, const std::span<const std::byte> bytes) {
    out.insert(out.end(), bytes.begin(), bytes.end());
}

void append_varint(byte_buffer& out, const std::uint64_t value) {
    const encoded_varint encoded = encode_varint(value);
    append_bytes(out, encoded.view());
}

template <typename T>
void append_uintx(byte_buffer& out, T value) {
    static_assert(std::is_unsigned_v<T>, "value must be unsigned int");
    for (int i = static_cast<int>(sizeof(T)) - 1; i >= 0; --i) {
        out.push_back(static_cast<std::byte>((value >> (i * 8)) & 0xFF));
    }
}

bool is_valid_utf8(const std::span<const std::byte> bytes) noexcept {
    std::size_t index = 0;
    while (index < bytes.size()) {
        const auto lead = static_cast<unsigned char>(bytes[index]);
        std::size_t tail_count = 0;
        std::uint32_t code_point = 0;

        if ((lead & 0x80U) == 0) {
            ++index;
            continue;
        }
        if ((lead & 0xe0U) == 0xc0U) {
            tail_count = 1;
            code_point = lead & 0x1fU;
            if (code_point == 0) {
                return false;
            }
        } else if ((lead & 0xf0U) == 0xe0U) {
            tail_count = 2;
            code_point = lead & 0x0fU;
        } else if ((lead & 0xf8U) == 0xf0U) {
            tail_count = 3;
            code_point = lead & 0x07U;
            if (code_point > 0x04U) {
                return false;
            }
        } else {
            return false;
        }

        if (bytes.size() - index <= tail_count) {
            return false;
        }

        for (std::size_t offset = 1; offset <= tail_count; ++offset) {
            const auto tail = static_cast<unsigned char>(bytes[index + offset]);
            if ((tail & 0xc0U) != 0x80U) {
                return false;
            }
            code_point = (code_point << 6) | (tail & 0x3fU);
        }

        if ((tail_count == 1 && code_point < 0x80U) || (tail_count == 2 && code_point < 0x800U) ||
            (tail_count == 3 && code_point < 0x10000U) || code_point > 0x10ffffU ||
            (code_point >= 0xd800U && code_point <= 0xdfffU)) {
            return false;
        }

        index += tail_count + 1;
    }

    return true;
}

wire_error map_track_identity_error(const track_identity_error error) {
    switch (error) {
    case track_identity_error::empty_namespace_field:
        return wire_error::empty_namespace_field_on_wire;
    case track_identity_error::too_many_namespace_fields:
        return wire_error::namespace_field_count_too_large;
    case track_identity_error::namespace_too_large:
        return wire_error::namespace_too_large;
    case track_identity_error::full_track_name_too_large:
        return wire_error::full_track_name_too_large;
    }

    return wire_error::namespace_too_large;
}

} // namespace

std::span<const std::byte> encoded_varint::view() const noexcept {
    return {bytes.data(), size};
}

std::expected<decoded_varint, wire_error> decode_varint(const std::span<const std::byte> encoded) {
    if (encoded.empty()) {
        return std::unexpected(wire_error::truncated_input);
    }

    const auto first = static_cast<unsigned char>(encoded.front());
    const std::size_t encoded_size = static_cast<std::size_t>(std::countl_one(first)) + 1;
    if (encoded_size > 9) {
        return std::unexpected(wire_error::invalid_varint_length);
    }
    if (encoded.size() < encoded_size) {
        return std::unexpected(wire_error::truncated_input);
    }

    std::uint64_t value = 0;
    if (encoded_size < 9) {
        const unsigned payload_bits = static_cast<unsigned>(8 - encoded_size);
        const auto mask = payload_bits == 0 ? 0U : ((1U << payload_bits) - 1U);
        value = first & mask;
    }

    for (std::size_t index = 1; index < encoded_size; ++index) {
        value = (value << 8) | static_cast<unsigned char>(encoded[index]);
    }

    return decoded_varint{value, encoded_size};
}

encoded_varint encode_varint(const std::uint64_t value) {
    encoded_varint encoded;
    encoded.size = varint_size_for_value(value);

    std::uint64_t remaining = value;
    for (std::size_t index = encoded.size - 1; index > 0; --index) {
        encoded.bytes[index] = static_cast<std::byte>(remaining & 0xffU);
        remaining >>= 8;
    }

    if (encoded.size == 9) {
        encoded.bytes[0] = std::byte{0xff};
        return encoded;
    }

    const auto prefix =
        encoded.size == 1
            ? static_cast<unsigned char>(0)
            : static_cast<unsigned char>(((1U << (encoded.size - 1)) - 1U) << (9 - encoded.size));
    const unsigned payload_bits = static_cast<unsigned>(8 - encoded.size);
    const auto payload_mask =
        payload_bits == 0 ? std::uint64_t{0} : ((std::uint64_t{1} << payload_bits) - 1U);
    const auto first_payload = static_cast<unsigned char>(remaining & payload_mask);
    encoded.bytes[0] = static_cast<std::byte>(prefix | first_payload);
    return encoded;
}

std::expected<decoded<location>, wire_error>
decode_location(const std::span<const std::byte> encoded) {
    const auto group = decode_varint(encoded);
    if (!group.has_value()) {
        return std::unexpected(group.error());
    }

    const auto object = decode_varint(encoded.subspan(group->encoded_size));
    if (!object.has_value()) {
        return std::unexpected(object.error());
    }

    return decoded<location>{{group->value, object->value},
                             group->encoded_size + object->encoded_size};
}

byte_buffer encode_location(const location& value) {
    byte_buffer encoded;
    encoded.reserve(18);
    append_varint(encoded, value.group);
    append_varint(encoded, value.object);
    return encoded;
}

std::expected<std::vector<key_value_pair>, wire_error>
decode_key_value_pairs(const std::span<const std::byte> encoded) {
    std::vector<key_value_pair> pairs;
    std::uint64_t previous_type = 0;
    std::size_t offset = 0;

    while (offset < encoded.size()) {
        const auto type_delta = decode_varint(encoded.subspan(offset));
        if (!type_delta.has_value()) {
            return std::unexpected(type_delta.error());
        }
        offset += type_delta->encoded_size;

        if (previous_type > std::numeric_limits<std::uint64_t>::max() - type_delta->value) {
            return std::unexpected(wire_error::key_type_overflow);
        }
        const std::uint64_t type = previous_type + type_delta->value;
        previous_type = type;

        if ((type & 1U) == 0) {
            const auto value = decode_varint(encoded.subspan(offset));
            if (!value.has_value()) {
                return std::unexpected(value.error());
            }
            offset += value->encoded_size;
            pairs.push_back({type, value->value});
            continue;
        }

        const auto value_length = decode_varint(encoded.subspan(offset));
        if (!value_length.has_value()) {
            return std::unexpected(value_length.error());
        }
        offset += value_length->encoded_size;
        if (value_length->value > max_key_value_byte_count) {
            return std::unexpected(wire_error::key_value_length_too_large);
        }
        if (encoded.size() - offset < value_length->value) {
            return std::unexpected(wire_error::truncated_input);
        }

        byte_buffer value_bytes(encoded.data() + static_cast<std::ptrdiff_t>(offset),
                                encoded.data() +
                                    static_cast<std::ptrdiff_t>(offset + value_length->value));
        offset += static_cast<std::size_t>(value_length->value);
        pairs.push_back({type, std::move(value_bytes)});
    }

    return pairs;
}

std::expected<byte_buffer, wire_error>
encode_key_value_pairs(const std::span<const key_value_pair> pairs) {
    byte_buffer encoded;
    std::uint64_t previous_type = 0;

    for (const auto& pair : pairs) {
        if (pair.type < previous_type) {
            return std::unexpected(wire_error::key_type_overflow);
        }
        append_varint(encoded, pair.type - previous_type);
        previous_type = pair.type;

        if (const auto* integer_value = std::get_if<std::uint64_t>(&pair.value)) {
            if ((pair.type & 1U) != 0) {
                return std::unexpected(wire_error::key_value_length_too_large);
            }
            append_varint(encoded, *integer_value);
            continue;
        }

        const auto* bytes_value = std::get_if<byte_buffer>(&pair.value);
        if ((pair.type & 1U) == 0 || bytes_value == nullptr ||
            bytes_value->size() > max_key_value_byte_count) {
            return std::unexpected(wire_error::key_value_length_too_large);
        }

        append_varint(encoded, bytes_value->size());
        append_bytes(encoded, *bytes_value);
    }

    return encoded;
}

std::expected<decoded<reason_phrase>, wire_error>
decode_reason_phrase(const std::span<const std::byte> encoded) {
    const auto length = decode_varint(encoded);
    if (!length.has_value()) {
        return std::unexpected(length.error());
    }
    if (length->value > reason_phrase::max_byte_count) {
        return std::unexpected(wire_error::reason_phrase_too_large);
    }
    if (encoded.size() - length->encoded_size < length->value) {
        return std::unexpected(wire_error::truncated_input);
    }

    const auto payload =
        encoded.subspan(length->encoded_size, static_cast<std::size_t>(length->value));
    if (!is_valid_utf8(payload)) {
        return std::unexpected(wire_error::invalid_utf8);
    }

    return decoded<reason_phrase>{
        {std::string(reinterpret_cast<const char*>(payload.data()), payload.size())},
        length->encoded_size + payload.size()};
}

std::expected<byte_buffer, wire_error> encode_reason_phrase(const reason_phrase& phrase) {
    if (phrase.text.size() > reason_phrase::max_byte_count) {
        return std::unexpected(wire_error::reason_phrase_too_large);
    }

    const std::span<const char> chars(phrase.text.data(), phrase.text.size());
    const auto payload = std::as_bytes(chars);
    if (!is_valid_utf8(payload)) {
        return std::unexpected(wire_error::invalid_utf8);
    }

    byte_buffer encoded;
    encoded.reserve(9 + payload.size());
    append_varint(encoded, payload.size());
    append_bytes(encoded, payload);
    return encoded;
}

std::expected<decoded<track_namespace>, wire_error>
decode_track_namespace(const std::span<const std::byte> encoded) {
    const auto field_count = decode_varint(encoded);
    if (!field_count.has_value()) {
        return std::unexpected(field_count.error());
    }
    if (field_count->value > track_namespace::max_field_count) {
        return std::unexpected(wire_error::namespace_field_count_too_large);
    }

    std::vector<byte_buffer> owned_fields;
    owned_fields.reserve(static_cast<std::size_t>(field_count->value));

    std::size_t offset = field_count->encoded_size;
    std::size_t total_field_bytes = 0;
    for (std::uint64_t index = 0; index < field_count->value; ++index) {
        const auto field_length = decode_varint(encoded.subspan(offset));
        if (!field_length.has_value()) {
            return std::unexpected(field_length.error());
        }
        offset += field_length->encoded_size;

        if (field_length->value == 0) {
            return std::unexpected(wire_error::empty_namespace_field_on_wire);
        }
        if (field_length->value > track_namespace::max_byte_count - total_field_bytes) {
            return std::unexpected(wire_error::namespace_too_large);
        }
        if (encoded.size() - offset < field_length->value) {
            return std::unexpected(wire_error::truncated_input);
        }

        owned_fields.emplace_back(encoded.data() + static_cast<std::ptrdiff_t>(offset),
                                  encoded.data() +
                                      static_cast<std::ptrdiff_t>(offset + field_length->value));
        offset += static_cast<std::size_t>(field_length->value);
        total_field_bytes += static_cast<std::size_t>(field_length->value);
    }

    std::vector<byte_buffer_view> field_views;
    field_views.reserve(owned_fields.size());
    for (const auto& field : owned_fields) {
        field_views.emplace_back(field);
    }

    auto name_space = track_namespace::make(field_views);
    if (!name_space.has_value()) {
        return std::unexpected(map_track_identity_error(name_space.error()));
    }

    return decoded<track_namespace>{std::move(name_space).value(), offset};
}

byte_buffer encode_track_namespace(const track_namespace& name_space) {
    byte_buffer encoded;
    encoded.reserve(9 + name_space.byte_count() + name_space.fields().size() * 9);
    append_varint(encoded, name_space.fields().size());
    for (const auto& field : name_space.fields()) {
        append_varint(encoded, field.size());
        append_bytes(encoded, field);
    }
    return encoded;
}

std::expected<decoded<full_track_name>, wire_error>
decode_full_track_name(const std::span<const std::byte> encoded) {
    const auto decoded_namespace = decode_track_namespace(encoded);
    if (!decoded_namespace.has_value()) {
        return std::unexpected(decoded_namespace.error());
    }

    const auto name_length = decode_varint(encoded.subspan(decoded_namespace->encoded_size));
    if (!name_length.has_value()) {
        return std::unexpected(name_length.error());
    }

    const std::size_t name_offset = decoded_namespace->encoded_size + name_length->encoded_size;
    if (encoded.size() - name_offset < name_length->value) {
        return std::unexpected(wire_error::truncated_input);
    }

    byte_buffer name_bytes(encoded.data() + static_cast<std::ptrdiff_t>(name_offset),
                           encoded.data() +
                               static_cast<std::ptrdiff_t>(name_offset + name_length->value));
    auto name = track_name::make(name_bytes);
    auto full_name =
        full_track_name::make(std::move(decoded_namespace->value), std::move(name).value());
    if (!full_name.has_value()) {
        return std::unexpected(map_track_identity_error(full_name.error()));
    }

    return decoded<full_track_name>{std::move(full_name).value(),
                                    name_offset + static_cast<std::size_t>(name_length->value)};
}

byte_buffer encode_full_track_name(const full_track_name& name) {
    byte_buffer encoded = encode_track_namespace(name.name_space());
    append_varint(encoded, name.name().byte_count());
    append_bytes(encoded, name.name().bytes());
    return encoded;
}

// prefer to use setup payload directly rather than unwrapping
std::expected<byte_buffer, wire_error>
encode_setup(std::span<const key_value_pair> setup_payload) {
    byte_buffer encoded;
    auto encoded_params = encode_key_value_pairs(setup_payload);
    if (!encoded_params) {
        return std::unexpected(encoded_params.error());
    }

    if (encoded_params->size() > std::numeric_limits<std::uint16_t>::max()) {
        return std::unexpected(wire_error::key_value_length_too_large);
    }
    const auto length = static_cast<std::uint16_t>(encoded_params->size());

    append_varint(encoded, 0x2F00U);
    append_uintx(encoded, length);
    append_bytes(encoded, *encoded_params);

    return encoded;
}

std::expected<std::vector<key_value_pair>, wire_error>
decode_setup(std::span<const std::byte> encoded) {
    const auto type = decode_varint(encoded);
    
    if (!type.has_value()) {
        return std::unexpected(type.error());
    }
    if (type->value != 0x2F00U) {
        return std::unexpected(wire_error::invalid_type);
    }

    std::size_t offset = type->encoded_size;
    if (encoded.size() < offset + 2) {
        return std::unexpected(wire_error::truncated_input);
    }

    const unsigned length = static_cast<unsigned>(encoded[offset] << 8) | static_cast<unsigned>(encoded[offset + 1]);
    offset += 2;
    if (encoded.size() < offset + length) {
        return std::unexpected(wire_error::truncated_input);
    }

    const auto options = decode_key_value_pairs(encoded.subspan(offset, length));

    // empty options still returns blank KVP
    return options;
}

} // namespace mqxx::moqt
