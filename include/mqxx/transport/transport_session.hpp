#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace mqxx::transport {

struct stream_id {
    std::uint64_t value{0};

    auto operator==(const stream_id& other) const -> bool = default;
};

class transport_session {
  public:
    virtual ~transport_session() = default;

    [[nodiscard]] virtual auto supports_datagrams() const noexcept -> bool = 0;
    [[nodiscard]] virtual auto open_uni_stream() -> stream_id = 0;
    virtual void write_stream(stream_id stream_id, std::span<const std::uint8_t> bytes,
                              bool fin) = 0;
    virtual void send_datagram(std::span<const std::uint8_t> bytes) = 0;
};

struct stream_write {
    stream_id stream_id;
    std::vector<std::uint8_t> bytes;
    bool fin{false};
};

struct datagram_write {
    std::vector<std::uint8_t> bytes;
};

} // namespace mqxx::transport
