#include "mqxx/transport/fake_session.hpp"

namespace mqxx::transport {

fake_session::fake_session(bool datagram_support) : datagram_support_(datagram_support) {}

bool fake_session::supports_datagrams() const noexcept {
    return datagram_support_;
}

open_uni_stream_result fake_session::open_uni_stream() {
    if (send_closed_) {
        return open_uni_stream_result::failure(open_uni_stream_error::session_closed);
    }

    return open_uni_stream_result::success(stream_id{.value = next_stream_id_++});
}

session_write_result fake_session::send_control(const byte_view bytes) {
    if (send_closed_) {
        return write_closed();
    }

    control_writes_.push_back(control_write{
        .bytes = byte_string{bytes.begin(), bytes.end()},
    });

    return session_write_result::success(unit{});
}

session_write_result fake_session::send_uni_stream(const stream_id stream_id, const byte_view bytes,
                                                   const bool fin) {
    if (send_closed_) {
        return write_closed();
    }

    stream_writes_.push_back(stream_write{
        .stream_id = stream_id,
        .bytes = byte_string{bytes.begin(), bytes.end()},
        .fin = fin,
    });

    return session_write_result::success(unit{});
}

datagram_send_result fake_session::send_datagram(const byte_view bytes) {
    if (!datagram_support_) {
        return datagram_send_result::failure(datagram_send_error::datagrams_not_supported);
    }

    if (send_closed_) {
        return datagram_send_result::failure(datagram_send_error::session_closed);
    }

    datagram_writes_.push_back(datagram_write{
        .bytes = byte_string{bytes.begin(), bytes.end()},
    });

    return datagram_send_result::success(unit{});
}

std::optional<session_event> fake_session::poll_event() {
    if (pending_events_.empty()) {
        return std::nullopt;
    }

    session_event event = pending_events_.front();
    pending_events_.pop_front();
    return event;
}

void fake_session::push_event(session_event event) {
    pending_events_.push_back(std::move(event));
}

void fake_session::close_for_sends() {
    send_closed_ = true;
}

const std::vector<control_write>& fake_session::control_writes() const noexcept {
    return control_writes_;
}

const std::vector<stream_write>& fake_session::stream_writes() const noexcept {
    return stream_writes_;
}

const std::vector<datagram_write>& fake_session::datagram_writes() const noexcept {
    return datagram_writes_;
}

session_write_result fake_session::write_closed() const {
    return session_write_result::failure(session_write_error::session_closed);
}

} // namespace mqxx::transport
