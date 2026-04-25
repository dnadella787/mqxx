#include "saltpepper/transport/fake_transport_session.hpp"

#include <stdexcept>

namespace saltpepper::transport {

FakeTransportSession::FakeTransportSession(bool datagram_support)
    : datagram_support_(datagram_support) {}

auto FakeTransportSession::supports_datagrams() const noexcept -> bool {
    return datagram_support_;
}

auto FakeTransportSession::open_uni_stream() -> StreamId {
    return StreamId{.value = next_stream_id_++};
}

void FakeTransportSession::write_stream(const StreamId stream_id,
                                        std::span<const std::uint8_t> bytes, const bool fin) {
    stream_writes_.push_back(StreamWrite{
        .stream_id = stream_id,
        .bytes = std::vector<std::uint8_t>{bytes.begin(), bytes.end()},
        .fin = fin,
    });
}

void FakeTransportSession::send_datagram(std::span<const std::uint8_t> bytes) {
    if (!datagram_support_) {
        throw std::logic_error("transport session does not support datagrams");
    }

    datagram_writes_.push_back(DatagramWrite{
        .bytes = std::vector<std::uint8_t>{bytes.begin(), bytes.end()},
    });
}

auto FakeTransportSession::stream_writes() const noexcept -> const std::vector<StreamWrite>& {
    return stream_writes_;
}

auto FakeTransportSession::datagram_writes() const noexcept -> const std::vector<DatagramWrite>& {
    return datagram_writes_;
}

} // namespace saltpepper::transport
