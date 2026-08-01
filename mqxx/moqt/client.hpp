#pragma once

#include "mqxx/moqt/session.hpp"

#include <future>
#include <expected>
#include <cstdint>

namespace mqxx::moqt {

struct url {
    std::string scheme;
    std::string authority; // should be broken into host/port later
    std::string path;

    // query and fragment are intentionally excluded for now

};

class moq_client {
    public:
        [[nodiscard]] std::expected<setup, setup_error>
        client_send_moq_setup(); // extract url data, send to build_setup and then send setup

    private:
        url client_url_;
        // leave maxes at default for now
        std::uint64_t max_auth_token_cache_size_ = 0;
        std::uint64_t max_filter_ranges_ = 0,
        std::uint64_t max_request_updates_ = 0

};

} // namespace mqxx::moqt