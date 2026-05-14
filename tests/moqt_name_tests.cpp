#include "mqxx/moqt/full_track_name.hpp"

#include <gtest/gtest.h>

#include <string_view>

using mqxx::moqt::byte_string;
using mqxx::moqt::full_track_name;
using mqxx::moqt::name_parse_error;
using mqxx::moqt::parse_serialized_full_track_name;
using mqxx::moqt::render_serialized_full_track_name;
using mqxx::moqt::render_serialized_name;
using mqxx::moqt::track_namespace;

namespace {

byte_string bytes(std::string_view text) {
    return byte_string{text.begin(), text.end()};
}

} // namespace

TEST(MoqtNameTest, RenderSerializedNameLeavesSafeAsciiBytesUnescaped) {
    EXPECT_EQ(render_serialized_name(bytes("camera_01")), "camera_01");
}

TEST(MoqtNameTest, RenderSerializedNameEscapesSeparatorAndBinaryBytes) {
    const byte_string value{'a', '-', 0xffU};
    EXPECT_EQ(render_serialized_name(value), "a.2d.ff");
}

TEST(MoqtNameTest, RenderAndParseRoundTripForFullTrackName) {
    const full_track_name original{
        .track_namespace = track_namespace{bytes("example.com"), bytes("meeting_7")},
        .track_name = bytes("camera-main"),
    };

    const std::string rendered = render_serialized_full_track_name(original);
    const auto parsed = parse_serialized_full_track_name(rendered);

    ASSERT_TRUE(parsed.ok());
    EXPECT_EQ(parsed.value(), original);
}

TEST(MoqtNameTest, ParseAllowsEmptyNamespaceAndNonEmptyTrackName) {
    const auto parsed = parse_serialized_full_track_name("--catalog");

    ASSERT_TRUE(parsed.ok());
    EXPECT_EQ(parsed.value().track_namespace.size(), 0U);
    EXPECT_EQ(parsed.value().track_name, bytes("catalog"));
}

TEST(MoqtNameTest, ParseRejectsMissingTrackSeparator) {
    const auto parsed = parse_serialized_full_track_name("example-camera");

    ASSERT_FALSE(parsed.ok());
    EXPECT_EQ(parsed.error(), name_parse_error::missing_track_separator);
}

TEST(MoqtNameTest, ParseRejectsUppercaseHexEscapes) {
    const auto parsed = parse_serialized_full_track_name("example--camera.2Dmain");

    ASSERT_FALSE(parsed.ok());
    EXPECT_EQ(parsed.error(), name_parse_error::uppercase_hex_escape);
}

TEST(MoqtNameTest, ParseRejectsRedundantHexEscapes) {
    const auto parsed = parse_serialized_full_track_name("example--.61udio");

    ASSERT_FALSE(parsed.ok());
    EXPECT_EQ(parsed.error(), name_parse_error::redundant_escape);
}

TEST(MoqtNameTest, ParseRejectsEmptyNamespaceFields) {
    const auto parsed = parse_serialized_full_track_name("-example--track");

    ASSERT_FALSE(parsed.ok());
    EXPECT_EQ(parsed.error(), name_parse_error::empty_namespace_field);
}
