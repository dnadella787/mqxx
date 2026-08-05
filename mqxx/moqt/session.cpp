#include "mqxx/moqt/session.hpp"

namespace mqxx::moqt {

namespace {

byte_buffer string_to_bytes(const std::string& str) {
    byte_buffer ret;
    ret.reserve(str.length());
    std::transform(std::begin(str), std::end(str), std::back_inserter(ret), [](char c) {
        return std::byte(c);
    });

    return ret;
}

} // namespace

std::variant<std::monostate, std::uint64_t, std::string>
setup::get_value_if_exists(setup_key key) const {
    auto it = std::find_if(
        params.begin(), params.end(),
        [key](const key_value_pair& kvp) {
            return kvp.type == static_cast<std::uint64_t>(key);
        });

    if (it == params.end()) {
        return std::monostate{};
    }

    const auto& val = it->value;

    if (const auto* n = std::get_if<std::uint64_t>(&val)) {
        return *n;
    }
    const auto& buf = std::get<byte_buffer>(val);
    return std::string(buf.begin(), buf.end());
}


[[nodiscard]] std::expected<setup, setup_error>
build_setup(
    endpoint_role role,
    bool web_transport,
    std::optional<std::string> authority,
    std::optional<std::string> path,
    std::uint64_t max_auth_token_cache_size,
    std::optional<byte_buffer> auth_token,
    std::string moqt_implementation,
    std::uint64_t max_filter_ranges,
    std::uint64_t max_request_updates) {

    if (authority && (role == endpoint_role::server || web_transport || authority->empty())) {
        return std::unexpected(setup_error::invalid_authority);
    }
    if (path && (role == endpoint_role::server || web_transport)) {
        return std::unexpected(setup_error::invalid_path);
    }
    if (auth_token && auth_token->empty()) {
        return std::unexpected(setup_error::invalid_auth_token);
    }

    setup result;
    result.params.reserve(4 + (path ? 1 : 0) + (auth_token ? 1 : 0) + (authority ? 1 : 0));

    // insert KVPs in least to greatest setup_keys
    if (path) { 
        result.params.push_back(key_value_pair{static_cast<std::uint64_t>(setup_key::path), string_to_bytes(*path)});
    }
    if (auth_token) {
        result.params.push_back(key_value_pair{static_cast<std::uint64_t>(setup_key::auth_token), *auth_token});
    }
    result.params.push_back(key_value_pair{static_cast<std::uint64_t>(setup_key::max_auth_token_cache_size), max_auth_token_cache_size});
    if (authority) {
        result.params.push_back(key_value_pair{static_cast<std::uint64_t>(setup_key::authority), string_to_bytes(*authority)});
    }
    result.params.push_back(key_value_pair{static_cast<std::uint64_t>(setup_key::max_filter_ranges), max_filter_ranges});
    result.params.push_back(key_value_pair{static_cast<std::uint64_t>(setup_key::moqt_implementation), string_to_bytes(moqt_implementation)});
    result.params.push_back(key_value_pair{static_cast<std::uint64_t>(setup_key::max_request_updates), max_request_updates});

    return result;
}


std::expected<void, setup_error>
control_plane::send_setup(const setup& local_setup) {
    // this should be packaged in a function await_peer_setup() that listens for peer setup

    if (outbound_state_ == control_stream_state::setup_sent) {
        return std::unexpected(setup_error::already_sent);
    }

    // encode setup and record error
    // encoding returns the ready-to-send setup [TYPE][LENGTH][PAYLOAD]
    auto encoded_setup = encode_setup(local_setup.params);
    if (!encoded_setup.has_value()) {
        // TODO: does this degrade the wire error in encode?
        return std::unexpected(setup_error::encode_error);
    }
    
    // hand off to control stream writer
    // passBytesToStream(*encoded_setup);

    outbound_state_ = control_stream_state::setup_sent;

    return {};
}

std::expected<setup, setup_error>
control_plane::handle_peer_setup() {
    if (inbound_state_ == control_stream_state::setup_received) {
        return std::unexpected(setup_error::already_received);
    }
 
    auto decoded_params = decode_setup(setup_bytes);
    if (!decoded_params.has_value()) {
        return std::unexpected(setup_error::decode_error);
    }
 
    setup received{std::move(*decoded_params)};

    if (role_ == endpoint_role::client) {
        if ((received.get_value_if_exists(setup_key::authority)).index() != 0) {
            return std::unexpected(setup_error::invalid_authority);
        }
        if ((received.get_value_if_exists(setup_key::path)).index() != 0) {
            return std::unexpected(setup_error::invalid_path);
        }
    }
 
    inbound_state_ = control_stream_state::setup_received;
 
    return received;
}

std::expected<void, goaway_error>
control_plane::send_goaway(
    std::optional<std::string> local_new_uri,
    std::optional<std::uint64_t> local_timeout) {
    
    if (outbound_state_ == control_stream_state::goaway_sent) {
        return std::unexpected(goaway_error::already_sent);
    }

    goaway local_goaway;
    if (role_ == endpoint_role::client && local_new_uri.has_value() && !local_new_uri->empty()) {
        return std::unexpected(goaway_error::protocol_violation);
    }
    else if (local_new_uri.has_value()) {
        if (local_new_uri->size() > (1U << 13)) {
            return std::unexpected(goaway_error::new_session_uri_too_large);
        }
        local_goaway.new_session_uri = *local_new_uri;
    }
    
    if (local_timeout.has_value()) {
        local_goaway.timeout = *local_timeout;
    }

    auto encoded_goaway = encode_goaway(local_goaway);
    if (!encoded_goaway.has_value()) {
        return std::unexpected(goaway_error::encode_error);
    }

    // hand off to control stream writer
    // passBytesToStream(*encoded_goaway);
    // startTimer(*local_timeout);

    outbound_state_ = control_stream_state::goaway_sent;

    return {};
}

std::expected<std::string, goaway_error>
control_plane::receive_goaway(std::span<const std::byte> goaway_bytes) {
    if (inbound_state_ == control_stream_state::goaway_received) {
        return std::unexpected(goaway_error::already_received);
    }

    auto decoded_goaway = decode_goaway(goaway_bytes);
    if (!decoded_goaway.has_value()) {
        return std::unexpected(goaway_error::decode_error);
    }

    if (role_ == endpoint_role::server && decoded_goaway->new_session_uri.size() > 0) {
        return std::unexpected(goaway_error::protocol_violation);
    }

    // startTimer(*decoded_goaway->timeout)
    // drain requests

    inbound_state_ = control_stream_state::goaway_received;

    return decoded_goaway->new_session_uri;
}

} // namespace mqxx::moqt