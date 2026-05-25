#include "mqxx/common/byte_buffer.hpp"

#include <array>

#include <gtest/gtest.h>

namespace mqxx {
namespace {

TEST(byte_view_test, default_constructed_view_is_empty) {
    const byte_view view;

    EXPECT_TRUE(view.empty());
}

TEST(byte_view_test, preserves_byte_count_and_ordering) {
    constexpr std::array<byte, 3> bytes = {byte{0x01}, byte{0x7f}, byte{0xff}};
    const byte_view view(bytes);

    ASSERT_EQ(view.size(), 3U);
    EXPECT_EQ(view[0], byte{0x01});
    EXPECT_EQ(view[1], byte{0x7f});
    EXPECT_EQ(view[2], byte{0xff});
}

TEST(byte_view_test, preserves_embedded_zero_bytes) {
    constexpr std::array<byte, 3> bytes = {byte{0x00}, byte{0x12}, byte{0x00}};
    const byte_view view(bytes);

    ASSERT_EQ(view.size(), 3U);
    EXPECT_EQ(view[0], byte{0x00});
    EXPECT_EQ(view[1], byte{0x12});
    EXPECT_EQ(view[2], byte{0x00});
}

TEST(byte_view_test, subview_references_expected_range) {
    constexpr std::array<byte, 4> bytes = {byte{0x10}, byte{0x20}, byte{0x30}, byte{0x40}};
    const byte_view view(bytes);
    const byte_view subview = view.subspan(1, 2);

    ASSERT_EQ(subview.size(), 2U);
    EXPECT_EQ(subview[0], byte{0x20});
    EXPECT_EQ(subview[1], byte{0x30});
}

} // namespace
} // namespace mqxx
