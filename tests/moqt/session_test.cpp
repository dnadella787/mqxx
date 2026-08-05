#include "mqxx/moqt/session.hpp"
#include "mqxx/moqt/wire.hpp"

#include <gtest/gtest.h>

namespace mqxx::moqt {
namespace {

TEST(build_setup_test, defaults_produce_the_four_unconditional_options) {
    const auto result = build_setup(endpoint_role::server, false);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->params.size(), 4U);
}

TEST(build_setup_test, rejects_empty_authority) {
    const auto result =
        build_setup(endpoint_role::client, false, std::string{});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), setup_error::invalid_authority);
}

TEST(build_setup_test, rejects_authority_from_server_role) {
    const auto result =
        build_setup(endpoint_role::server, false, std::string{"example.com"});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), setup_error::invalid_authority);
}

TEST(build_setup_test, rejects_authority_over_web_transport) {
    const auto result = build_setup(
        endpoint_role::client, true, std::string{"example.com"});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), setup_error::invalid_authority);
}

TEST(build_setup_test, accepts_empty_path_for_native_quic_client) {
    const auto result = build_setup(
        endpoint_role::client, false, std::nullopt, std::string{});

    ASSERT_TRUE(result.has_value());

    const auto path_value = result->get_value_if_exists(setup_key::path);
    ASSERT_TRUE(std::holds_alternative<std::string>(path_value));
    EXPECT_EQ(std::get<std::string>(path_value), "");
}

TEST(build_setup_test, rejects_path_from_server_role) {
    const auto result =
        build_setup(endpoint_role::server, false, std::nullopt, std::string{"/moqt"});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), setup_error::invalid_path);
}

TEST(build_setup_test, rejects_path_over_web_transport) {
    const auto result =
        build_setup(endpoint_role::client, true, std::nullopt, std::string{"/moqt"});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), setup_error::invalid_path);
}

TEST(build_setup_test, rejects_empty_auth_token) {
    const auto result = build_setup(
        endpoint_role::client, false, std::nullopt, std::nullopt, 0, byte_buffer{});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), setup_error::invalid_auth_token);
}

TEST(build_setup_test, accepts_non_empty_auth_token) {
    const byte_buffer token{std::byte{0x01}, std::byte{0x02}};
    const auto result = build_setup(
        endpoint_role::client, false, std::nullopt, std::nullopt, 0, token);

    ASSERT_TRUE(result.has_value());
    const auto token_value = result->get_value_if_exists(setup_key::auth_token);
    ASSERT_TRUE(std::holds_alternative<std::string>(token_value));
}

TEST(build_setup_test, all_optionals_present_still_encode_in_ascending_type_order) {
    const auto result = build_setup(
        endpoint_role::client,
        false,
        std::string{"example.com"},
        std::string{"/moqt"},
        4096,
        byte_buffer{std::byte{0xAB}},
        "mqxx/0.1",
        5,
        7);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->params.size(), 7U);

    const auto encoded = encode_key_value_pairs(result->params);
    ASSERT_TRUE(encoded.has_value());
}

TEST(build_setup_test, round_trips_through_encode_setup_and_decode_setup) {
    const auto built = build_setup(
        endpoint_role::client, false, std::string{"example.com"}, std::string{"/moqt"});
    ASSERT_TRUE(built.has_value());

    const auto encoded = encode_setup(built->params);
    ASSERT_TRUE(encoded.has_value());

    const auto decoded = decode_setup(*encoded);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(*decoded, built->params);
}

TEST(setup_test, get_value_if_exists_returns_monostate_when_absent) {
    const setup empty_setup;

    const auto result = empty_setup.get_value_if_exists(setup_key::authority);

    EXPECT_TRUE(std::holds_alternative<std::monostate>(result));
}

TEST(setup_test, get_value_if_exists_returns_integer_for_even_typed_key) {
    const setup with_max{
        {key_value_pair{static_cast<std::uint64_t>(setup_key::max_filter_ranges),
                        std::uint64_t{9}}}};

    const auto result = with_max.get_value_if_exists(setup_key::max_filter_ranges);

    ASSERT_TRUE(std::holds_alternative<std::uint64_t>(result));
    EXPECT_EQ(std::get<std::uint64_t>(result), 9U);
}

TEST(setup_test, get_value_if_exists_returns_string_for_odd_typed_key) {
    const setup with_path{
        {key_value_pair{static_cast<std::uint64_t>(setup_key::path),
                        byte_buffer{std::byte{'/'}, std::byte{'a'}}}}};

    const auto result = with_path.get_value_if_exists(setup_key::path);

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "/a");
}

TEST(control_plane_test, send_setup_succeeds_once) {
    control_plane plane(endpoint_role::client, false);
    const auto built = build_setup(endpoint_role::client, false);
    ASSERT_TRUE(built.has_value());

    const auto result = plane.send_setup(*built);

    EXPECT_TRUE(result.has_value());
}

TEST(control_plane_test, send_setup_rejects_second_send) {
    control_plane plane(endpoint_role::client, false);
    const auto built = build_setup(endpoint_role::client, false);
    ASSERT_TRUE(built.has_value());
    ASSERT_TRUE(plane.send_setup(*built).has_value());

    const auto second = plane.send_setup(*built);

    ASSERT_FALSE(second.has_value());
    EXPECT_EQ(second.error(), setup_error::already_sent);
}

TEST(control_plane_test, send_setup_reports_encode_error_for_oversized_value) {
    control_plane plane(endpoint_role::client, false);
    const setup oversized{{key_value_pair{1U, byte_buffer(65536, std::byte{0x00})}}};

    const auto result = plane.send_setup(oversized);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), setup_error::encode_error);
}

TEST(control_plane_test, handle_peer_setup_decodes_and_marks_inbound_received) {
    control_plane plane(endpoint_role::server, false);
    const auto peer_setup = build_setup(endpoint_role::client, false);
    ASSERT_TRUE(peer_setup.has_value());
    const auto encoded = encode_setup(peer_setup->params);
    ASSERT_TRUE(encoded.has_value());
    plane.setup_bytes = *encoded;

    const auto result = plane.handle_peer_setup();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->params, peer_setup->params);
}

TEST(control_plane_test, handle_peer_setup_rejects_second_receive) {
    control_plane plane(endpoint_role::server, false);
    const auto peer_setup = build_setup(endpoint_role::client, false);
    ASSERT_TRUE(peer_setup.has_value());
    const auto encoded = encode_setup(peer_setup->params);
    ASSERT_TRUE(encoded.has_value());
    plane.setup_bytes = *encoded;
    ASSERT_TRUE(plane.handle_peer_setup().has_value());

    const auto second = plane.handle_peer_setup();

    ASSERT_FALSE(second.has_value());
    EXPECT_EQ(second.error(), setup_error::already_received);
}

TEST(control_plane_test, handle_peer_setup_reports_decode_error_for_malformed_bytes) {
    control_plane plane(endpoint_role::server, false);
    plane.setup_bytes = byte_buffer{std::byte{0x01}, std::byte{0x00}, std::byte{0x00}};

    const auto result = plane.handle_peer_setup();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), setup_error::decode_error);
}

TEST(control_plane_test, client_rejects_authority_in_peer_setup) {
    control_plane plane(endpoint_role::client, false);
    const setup malicious_server_setup{
        {key_value_pair{static_cast<std::uint64_t>(setup_key::authority),
                        byte_buffer{std::byte{'x'}}}}};
    const auto encoded = encode_setup(malicious_server_setup.params);
    ASSERT_TRUE(encoded.has_value());
    plane.setup_bytes = *encoded;

    const auto result = plane.handle_peer_setup();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), setup_error::invalid_authority);
}

TEST(control_plane_test, server_accepts_authority_in_peer_setup) {
    control_plane plane(endpoint_role::server, false);
    const auto peer_setup =
        build_setup(endpoint_role::client, false, std::string{"example.com"});
    ASSERT_TRUE(peer_setup.has_value());
    const auto encoded = encode_setup(peer_setup->params);
    ASSERT_TRUE(encoded.has_value());
    plane.setup_bytes = *encoded;

    const auto result = plane.handle_peer_setup();

    ASSERT_TRUE(result.has_value());
}

TEST(control_plane_test, client_send_goaway_rejects_non_empty_uri) {
    control_plane plane(endpoint_role::client, false);

    const auto result = plane.send_goaway(std::string{"moqt://elsewhere.example"});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), goaway_error::protocol_violation);
}

TEST(control_plane_test, client_send_goaway_with_no_uri_succeeds) {
    control_plane plane(endpoint_role::client, false);

    const auto result = plane.send_goaway();

    EXPECT_TRUE(result.has_value());
}

TEST(control_plane_test, server_send_goaway_with_uri_within_limit_succeeds) {
    control_plane plane(endpoint_role::server, false);

    const auto result = plane.send_goaway(std::string{"moqt://elsewhere.example"}, 5000U);

    EXPECT_TRUE(result.has_value());
}

TEST(control_plane_test, server_send_goaway_rejects_uri_over_max_length) {
    control_plane plane(endpoint_role::server, false);
    const std::string oversized_uri(8193, 'a');

    const auto result = plane.send_goaway(oversized_uri);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), goaway_error::new_session_uri_too_large);
}

TEST(control_plane_test, send_goaway_rejects_second_send) {
    control_plane plane(endpoint_role::client, false);
    ASSERT_TRUE(plane.send_goaway().has_value());

    const auto second = plane.send_goaway();

    ASSERT_FALSE(second.has_value());
    EXPECT_EQ(second.error(), goaway_error::already_sent);
}

TEST(control_plane_test, receive_goaway_returns_new_session_uri) {
    control_plane plane(endpoint_role::client, false);
    const auto encoded = encode_goaway(goaway{"moqt://elsewhere.example", 2000});
    ASSERT_TRUE(encoded.has_value());

    const auto result = plane.receive_goaway(*encoded);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "moqt://elsewhere.example");
}

TEST(control_plane_test, server_receiving_non_empty_uri_is_protocol_violation) {
    control_plane plane(endpoint_role::server, false);
    const auto encoded = encode_goaway(goaway{"moqt://elsewhere.example", 0});
    ASSERT_TRUE(encoded.has_value());

    const auto result = plane.receive_goaway(*encoded);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), goaway_error::protocol_violation);
}

TEST(control_plane_test, receive_goaway_rejects_second_receive) {
    control_plane plane(endpoint_role::client, false);
    const auto encoded = encode_goaway(goaway{});
    ASSERT_TRUE(encoded.has_value());
    ASSERT_TRUE(plane.receive_goaway(*encoded).has_value());

    const auto second = plane.receive_goaway(*encoded);

    ASSERT_FALSE(second.has_value());
    EXPECT_EQ(second.error(), goaway_error::already_received);
}

TEST(control_plane_test, receive_goaway_reports_decode_error_for_malformed_bytes) {
    control_plane plane(endpoint_role::client, false);
    const byte_buffer malformed{std::byte{0x01}, std::byte{0x00}, std::byte{0x00}};

    const auto result = plane.receive_goaway(malformed);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), goaway_error::decode_error);
}

} // namespace
} // namespace mqxx::moqt