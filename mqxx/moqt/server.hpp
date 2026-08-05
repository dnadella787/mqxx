// TODO: server side handling of receipt of SETUP and base server class

#pragma once

#include "mqxx/moqt/session.hpp"

#include <cstdint>
#include <expected>

namespace mqxx::moqt {

class moq_server {
    public:
        explicit moq_server(bool web_transport = false)
            : web_transport_(web_transport),
              control_plane_(endpoint_role::server, web_transport) {}

        [[nodiscard]] std::expected<void, setup_error>
        server_send_moq_setup();

    private:
        bool web_transport_ = false;
        std::uint64_t max_auth_token_cache_size_ = 0;
        std::uint64_t max_filter_ranges_ = 0;
        std::uint64_t max_request_updates_ = 0;

        control_plane control_plane_;
};

} // namespace mqxx::moqt