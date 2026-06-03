#include "mqxx/moqt/track_identity.hpp"

#include <array>
#include <vector>

#include <gtest/gtest.h>

namespace mqxx::moqt {
namespace {

byte_view as_view(const std::vector<byte>& bytes) {
    return bytes;
}

TEST(track_namespace_test, accepts_empty_namespace_prefix) {
    constexpr std::array<byte_view, 0> fields = {};
    const auto ns = track_namespace::make(fields);

    ASSERT_TRUE(ns.has_value());
    EXPECT_TRUE(ns->fields().empty());
    EXPECT_EQ(ns->byte_count(), 0U);
}

TEST(track_namespace_test, preserves_binary_fields) {
    std::vector first = {byte{0x00}, byte{0x7f}, byte{0xff}};
    std::vector second = {byte{0x12}};
    std::array fields {as_view(first), as_view(second)};

    const auto name_space = track_namespace::make(fields);

    ASSERT_TRUE(name_space.has_value());
    ASSERT_EQ(name_space->fields().size(), 2U);
    ASSERT_EQ(name_space->fields()[0].size(), 3U);
    EXPECT_EQ(name_space->fields()[0][0], byte{0x00});
    EXPECT_EQ(name_space->fields()[0][1], byte{0x7f});
    EXPECT_EQ(name_space->fields()[0][2], byte{0xff});
    EXPECT_EQ(name_space->fields()[1][0], byte{0x12});
    EXPECT_EQ(name_space->byte_count(), 4U);
}

TEST(track_namespace_test, owns_copied_field_bytes) {
    std::array source = {byte{0x10}};
    std::array fields = {byte_view(source)};

    auto ns = track_namespace::make(fields);
    source[0] = byte{0x20};

    ASSERT_TRUE(ns.has_value());
    EXPECT_EQ(ns->fields()[0][0], byte{0x10}); // copy so not equivalent anymore
}

TEST(track_namespace_test, rejects_empty_namespace_field) {
    std::array fields = {byte_view()};

    const auto name_space = track_namespace::make(fields);

    ASSERT_FALSE(name_space.has_value());
    EXPECT_EQ(name_space.error(), track_identity_error::empty_namespace_field);
}

TEST(track_namespace_test, accepts_thirty_two_namespace_fields) {
    constexpr std::array<byte, 1> field = {byte{0x01}};
    std::array<byte_view, track_namespace::max_field_count> fields{};
    fields.fill(byte_view(field));

    const auto name_space = track_namespace::make(fields);

    ASSERT_TRUE(name_space.has_value());
    EXPECT_EQ(name_space->fields().size(), track_namespace::max_field_count);
    EXPECT_EQ(name_space->byte_count(), track_namespace::max_field_count);
}

TEST(track_namespace_test, rejects_thirty_three_namespace_fields) {
    constexpr std::array field = {byte{0x01}};
    std::array<byte_view, track_namespace::max_field_count + 1> fields{};
    fields.fill(byte_view(field));

    const auto name_space = track_namespace::make(fields);

    ASSERT_FALSE(name_space.has_value());
    EXPECT_EQ(name_space.error(), track_identity_error::too_many_namespace_fields);
}

TEST(track_namespace_test, accepts_four_kib_namespace) {
    const std::vector field(track_namespace::max_byte_count, byte{0x01});
    const std::array fields = {as_view(field)};

    const auto name_space = track_namespace::make(fields);

    ASSERT_TRUE(name_space.has_value());
    EXPECT_EQ(name_space->byte_count(), track_namespace::max_byte_count);
}

TEST(track_namespace_test, rejects_namespace_over_four_kib) {
    const std::vector field(track_namespace::max_byte_count + 1, byte{0x01});
    const std::array fields = {as_view(field)};

    const auto name_space = track_namespace::make(fields);

    ASSERT_FALSE(name_space.has_value());
    EXPECT_EQ(name_space.error(), track_identity_error::namespace_too_large);
}

TEST(track_name_test, accepts_empty_track_name) {
    const auto name = track_name::make(byte_view());

    ASSERT_TRUE(name.has_value());
    EXPECT_TRUE(name->bytes().empty());
    EXPECT_EQ(name->byte_count(), 0U);
}

TEST(track_name_test, owns_copied_bytes) {
    std::array source = {byte{0x10}};

    const auto name = track_name::make(byte_view(source));
    source[0] = byte{0x20};

    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(name->bytes()[0], byte{0x10});
}

TEST(track_name_test, equality_and_ordering_use_exact_bytes) {
    constexpr std::array low_bytes = {byte{0x01}};
    constexpr std::array high_bytes = {byte{0x02}};

    const auto low_name = track_name::make(byte_view(low_bytes));
    const auto same_low_name = track_name::make(byte_view(low_bytes));
    const auto high_name = track_name::make(byte_view(high_bytes));

    ASSERT_TRUE(low_name.has_value());
    ASSERT_TRUE(same_low_name.has_value());
    ASSERT_TRUE(high_name.has_value());
    EXPECT_EQ(*low_name, *same_low_name);
    EXPECT_LT(*low_name, *high_name);
}

TEST(track_namespace_test, equality_and_ordering_use_exact_bytes) {
    constexpr std::array low = {byte{0x01}};
    constexpr std::array high = {byte{0x02}};
    const std::array low_fields = {byte_view(low)};
    const std::array high_fields = {byte_view(high)};

    const auto low_namespace = track_namespace::make(low_fields);
    const auto same_low_namespace = track_namespace::make(low_fields);
    const auto high_namespace = track_namespace::make(high_fields);

    ASSERT_TRUE(low_namespace.has_value());
    ASSERT_TRUE(same_low_namespace.has_value());
    ASSERT_TRUE(high_namespace.has_value());
    EXPECT_EQ(*low_namespace, *same_low_namespace);
    EXPECT_LT(*low_namespace, *high_namespace);
}

TEST(full_track_name_test, exposes_namespace_name_and_byte_count) {
    constexpr std::array field = {byte{0x01}};
    constexpr std::array name_bytes = {byte{0x02}, byte{0x03}};
    const std::array fields = {byte_view(field)};
    const auto name_space = track_namespace::make(fields);
    const auto name = track_name::make(byte_view(name_bytes));

    ASSERT_TRUE(name_space.has_value());
    ASSERT_TRUE(name.has_value());
    const auto full_name = full_track_name::make(*name_space, *name);

    ASSERT_TRUE(full_name.has_value());
    EXPECT_EQ(full_name->name_space(), *name_space);
    EXPECT_EQ(full_name->name(), *name);
    EXPECT_EQ(full_name->byte_count(), 3U);
}

TEST(full_track_name_test, accepts_four_kib_full_track_name) {
    const std::vector name_bytes(full_track_name::max_byte_count, byte{0x01});
    const std::array<byte_view, 0> fields = {};
    const auto name_space = track_namespace::make(fields);
    const auto name = track_name::make(as_view(name_bytes));

    ASSERT_TRUE(name_space.has_value());
    ASSERT_TRUE(name.has_value());
    const auto full_name = full_track_name::make(*name_space, *name);

    ASSERT_TRUE(full_name.has_value());
    EXPECT_EQ(full_name->byte_count(), full_track_name::max_byte_count);
}

TEST(full_track_name_test, rejects_full_track_name_over_four_kib) {
    constexpr std::array field = {byte{0x01}};
    const std::vector name_bytes(full_track_name::max_byte_count, byte{0x02});
    const std::array fields = {byte_view(field)};
    const auto name_space = track_namespace::make(fields);
    const auto name = track_name::make(as_view(name_bytes));

    ASSERT_TRUE(name_space.has_value());
    ASSERT_TRUE(name.has_value());
    const auto full_name = full_track_name::make(*name_space, *name);

    ASSERT_FALSE(full_name.has_value());
    EXPECT_EQ(full_name.error(), track_identity_error::full_track_name_too_large);
}

TEST(full_track_name_test, equality_and_ordering_use_namespace_then_name_bytes) {
    constexpr std::array namespace_bytes = {byte{0x01}};
    constexpr std::array low_name_bytes = {byte{0x01}};
    constexpr std::array high_name_bytes = {byte{0x02}};
    const std::array fields = {byte_view(namespace_bytes)};

    const auto name_space = track_namespace::make(fields);
    const auto low_name = track_name::make(byte_view(low_name_bytes));
    const auto same_low_name = track_name::make(byte_view(low_name_bytes));
    const auto high_name = track_name::make(byte_view(high_name_bytes));

    ASSERT_TRUE(name_space.has_value());
    ASSERT_TRUE(low_name.has_value());
    ASSERT_TRUE(same_low_name.has_value());
    ASSERT_TRUE(high_name.has_value());

    const auto low_full_name = full_track_name::make(*name_space, *low_name);
    const auto same_low_full_name = full_track_name::make(*name_space, *same_low_name);
    const auto high_full_name = full_track_name::make(*name_space, *high_name);

    ASSERT_TRUE(low_full_name.has_value());
    ASSERT_TRUE(same_low_full_name.has_value());
    ASSERT_TRUE(high_full_name.has_value());
    EXPECT_EQ(*low_full_name, *same_low_full_name);
    EXPECT_LT(*low_full_name, *high_full_name);
}

} // namespace
} // namespace mqxx::moqt
