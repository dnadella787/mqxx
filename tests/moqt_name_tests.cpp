#include "saltpepper/moqt/full_track_name.hpp"
#include "test_framework.hpp"

using saltpepper::moqt::ByteString;
using saltpepper::moqt::FullTrackName;
using saltpepper::moqt::NameParseError;
using saltpepper::moqt::parse_serialized_full_track_name;
using saltpepper::moqt::render_serialized_full_track_name;
using saltpepper::moqt::render_serialized_name;
using saltpepper::moqt::TrackNamespace;

namespace {

auto bytes(std::string_view text) -> ByteString {
    return ByteString{text.begin(), text.end()};
}

} // namespace

SP_TEST(render_serialized_name_leaves_safe_ascii_bytes_unescaped) {
    SP_EXPECT_EQ(render_serialized_name(bytes("camera_01")), "camera_01");
}

SP_TEST(render_serialized_name_escapes_separator_and_binary_bytes) {
    const ByteString value{'a', '-', 0xffU};
    SP_EXPECT_EQ(render_serialized_name(value), "a.2d.ff");
}

SP_TEST(render_and_parse_round_trip_for_full_track_name) {
    const FullTrackName original{
        .track_namespace = TrackNamespace{bytes("example.com"), bytes("meeting_7")},
        .track_name = bytes("camera-main"),
    };

    const std::string rendered = render_serialized_full_track_name(original);
    const auto parsed = parse_serialized_full_track_name(rendered);

    SP_EXPECT(parsed.ok());
    SP_EXPECT_EQ(parsed.value.value(), original);
}

SP_TEST(parse_allows_empty_namespace_and_non_empty_track_name) {
    const auto parsed = parse_serialized_full_track_name("--catalog");

    SP_EXPECT(parsed.ok());
    SP_EXPECT_EQ(parsed.value->track_namespace.size(), 0U);
    SP_EXPECT_EQ(parsed.value->track_name, bytes("catalog"));
}

SP_TEST(parse_rejects_missing_track_separator) {
    const auto parsed = parse_serialized_full_track_name("example-camera");

    SP_EXPECT(!parsed.ok());
    SP_EXPECT_EQ(parsed.error, NameParseError::missing_track_separator);
}

SP_TEST(parse_rejects_uppercase_hex_escapes) {
    const auto parsed = parse_serialized_full_track_name("example--camera.2Dmain");

    SP_EXPECT(!parsed.ok());
    SP_EXPECT_EQ(parsed.error, NameParseError::uppercase_hex_escape);
}

SP_TEST(parse_rejects_redundant_hex_escapes) {
    const auto parsed = parse_serialized_full_track_name("example--.61udio");

    SP_EXPECT(!parsed.ok());
    SP_EXPECT_EQ(parsed.error, NameParseError::redundant_escape);
}

SP_TEST(parse_rejects_empty_namespace_fields) {
    const auto parsed = parse_serialized_full_track_name("-example--track");

    SP_EXPECT(!parsed.ok());
    SP_EXPECT_EQ(parsed.error, NameParseError::empty_namespace_field);
}
