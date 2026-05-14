#pragma once

#include "mqxx/common/byte_buffer.hpp"
#include "mqxx/common/result.hpp"

#include <cstdint>
#include <optional>
#include <variant>

namespace mqxx::transport {

struct stream_id {
    std::uint64_t value{0};

    bool operator==(const stream_id& other) const = default;
};

enum class open_uni_stream_error {
    session_closed,
};

enum class session_write_error {
    session_closed,
};

enum class datagram_send_error {
    datagrams_not_supported,
    session_closed,
};

using open_uni_stream_result = result<stream_id, open_uni_stream_error>;
using session_write_result = result<unit, session_write_error>;
using datagram_send_result = result<unit, datagram_send_error>;

struct control_write {
    byte_string bytes;

    bool operator==(const control_write& other) const = default;
};

struct stream_write {
    stream_id stream_id;
    byte_string bytes;
    bool fin{false};

    bool operator==(const stream_write& other) const = default;
};

struct datagram_write {
    byte_string bytes;

    bool operator==(const datagram_write& other) const = default;
};

struct inbound_control_message {
    byte_string bytes;

    bool operator==(const inbound_control_message& other) const = default;
};

struct inbound_uni_stream_data {
    stream_id stream_id;
    byte_string bytes;
    bool fin{false};

    bool operator==(const inbound_uni_stream_data& other) const = default;
};

struct inbound_datagram {
    byte_string bytes;

    bool operator==(const inbound_datagram& other) const = default;
};

struct stream_reset_signal {
    stream_id stream_id;
    std::uint64_t error_code{0};
    bool peer_initiated{true};

    bool operator==(const stream_reset_signal& other) const = default;
};

struct session_shutdown_signal {
    std::uint64_t error_code{0};
    bool peer_initiated{true};

    bool operator==(const session_shutdown_signal& other) const = default;
};

struct flow_control_update {
    stream_id stream_id;
    std::uint64_t available_credit{0};

    bool operator==(const flow_control_update& other) const = default;
};

struct delivery_notification {
    stream_id stream_id;
    std::uint64_t delivered_bytes{0};
    bool fin_delivered{false};

    bool operator==(const delivery_notification& other) const = default;
};

using session_event = std::variant<inbound_control_message, inbound_uni_stream_data,
                                   inbound_datagram, stream_reset_signal,
                                   session_shutdown_signal, flow_control_update,
                                   delivery_notification>;

class session {
  public:
    virtual ~session() = default;

    [[nodiscard]] virtual bool supports_datagrams() const noexcept = 0;
    [[nodiscard]] virtual open_uni_stream_result open_uni_stream() = 0;
    virtual session_write_result send_control(byte_view bytes) = 0;
    virtual session_write_result send_uni_stream(stream_id stream_id, byte_view bytes, bool fin) = 0;
    [[nodiscard]] virtual datagram_send_result send_datagram(byte_view bytes) = 0;
    [[nodiscard]] virtual std::optional<session_event> poll_event() = 0;
};

} // namespace mqxx::transport
