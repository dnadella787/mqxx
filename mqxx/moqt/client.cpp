#include "mqxx/moqt/client.hpp"

namespace mqxx {

[[nodiscard]] std::expected<setup, setup_error>
moq_client::client_send_moq_setup() {
    auto client_setup = build_setup(
        role = endpoint_role::client,
        web_transport = false, // defer WebTransport logic to ALPN handling
        authority = client_url_.authority,
        path = client_url_.path,
        max_auth_token_cache_size = max_auth_token_cache_size_,
        max_filter_ranges = max_filter_ranges_,
        max_request_updates = max_request_updates_
        // moqt_implementation handled in function as default
    );
    if (!client_setup.has_value()) {
        return std::unexpected(client_setup.error())
    }

    control_plane moq_control_plane = control_plane(endpoint_role::client, false);

}

} // namespace mqxx