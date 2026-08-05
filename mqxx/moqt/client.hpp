#pragma once

#include "mqxx/moqt/session.hpp"

#include <cstdint>
#include <expected>
#include <utility>

namespace mqxx::moqt {

struct uri {
    std::string scheme;
    std::string authority; // should be broken into host/port later
    std::string path;

    // query and fragment are intentionally excluded for now
};

class moq_client {
    public:
        explicit moq_client(uri client_uri, bool web_transport = false)
            : client_uri_(std::move(client_uri)),
              web_transport_(web_transport),
              control_plane_(endpoint_role::client, web_transport) {}

        [[nodiscard]] std::expected<void, setup_error>
        client_send_moq_setup(); // extract uri data, send to build_setup and then send setup

    private:
        uri client_uri_;
        bool web_transport_ = false;
        // leave maxes at default for now
        std::uint64_t max_auth_token_cache_size_ = 0;
        std::uint64_t max_filter_ranges_ = 0;
        std::uint64_t max_request_updates_ = 0;

        control_plane control_plane_;
};

} // namespace mqxx::moqt