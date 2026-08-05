#include "mqxx/moqt/client.hpp"

namespace mqxx::moqt {

std::expected<void, setup_error>
moq_client::client_send_moq_setup() {
    auto client_setup = build_setup(
        endpoint_role::client,
        web_transport_,
        client_uri_.authority,
        client_uri_.path,
        max_auth_token_cache_size_,
        std::nullopt,
        "moqt-18",
        max_filter_ranges_,
        max_request_updates_);
    if (!client_setup.has_value()) {
        return std::unexpected(client_setup.error());
    }

    return control_plane_.send_setup(*client_setup);
}

} // namespace mqxx::moqt