// #pragma once

// #include <cstdint>
// #include <expected>

// namespace mqxx::moqt {

// enum class request_id_error : std::uint8_t {
//     wrong_parity,
//     duplicate_request
// };

// struct request_id {
//     std::uint64_t value{0};
//     constexpr bool operator==(const request_id&) const = default;
//     constexpr bool operator<=>(const request_id& other) const { return value < other.value; }
// };

// // need: client/server check to assign next id
// class request_id_tracker {
//     public:
//         [[nodiscard]] request_id allocate_local_id();
//         [[nodiscard]] std::expected<void, request_id_error>
//         process_peer_id(request_id id);
//     private:
//         // endpoint_role local_role;
//         request_id next_local_id = 0;
//         request_id next_expected_id = 0;
// };

// } // namespace mqxx::moqt