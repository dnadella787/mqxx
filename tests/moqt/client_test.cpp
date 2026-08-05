#include "mqxx/moqt/client.hpp"

#include <gtest/gtest.h>

namespace mqxx::moqt {
namespace {

TEST(moq_client_test, sends_setup_over_native_quic_with_authority_and_path) {
    moq_client client(uri{"moqt", "example.com", "/moqt"}, false);

    const auto result = client.client_send_moq_setup();

    EXPECT_TRUE(result.has_value());
}

TEST(moq_client_test, second_send_fails_because_control_plane_is_retained) {
    moq_client client(uri{"moqt", "example.com", "/moqt"}, false);
    ASSERT_TRUE(client.client_send_moq_setup().has_value());

    const auto second = client.client_send_moq_setup();

    ASSERT_FALSE(second.has_value());
    EXPECT_EQ(second.error(), setup_error::already_sent);
}

TEST(moq_client_test, empty_authority_is_rejected) {
    moq_client client(uri{"moqt", "", "/moqt"}, false);

    const auto result = client.client_send_moq_setup();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), setup_error::invalid_authority);
}

TEST(moq_client_test, web_transport_client_is_rejected_via_build_setup_safety_net) {
    moq_client client(uri{"https", "example.com", "/moqt"}, true);

    const auto result = client.client_send_moq_setup();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), setup_error::invalid_authority);
}

TEST(moq_client_test, empty_path_over_native_quic_is_accepted) {
    moq_client client(uri{"moqt", "example.com", ""}, false);

    const auto result = client.client_send_moq_setup();

    EXPECT_TRUE(result.has_value());
}

} // namespace
} // namespace mqxx::moqt