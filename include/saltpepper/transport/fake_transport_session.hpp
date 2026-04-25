#pragma once

#include "saltpepper/transport/transport_session.hpp"

#include <vector>

namespace saltpepper::transport {

class FakeTransportSession final : public TransportSession {
  public:
    explicit FakeTransportSession(bool datagram_support = true);

    [[nodiscard]] auto supports_datagrams() const noexcept -> bool override;
    [[nodiscard]] auto open_uni_stream() -> StreamId override;
    void write_stream(StreamId stream_id, std::span<const std::uint8_t> bytes, bool fin) override;
    void send_datagram(std::span<const std::uint8_t> bytes) override;

    [[nodiscard]] auto stream_writes() const noexcept -> const std::vector<StreamWrite>&;
    [[nodiscard]] auto datagram_writes() const noexcept -> const std::vector<DatagramWrite>&;

  private:
    bool datagram_support_{true};
    std::uint64_t next_stream_id_{0};
    std::vector<StreamWrite> stream_writes_;
    std::vector<DatagramWrite> datagram_writes_;
};

} // namespace saltpepper::transport
