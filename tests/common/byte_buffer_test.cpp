#include "mqxx/common/byte_buffer.hpp"

#include <array>

#include <gtest/gtest.h>

namespace mqxx {
namespace {

TEST(byte_buffer_test, default_constructed_view_is_empty) {
    const byte_buffer_view view;

    EXPECT_TRUE(view.empty());
}

TEST(byte_buffer_test, preserves_byte_count_and_ordering) {
    constexpr std::array<std::byte, 3> bytes = {std::byte{0x01}, std::byte{0x7f}, std::byte{0xff}};
    const byte_buffer_view view(bytes);

    ASSERT_EQ(view.size(), 3U);
    EXPECT_EQ(view[0], std::byte{0x01});
    EXPECT_EQ(view[1], std::byte{0x7f});
    EXPECT_EQ(view[2], std::byte{0xff});
}

TEST(byte_buffer_test, preserves_embedded_zero_bytes) {
    constexpr std::array<std::byte, 3> bytes = {std::byte{0x00}, std::byte{0x12}, std::byte{0x00}};
    const byte_buffer_view view(bytes);

    ASSERT_EQ(view.size(), 3U);
    EXPECT_EQ(view[0], std::byte{0x00});
    EXPECT_EQ(view[1], std::byte{0x12});
    EXPECT_EQ(view[2], std::byte{0x00});
}

TEST(byte_buffer_test, subview_references_expected_range) {
    constexpr std::array<std::byte, 4> bytes = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30},
                                                std::byte{0x40}};
    const byte_buffer_view view(bytes);
    const byte_buffer_view subview = view.subspan(1, 2);

    ASSERT_EQ(subview.size(), 2U);
    EXPECT_EQ(subview[0], std::byte{0x20});
    EXPECT_EQ(subview[1], std::byte{0x30});
}

} // namespace
} // namespace mqxx
