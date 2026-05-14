#pragma once

#include "mqxx/transport/session.hpp"

#include <deque>
#include <vector>

namespace mqxx::transport {

class fake_session final : public session {
  public:
    explicit fake_session(bool datagram_support = true);

    [[nodiscard]] bool supports_datagrams() const noexcept override;
    [[nodiscard]] open_uni_stream_result open_uni_stream() override;
    session_write_result send_control(byte_view bytes) override;
    session_write_result send_uni_stream(stream_id stream_id, byte_view bytes, bool fin) override;
    [[nodiscard]] datagram_send_result send_datagram(byte_view bytes) override;
    [[nodiscard]] std::optional<session_event> poll_event() override;

    void push_event(session_event event);
    void close_for_sends();

    [[nodiscard]] const std::vector<control_write>& control_writes() const noexcept;
    [[nodiscard]] const std::vector<stream_write>& stream_writes() const noexcept;
    [[nodiscard]] const std::vector<datagram_write>& datagram_writes() const noexcept;

  private:
    [[nodiscard]] session_write_result write_closed() const;

    bool datagram_support_{true};
    bool send_closed_{false};
    std::uint64_t next_stream_id_{0};
    std::vector<control_write> control_writes_;
    std::vector<stream_write> stream_writes_;
    std::vector<datagram_write> datagram_writes_;
    std::deque<session_event> pending_events_;
};

} // namespace mqxx::transport
