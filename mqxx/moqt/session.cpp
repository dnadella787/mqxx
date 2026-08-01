#include "mqxx/moqt/session.hpp"

namespace mqxx {

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

const key_value_pair& setup::get_param_if_exists(setup_key key) {
    for (const auto& param : this->params) {
        if (param.type == static_cast<std::uint64_t>(key)) {
            return &param;
        }
    }

    return nullptr;
}

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
    std::uint64_t max_request_updates = 0) {

    if (authority && (role == endpoint_role::server || web_transport || authority->empty())) {
        return std::unexpected(setup_error::invalid_authority);
    }
    if (path && (role == endpoint_role::server || web_transport || path->empty())) {
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
    // local_setup is a client-created vector of KVPs for setup options = params
    // version negotiation is done on ALPN, prior to the setup message

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
control_plane::await_setup() {
    // 0x2F00 arrives on a unidirectional stream
    auto server_setup = decode_setup(server_setup_bytes);
    if (!server_setup.has_value()) {
        return std::unexpected(setup_error::decode_error);
    }

    // validate setup options
    if (server_setup.get_param_if_exists(setup_key::AUTHORITY)) {
        return std::unexpected(setup_error::invalid_authority);
    }
    if (server_setup.get_param_if_exists(setup_key::PATH)) {
        return std::unexpected(setup_error::invalid_path);
    }

    inbound_state_ = control_stream_state::setup_received;

    return std::move(server_setup);
}

std::expected<setup, setup_error>
control_plane::send_and_await_setup(const setup& local_setup) {
    // orchestrate send and await + co await logic
}

} // namespace