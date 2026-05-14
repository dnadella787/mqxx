#include "mqxx/moqt/namespace_registry.hpp"

#include <gtest/gtest.h>

#include <string_view>

using mqxx::moqt::byte_string;
using mqxx::moqt::full_track_name;
using mqxx::moqt::namespace_registry;
using mqxx::moqt::render_serialized_full_track_name;
using mqxx::moqt::track_namespace;

namespace {

byte_string bytes(std::string_view text) {
    return byte_string{text.begin(), text.end()};
}

full_track_name make_track(std::string_view namespace_a, std::string_view namespace_b,
                           std::string_view track_name) {
    return full_track_name{
        .track_namespace = track_namespace{bytes(namespace_a), bytes(namespace_b)},
        .track_name = bytes(track_name),
    };
}

} // namespace

TEST(MoqtNamespaceRegistryTest, DeduplicatesTracks) {
    namespace_registry registry;
    const auto track = make_track("example.com", "meeting_1", "camera");

    EXPECT_TRUE(registry.announce_track(track));
    EXPECT_FALSE(registry.announce_track(track));
}

TEST(MoqtNamespaceRegistryTest, ReturnsMatchingPrefixesInDeterministicOrder) {
    namespace_registry registry;

    EXPECT_TRUE(registry.announce_track(make_track("example.com", "meeting_1", "audio")));
    EXPECT_TRUE(registry.announce_track(make_track("example.com", "meeting_1", "camera")));
    EXPECT_TRUE(registry.announce_track(make_track("example.com", "meeting_2", "camera")));

    const auto matches =
        registry.tracks_with_prefix(track_namespace{bytes("example.com"), bytes("meeting_1")});

    ASSERT_EQ(matches.size(), 2U);
    EXPECT_EQ(render_serialized_full_track_name(matches[0]), "example.2ecom-meeting_1--audio");
    EXPECT_EQ(render_serialized_full_track_name(matches[1]), "example.2ecom-meeting_1--camera");
}

TEST(MoqtNamespaceRegistryTest, WithdrawRemovesTrack) {
    namespace_registry registry;
    const auto track = make_track("example.com", "meeting_1", "camera");

    EXPECT_TRUE(registry.announce_track(track));
    EXPECT_TRUE(registry.withdraw_track(track));
    EXPECT_FALSE(registry.withdraw_track(track));
    EXPECT_EQ(registry.tracks_with_prefix(track_namespace{bytes("example.com"), bytes("meeting_1")})
                  .size(),
              0U);
}
