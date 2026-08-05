#pragma once
 
#include "mqxx/moqt/wire.hpp"
 
#include <algorithm>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace mqxx::moqt {

enum class control_stream_state : std::uint8_t {
    not_yet_open,
    setup_sent,
    setup_received,
    open,
    goaway_sent,
    goaway_received,
};

enum class endpoint_role : std::uint8_t {
    client,
    server,
};

// in-order (10.3.1.x)
enum class setup_key : std::uint64_t {
    path = 0x01U,
    auth_token = 0x03U,
    max_auth_token_cache_size = 0x04U,
    authority = 0x05U,
    max_filter_ranges = 0x06U,
    moqt_implementation = 0x07U,
    max_request_updates = 0x08U,
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
    invalid_auth_token,
    already_received
};

enum class goaway_error : std::uint8_t {
    protocol_violation,
    new_session_uri_too_large,
    encode_error,
    decode_error,
    already_sent,
    already_received,
};

struct setup {
    std::vector<key_value_pair> params;
    std::variant<std::monostate, std::uint64_t, std::string>
    get_value_if_exists(setup_key key) const;
};

// SETUP no longer divided into CLIENT/SERVER_SETUP (A.3)
[[nodiscard]] std::expected<setup, setup_error>
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
        handle_peer_setup();

        [[nodiscard]] std::expected<void, goaway_error>
        send_goaway(
            std::optional<std::string> local_new_uri = std::nullopt,
            std::optional<std::uint64_t> local_timeout = std::nullopt);
        [[nodiscard]] std::expected<std::string, goaway_error>
        receive_goaway(std::span<const std::byte> goaway_bytes);

        byte_buffer setup_bytes; // testing placeholder

    private:
        endpoint_role role_;
        bool web_transport_;

        control_stream_state outbound_state_{control_stream_state::not_yet_open};
        control_stream_state inbound_state_{control_stream_state::not_yet_open};
};

} // namespace mqxx::moqt