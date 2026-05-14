#include "mqxx/moqt/static_track_descriptor.hpp"

#include <gtest/gtest.h>

using mqxx::moqt::render_serialized_full_track_name;
using mqxx::moqt::static_track_descriptor;

namespace {

using camera_track = static_track_descriptor<"camera_hd", "example.com", "meeting_7">;

} // namespace

TEST(MoqtStaticTrackDescriptorTest, ExposesCompileTimeShape) {
    static_assert(camera_track::namespace_field_count == 2U);
    constexpr auto fields = camera_track::namespace_fields();

    EXPECT_EQ(fields[0], "example.com");
    EXPECT_EQ(fields[1], "meeting_7");
    EXPECT_EQ(camera_track::track_name(), "camera_hd");
}

TEST(MoqtStaticTrackDescriptorTest, BuildsRuntimeName) {
    const auto runtime_name = camera_track::make_runtime_name();
    EXPECT_EQ(render_serialized_full_track_name(runtime_name),
              "example.2ecom-meeting_7--camera_hd");
}
