#include "mqxx/moqt/server.hpp"

#include <gtest/gtest.h>

namespace mqxx::moqt {
namespace {

TEST(moq_server_test, sends_setup_successfully) {
    moq_server server;

    const auto result = server.server_send_moq_setup();

    EXPECT_TRUE(result.has_value());
}

TEST(moq_server_test, second_send_fails_because_control_plane_is_retained) {
    moq_server server;
    ASSERT_TRUE(server.server_send_moq_setup().has_value());

    const auto second = server.server_send_moq_setup();

    ASSERT_FALSE(second.has_value());
    EXPECT_EQ(second.error(), setup_error::already_sent);
}

TEST(moq_server_test, web_transport_flag_does_not_affect_server_send) {
    moq_server server(true);

    const auto result = server.server_send_moq_setup();

    EXPECT_TRUE(result.has_value());
}

} // namespace
} // namespace mqxx::moqt