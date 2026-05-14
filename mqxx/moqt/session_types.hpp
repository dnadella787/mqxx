#pragma once

#include "mqxx/common/byte_buffer.hpp"
#include "mqxx/moqt/full_track_name.hpp"

#include <cstdint>
#include <optional>

namespace mqxx::moqt {

enum class group_order {
    ascending,
    descending,
};

struct object_location {
    std::uint64_t group_id{0};
    std::uint64_t subgroup_id{0};
    std::uint64_t object_id{0};

    bool operator==(const object_location& other) const = default;
};

struct subscription_range {
    std::optional<std::uint64_t> start_group{};
    std::optional<std::uint64_t> start_object{};
    std::optional<std::uint64_t> end_group{};
    std::optional<std::uint64_t> end_object{};

    bool operator==(const subscription_range& other) const = default;
};

struct subscription_request {
    full_track_name track;
    subscription_range range{};
    std::optional<std::uint8_t> subscriber_priority{};
    std::optional<group_order> group_ordering{};
    bool group_boundary_updates{true};
    bool end_of_track_updates{true};

    bool operator==(const subscription_request& other) const = default;
};

struct fetch_request {
    full_track_name track;
    subscription_range range{};

    bool operator==(const fetch_request& other) const = default;
};

struct publish_namespace_request {
    track_namespace published_namespace;

    bool operator==(const publish_namespace_request& other) const = default;
};

enum class track_status_code {
    active,
    ended,
    not_found,
    not_authorized,
    error,
};

struct track_status {
    full_track_name track;
    track_status_code code{track_status_code::active};
    std::optional<object_location> latest_object{};

    bool operator==(const track_status& other) const = default;
};

struct object_header {
    full_track_name track;
    object_location location{};
    std::uint8_t publisher_priority{0};
    bool end_of_group{false};
    bool end_of_track{false};

    bool operator==(const object_header& other) const = default;
};

struct object_message {
    object_header header;
    byte_string payload;

    bool operator==(const object_message& other) const = default;
};

struct group_boundary {
    full_track_name track;
    std::uint64_t group_id{0};

    bool operator==(const group_boundary& other) const = default;
};

struct subgroup_boundary {
    full_track_name track;
    std::uint64_t group_id{0};
    std::uint64_t subgroup_id{0};

    bool operator==(const subgroup_boundary& other) const = default;
};

enum class request_reset_code {
    cancelled,
    protocol_error,
    not_authorized,
    gone,
    transport_error,
    internal_error,
};

struct request_reset {
    request_reset_code code{request_reset_code::internal_error};
    std::optional<std::uint64_t> wire_code{};

    bool operator==(const request_reset& other) const = default;
};

} // namespace mqxx::moqt
