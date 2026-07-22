// #pragma once

// #include "mqxx/moqt/wire.hpp"

// #include <cstdint>
// #include <expected>
// #include <vector>

// namespace mqxx::moqt {

// enum class control_stream_state : std::uint8_t {
//     not_yet_open,
//     setup_sent,
//     setup_received,
//     open,
// };

// enum class setup_error : std::uint8_t {
//     already_sent,
//     placeholder_error,
// };

// struct setup {
//     std::vector<key_value_pair> params;
// };

// class control_plane {
//     public:
//         [[nodiscard]] std::expected<void, setup_error>
//         send_setup(setup local_setup);
//         [[nodiscard]] std::expected<setup_result, setup_error>
//         await_setup();

//         // [[nodiscard]] go_away();

        

//     private:
//         explicit control_plane(endpoint_role role);

//         endpoint_role role_;

//         control_stream_state outbound_state_{control_stream_state::not_yet_open};
//         control_stream_state inbound_state_{control_stream_state::not_yet_open};
// };

// };