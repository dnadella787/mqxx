#include "saltpepper/moqt/namespace_registry.hpp"
#include "test_framework.hpp"

using saltpepper::moqt::ByteString;
using saltpepper::moqt::FullTrackName;
using saltpepper::moqt::NamespaceRegistry;
using saltpepper::moqt::render_serialized_full_track_name;
using saltpepper::moqt::TrackNamespace;

namespace {

auto bytes(std::string_view text) -> ByteString {
    return ByteString{text.begin(), text.end()};
}

auto make_track(std::string_view namespace_a, std::string_view namespace_b,
                std::string_view track_name) -> FullTrackName {
    return FullTrackName{
        .track_namespace = TrackNamespace{bytes(namespace_a), bytes(namespace_b)},
        .track_name = bytes(track_name),
    };
}

} // namespace

SP_TEST(namespace_registry_deduplicates_tracks) {
    NamespaceRegistry registry;
    const auto track = make_track("example.com", "meeting_1", "camera");

    SP_EXPECT(registry.announce_track(track));
    SP_EXPECT(!registry.announce_track(track));
}

SP_TEST(namespace_registry_returns_matching_prefixes_in_deterministic_order) {
    NamespaceRegistry registry;

    SP_EXPECT(registry.announce_track(make_track("example.com", "meeting_1", "audio")));
    SP_EXPECT(registry.announce_track(make_track("example.com", "meeting_1", "camera")));
    SP_EXPECT(registry.announce_track(make_track("example.com", "meeting_2", "camera")));

    const auto matches =
        registry.tracks_with_prefix(TrackNamespace{bytes("example.com"), bytes("meeting_1")});

    SP_EXPECT_EQ(matches.size(), 2U);
    SP_EXPECT_EQ(render_serialized_full_track_name(matches[0]), "example.2ecom-meeting_1--audio");
    SP_EXPECT_EQ(render_serialized_full_track_name(matches[1]), "example.2ecom-meeting_1--camera");
}

SP_TEST(namespace_registry_withdraw_removes_track) {
    NamespaceRegistry registry;
    const auto track = make_track("example.com", "meeting_1", "camera");

    SP_EXPECT(registry.announce_track(track));
    SP_EXPECT(registry.withdraw_track(track));
    SP_EXPECT(!registry.withdraw_track(track));
    SP_EXPECT_EQ(
        registry.tracks_with_prefix(TrackNamespace{bytes("example.com"), bytes("meeting_1")})
            .size(),
        0U);
}
