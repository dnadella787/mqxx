module;

#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

export module mqxx.transport.fake_transport_session;

export import mqxx.transport.session;

export namespace mqxx::transport {

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
                                          std::span<const std::uint8_t> bytes, const bool fin) {
    stream_writes_.push_back(stream_write{
        .stream_id = stream_id,
        .bytes = std::vector<std::uint8_t>{bytes.begin(), bytes.end()},
        .fin = fin,
    });
}

void fake_transport_session::send_datagram(std::span<const std::uint8_t> bytes) {
    if (!datagram_support_) {
        throw std::logic_error("transport session does not support datagrams");
    }

    datagram_writes_.push_back(datagram_write{
        .bytes = std::vector<std::uint8_t>{bytes.begin(), bytes.end()},
    });
}

auto fake_transport_session::stream_writes() const noexcept -> const std::vector<stream_write>& {
    return stream_writes_;
}

auto fake_transport_session::datagram_writes() const noexcept
    -> const std::vector<datagram_write>& {
    return datagram_writes_;
}

} // namespace mqxx::transport
