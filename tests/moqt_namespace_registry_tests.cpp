import mqxx.moqt.namespace_registry;
import mqxx.test.framework;

#include <string_view>

#define SP_TEST(name)                                                                              \
    static void name();                                                                            \
    static const ::mqxx::test::test_registration name##_registration{#name, &name};                 \
    static void name()

#define SP_EXPECT(expression)                                                                      \
    ::mqxx::test::expect_true((expression), #expression, __FILE__, __LINE__)

#define SP_EXPECT_EQ(left, right)                                                                  \
    ::mqxx::test::expect_equal((left), (right), #left, #right, __FILE__, __LINE__)

using mqxx::moqt::byte_string;
using mqxx::moqt::full_track_name;
using mqxx::moqt::namespace_registry;
using mqxx::moqt::render_serialized_full_track_name;
using mqxx::moqt::track_namespace;

namespace {

auto bytes(std::string_view text) -> byte_string {
    return byte_string{text.begin(), text.end()};
}

auto make_track(std::string_view namespace_a, std::string_view namespace_b,
                std::string_view track_name) -> full_track_name {
    return full_track_name{
        .track_namespace = track_namespace{bytes(namespace_a), bytes(namespace_b)},
        .track_name = bytes(track_name),
    };
}

} // namespace

SP_TEST(namespace_registry_deduplicates_tracks) {
    namespace_registry registry;
    const auto track = make_track("example.com", "meeting_1", "camera");

    SP_EXPECT(registry.announce_track(track));
    SP_EXPECT(!registry.announce_track(track));
}

SP_TEST(namespace_registry_returns_matching_prefixes_in_deterministic_order) {
    namespace_registry registry;

    SP_EXPECT(registry.announce_track(make_track("example.com", "meeting_1", "audio")));
    SP_EXPECT(registry.announce_track(make_track("example.com", "meeting_1", "camera")));
    SP_EXPECT(registry.announce_track(make_track("example.com", "meeting_2", "camera")));

    const auto matches =
        registry.tracks_with_prefix(track_namespace{bytes("example.com"), bytes("meeting_1")});

    SP_EXPECT_EQ(matches.size(), 2U);
    SP_EXPECT_EQ(render_serialized_full_track_name(matches[0]), "example.2ecom-meeting_1--audio");
    SP_EXPECT_EQ(render_serialized_full_track_name(matches[1]), "example.2ecom-meeting_1--camera");
}

SP_TEST(namespace_registry_withdraw_removes_track) {
    namespace_registry registry;
    const auto track = make_track("example.com", "meeting_1", "camera");

    SP_EXPECT(registry.announce_track(track));
    SP_EXPECT(registry.withdraw_track(track));
    SP_EXPECT(!registry.withdraw_track(track));
    SP_EXPECT_EQ(
        registry.tracks_with_prefix(track_namespace{bytes("example.com"), bytes("meeting_1")})
            .size(),
        0U);
}

#undef SP_EXPECT_EQ
#undef SP_EXPECT
#undef SP_TEST
