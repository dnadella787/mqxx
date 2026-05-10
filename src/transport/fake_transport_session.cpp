#include "mqxx/transport/fake_transport_session.hpp"

namespace mqxx::transport {

fake_transport_session::fake_transport_session(bool datagram_support)
    : datagram_support_(datagram_support) {}

auto fake_transport_session::supports_datagrams() const noexcept -> bool {
    return datagram_support_;
}

auto fake_transport_session::open_uni_stream() -> stream_id {
    return stream_id{.value = next_stream_id_++};
}

void fake_transport_session::write_stream(const stream_id stream_id,
                                          const common::byte_view bytes, const bool fin) {
    stream_writes_.push_back(stream_write{
        .stream_id = stream_id,
        .bytes = common::byte_string{bytes.begin(), bytes.end()},
        .fin = fin,
    });
}

auto fake_transport_session::send_datagram(const common::byte_view bytes) -> datagram_send_result {
    if (!datagram_support_) {
        return datagram_send_result::failure(datagram_send_error::datagrams_not_supported);
    }

    datagram_writes_.push_back(datagram_write{
        .bytes = common::byte_string{bytes.begin(), bytes.end()},
    });

    return datagram_send_result::success(common::unit{});
}

auto fake_transport_session::stream_writes() const noexcept -> const std::vector<stream_write>& {
    return stream_writes_;
}

auto fake_transport_session::datagram_writes() const noexcept
    -> const std::vector<datagram_write>& {
    return datagram_writes_;
}

} // namespace mqxx::transport
