#pragma once

#include "mqxx/common/byte_buffer.hpp"

#include <compare>
#include <cstddef>
#include <expected>
#include <span>
#include <vector>

namespace mqxx::moqt {

enum class track_identity_error : std::uint8_t {
    empty_namespace_field,
    too_many_namespace_fields,
    namespace_too_large,
    full_track_name_too_large,
};

class track_namespace {
  public:
    using field = std::vector<byte>;

    static constexpr std::size_t max_field_count = 32;
    static constexpr std::size_t max_byte_count = 4096;

    [[nodiscard]] static std::expected<track_namespace, track_identity_error>
    make(std::span<const byte_view> fields);

    [[nodiscard]] std::span<const field> fields() const noexcept;
    [[nodiscard]] std::size_t byte_count() const noexcept;

    [[nodiscard]] bool operator==(const track_namespace& other) const = default;
    [[nodiscard]] std::strong_ordering operator<=>(const track_namespace& other) const = default;

  private:
    explicit track_namespace(std::vector<field> fields, std::size_t byte_count);

    std::vector<field> fields_;
    std::size_t byte_count_ = 0;
};

class track_name {
  public:
    [[nodiscard]] static std::expected<track_name, track_identity_error> make(byte_view bytes);

    [[nodiscard]] byte_view bytes() const noexcept;
    [[nodiscard]] std::size_t byte_count() const noexcept;

    [[nodiscard]] bool operator==(const track_name& other) const = default;
    [[nodiscard]] std::strong_ordering operator<=>(const track_name& other) const = default;

  private:
    explicit track_name(std::vector<byte> bytes);

    std::vector<byte> bytes_;
};

class full_track_name {
  public:
    static constexpr std::size_t max_byte_count = 4096;

    [[nodiscard]] static std::expected<full_track_name, track_identity_error>
    make(track_namespace name_space, track_name name);

    [[nodiscard]] const track_namespace& name_space() const noexcept;
    [[nodiscard]] const track_name& name() const noexcept;
    [[nodiscard]] std::size_t byte_count() const noexcept;

    [[nodiscard]] bool operator==(const full_track_name& other) const = default;
    [[nodiscard]] std::strong_ordering operator<=>(const full_track_name& other) const = default;

  private:
    full_track_name(track_namespace name_space, track_name name);

    track_namespace namespace_;
    track_name name_;
};

} // namespace mqxx::moqt
