#pragma once

#include "mqxx/transport/transport_session.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace mqxx::transport {

class fake_transport_session final : public transport_session {
  public:
    explicit fake_transport_session(bool datagram_support = true);

    [[nodiscard]] auto supports_datagrams() const noexcept -> bool override;
    [[nodiscard]] auto open_uni_stream() -> stream_id override;
    void write_stream(stream_id stream_id, std::span<const std::uint8_t> bytes, bool fin) override;
    void send_datagram(std::span<const std::uint8_t> bytes) override;

    [[nodiscard]] auto stream_writes() const noexcept -> const std::vector<stream_write>&;
    [[nodiscard]] auto datagram_writes() const noexcept -> const std::vector<datagram_write>&;

  private:
    bool datagram_support_{true};
    std::uint64_t next_stream_id_{0};
    std::vector<stream_write> stream_writes_;
    std::vector<datagram_write> datagram_writes_;
};

} // namespace mqxx::transport
