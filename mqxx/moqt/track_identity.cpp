#include "mqxx/moqt/track_identity.hpp"

#include <utility>

namespace mqxx::moqt {
namespace {

constexpr char hex_digits[] = "0123456789abcdef";

bool is_literal_byte(const unsigned char value) noexcept {
    return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
           (value >= '0' && value <= '9') || value == '_';
}

void append_described_bytes(std::string& rendered, byte_buffer_view bytes);

std::string describe_track_bytes(const byte_buffer_view bytes) {
    std::string rendered;
    rendered.reserve(bytes.size() * 3);
    append_described_bytes(rendered, bytes);
    return rendered;
}

void append_described_bytes(std::string& rendered, const byte_buffer_view bytes) {
    for (const std::byte byte : bytes) {
        const unsigned char value = static_cast<unsigned char>(byte);
        if (is_literal_byte(value)) {
            rendered.push_back(static_cast<char>(value));
            continue;
        }

        rendered.push_back('.');
        rendered.push_back(hex_digits[value >> 4]);
        rendered.push_back(hex_digits[value & 0x0fU]);
    }
}

} // namespace

std::expected<track_namespace, track_identity_error>
track_namespace::make(const std::span<const byte_buffer_view> fields) {
    // we allow track namespaces without any fields but if there are fields,
    // each field must have at least 1 byte
    if (fields.size() > max_field_count) {
        return std::unexpected(track_identity_error::too_many_namespace_fields);
    }

    std::vector<field> owned_fields;
    owned_fields.reserve(fields.size());

    std::size_t byte_count = 0;
    for (const byte_buffer_view field_bytes : fields) {
        if (field_bytes.empty()) {
            return std::unexpected(track_identity_error::empty_namespace_field);
        }

        if (field_bytes.size() > max_byte_count - byte_count) {
            return std::unexpected(track_identity_error::namespace_too_large);
        }
        byte_count += field_bytes.size();

        owned_fields.emplace_back(field_bytes.begin(), field_bytes.end());
    }

    return track_namespace(std::move(owned_fields), byte_count);
}

std::span<const track_namespace::field> track_namespace::fields() const noexcept {
    return fields_;
}

std::size_t track_namespace::byte_count() const noexcept {
    return byte_count_;
}

std::string track_namespace::describe() const {
    std::string rendered;
    rendered.reserve(byte_count_ * 3 + fields_.size());

    for (std::size_t index = 0; index < fields_.size(); ++index) {
        if (index != 0) {
            rendered.push_back('-');
        }
        append_described_bytes(rendered, fields_[index]);
    }

    return rendered;
}

track_namespace::track_namespace(std::vector<field> fields, std::size_t byte_count)
    : fields_(std::move(fields)), byte_count_(byte_count) {}

std::expected<track_name, track_identity_error> track_name::make(byte_buffer_view bytes) {
    return track_name(std::vector(bytes.begin(), bytes.end()));
}

byte_buffer_view track_name::bytes() const noexcept {
    return bytes_;
}

std::size_t track_name::byte_count() const noexcept {
    return bytes_.size();
}

std::string track_name::describe() const {
    return describe_track_bytes(bytes_);
}

track_name::track_name(mqxx::byte_buffer owned_bytes) : bytes_(std::move(owned_bytes)) {}

std::expected<full_track_name, track_identity_error>
full_track_name::make(track_namespace name_space, track_name name) {
    if (name.byte_count() > max_byte_count - name_space.byte_count()) {
        return std::unexpected(track_identity_error::full_track_name_too_large);
    }

    return full_track_name(std::move(name_space), std::move(name));
}

const track_namespace& full_track_name::name_space() const noexcept {
    return namespace_;
}

const track_name& full_track_name::name() const noexcept {
    return name_;
}

std::size_t full_track_name::byte_count() const noexcept {
    return namespace_.byte_count() + name_.byte_count();
}

std::string full_track_name::describe() const {
    std::string rendered = namespace_.describe();
    rendered.append("--");
    append_described_bytes(rendered, name_.bytes());
    return rendered;
}

full_track_name::full_track_name(track_namespace name_space, track_name name)
    : namespace_(std::move(name_space)), name_(std::move(name)) {}

} // namespace mqxx::moqt
