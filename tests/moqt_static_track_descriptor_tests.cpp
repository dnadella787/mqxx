#include "saltpepper/moqt/static_track_descriptor.hpp"
#include "test_framework.hpp"

using saltpepper::moqt::render_serialized_full_track_name;
using saltpepper::moqt::StaticTrackDescriptor;

namespace {

using CameraTrack = StaticTrackDescriptor<"camera_hd", "example.com", "meeting_7">;

} // namespace

SP_TEST(static_track_descriptor_exposes_compile_time_shape) {
    static_assert(CameraTrack::namespace_field_count == 2U);
    constexpr auto fields = CameraTrack::namespace_fields();

    SP_EXPECT_EQ(fields[0], "example.com");
    SP_EXPECT_EQ(fields[1], "meeting_7");
    SP_EXPECT_EQ(CameraTrack::track_name(), "camera_hd");
}

SP_TEST(static_track_descriptor_builds_runtime_name) {
    const auto runtime_name = CameraTrack::make_runtime_name();
    SP_EXPECT_EQ(render_serialized_full_track_name(runtime_name),
                 "example.2ecom-meeting_7--camera_hd");
}
