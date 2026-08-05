#include "mqxx/moqt/server.hpp"

namespace mqxx::moqt {

std::expected<void, setup_error>
moq_server::server_send_moq_setup() {
    auto server_setup = build_setup(
        endpoint_role::server,
        web_transport_,
        std::nullopt,
        std::nullopt,
        max_auth_token_cache_size_,
        std::nullopt,
        "moqt-18",
        max_filter_ranges_,
        max_request_updates_);
    if (!server_setup.has_value()) {
        return std::unexpected(server_setup.error());
    }

    return control_plane_.send_setup(*server_setup);
}

} // namespace mqxx::moqt