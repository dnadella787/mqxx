#pragma once

#include <future>
#include <expected>
#include <cstdint> 

namespace mqxx::moqt {

enum class setup_error : std::uint8_t {
    placeholder_error
};

class moq_client {
    public:
        struct setup_result {
            uint64_t negotiated_version;
        };

        [[nodiscard]] std::expected<void, setup_error>
        send_setup();
        [[nodiscard]] std::expected<setup_result, setup_error>
        await_setup();
};

} // namespace mqxx::moqt