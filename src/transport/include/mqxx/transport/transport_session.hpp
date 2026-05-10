#pragma once

#include "mqxx/common/byte_buffer.hpp"
#include "mqxx/common/result.hpp"

#include <cstdint>

namespace mqxx::transport {

struct stream_id {
    std::uint64_t value{0};

    auto operator==(const stream_id& other) const -> bool = default;
};

enum class datagram_send_error {
    datagrams_not_supported,
};

using datagram_send_result = common::result<common::unit, datagram_send_error>;

struct stream_write {
    stream_id stream_id;
    common::byte_string bytes;
    bool fin{false};
};

struct datagram_write {
    common::byte_string bytes;
};

class transport_session {
  public:
    virtual ~transport_session() = default;

    [[nodiscard]] virtual auto supports_datagrams() const noexcept -> bool = 0;
    [[nodiscard]] virtual auto open_uni_stream() -> stream_id = 0;
    virtual void write_stream(stream_id stream_id, common::byte_view bytes, bool fin) = 0;
    [[nodiscard]] virtual auto send_datagram(common::byte_view bytes) -> datagram_send_result = 0;
};

} // namespace mqxx::transport
