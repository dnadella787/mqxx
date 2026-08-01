#pragma once

#include "mqxx/moqt/wire.hpp"

#include <cstdint>
#include <expected>
#include <vector>
#include <algorithm>

namespace mqxx::moqt {

enum class control_stream_state : std::uint8_t {
    not_yet_open,
    setup_sent,
    setup_received,
    open,
};

enum class endpoint_role {
    client,
    server,
}

// in-order (10.3.1.x)
enum class setup_key : std::uint64_t {
    AUTHORITY = 0x05U,
    PATH = 0x01U,
    MAX_AUTH_TOKEN_CACHE_SIZE = 0x04U,
    AUTH_TOKEN = 0x03U,
    MOQT_IMPLEMENTATION = 0x07U,
    MAX_FILTER_RANGES = 0x06U,
    MAX_REQUEST_UPDATES = 0x08U,
};

enum class setup_error : std::uint8_t {
    already_sent,
    already_open,
    encode_error,
    decode_error,
    invalid_authority,
    malformed_authority,
    invalid_path,
    malformed_path,
};

struct setup {
    std::vector<key_value_pair> params;
    std::optional<const key_value_pair&> get_param_if_exists(setup_key key);
};

// SETUP no longer divided into CLIENT/SERVER_SETUP (A.3)
[[nodiscard]] std::expected<setup, setup_builder_error>
build_setup(
    endpoint_role role,
    bool web_transport,
    std::optional<std::string> authority = std::nullopt,
    std::optional<std::string> path = std::nullopt,
    std::uint64_t max_auth_token_cache_size = 0,
    std::optional<byte_buffer> auth_token = std::nullopt,
    std::string moqt_implementation = "moqt-18",
    std::uint64_t max_filter_ranges = 0,
    std::uint64_t max_request_updates = 0
);

class control_plane {
    public:
        explicit control_plane(endpoint_role role, bool web_transport)
            : role_(role), web_transport_(web_transport) {};

        [[nodiscard]] std::expected<void, setup_error>
        send_setup(const setup& local_setup);
        [[nodiscard]] std::expected<setup, setup_error>
        await_setup();
        [[nodiscard]] std::expected<setup, setup_error>
        send_and_await_setup(const setup& local_setup);

        // [[nodiscard]] go_away();

    private:
        endpoint_role role_;
        bool web_transport_;
        byte_buffer server_setup_bytes; // placeholder

        control_stream_state outbound_state_{control_stream_state::not_yet_open};
        control_stream_state inbound_state_{control_stream_state::not_yet_open};
};

};