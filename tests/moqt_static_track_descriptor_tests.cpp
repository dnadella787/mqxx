import mqxx.moqt.static_track_descriptor;
import mqxx.test.framework;

#define SP_TEST(name)                                                                              \
    static void name();                                                                            \
    static const ::mqxx::test::test_registration name##_registration{#name, &name};                 \
    static void name()

#define SP_EXPECT(expression)                                                                      \
    ::mqxx::test::expect_true((expression), #expression, __FILE__, __LINE__)

#define SP_EXPECT_EQ(left, right)                                                                  \
    ::mqxx::test::expect_equal((left), (right), #left, #right, __FILE__, __LINE__)

using mqxx::moqt::render_serialized_full_track_name;
using mqxx::moqt::static_track_descriptor;

namespace {

using camera_track = static_track_descriptor<"camera_hd", "example.com", "meeting_7">;

} // namespace

SP_TEST(static_track_descriptor_exposes_compile_time_shape) {
    static_assert(camera_track::namespace_field_count == 2U);
    constexpr auto fields = camera_track::namespace_fields();

    SP_EXPECT_EQ(fields[0], "example.com");
    SP_EXPECT_EQ(fields[1], "meeting_7");
    SP_EXPECT_EQ(camera_track::track_name(), "camera_hd");
}

SP_TEST(static_track_descriptor_builds_runtime_name) {
    const auto runtime_name = camera_track::make_runtime_name();
    SP_EXPECT_EQ(render_serialized_full_track_name(runtime_name),
                 "example.2ecom-meeting_7--camera_hd");
}

#undef SP_EXPECT_EQ
#undef SP_EXPECT
#undef SP_TEST
