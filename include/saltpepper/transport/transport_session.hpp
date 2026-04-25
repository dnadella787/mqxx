#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace saltpepper::transport {

struct StreamId {
    std::uint64_t value{0};

    auto operator==(const StreamId& other) const -> bool = default;
};

class TransportSession {
  public:
    virtual ~TransportSession() = default;

    [[nodiscard]] virtual auto supports_datagrams() const noexcept -> bool = 0;
    [[nodiscard]] virtual auto open_uni_stream() -> StreamId = 0;
    virtual void write_stream(StreamId stream_id, std::span<const std::uint8_t> bytes,
                              bool fin) = 0;
    virtual void send_datagram(std::span<const std::uint8_t> bytes) = 0;
};

struct StreamWrite {
    StreamId stream_id;
    std::vector<std::uint8_t> bytes;
    bool fin{false};
};

struct DatagramWrite {
    std::vector<std::uint8_t> bytes;
};

} // namespace saltpepper::transport
