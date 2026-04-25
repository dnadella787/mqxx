#include "saltpepper/transport/fake_transport_session.hpp"
#include "test_framework.hpp"

#include <stdexcept>

using saltpepper::transport::FakeTransportSession;
using saltpepper::transport::StreamId;

namespace {

auto payload(std::initializer_list<std::uint8_t> bytes) -> std::vector<std::uint8_t> {
    return std::vector<std::uint8_t>{bytes};
}

} // namespace

SP_TEST(fake_transport_records_stream_writes) {
    FakeTransportSession transport;
    const StreamId stream_id = transport.open_uni_stream();
    const auto bytes = payload({0x01U, 0x02U, 0x03U});

    transport.write_stream(stream_id, bytes, true);

    SP_EXPECT_EQ(stream_id.value, 0U);
    SP_EXPECT_EQ(transport.stream_writes().size(), 1U);
    SP_EXPECT_EQ(transport.stream_writes()[0].bytes, bytes);
    SP_EXPECT(transport.stream_writes()[0].fin);
}

SP_TEST(fake_transport_records_datagrams) {
    FakeTransportSession transport(true);
    const auto bytes = payload({0xaaU, 0xbbU});

    transport.send_datagram(bytes);

    SP_EXPECT_EQ(transport.datagram_writes().size(), 1U);
    SP_EXPECT_EQ(transport.datagram_writes()[0].bytes, bytes);
}

SP_TEST(fake_transport_rejects_datagrams_when_disabled) {
    FakeTransportSession transport(false);
    bool threw = false;

    try {
        transport.send_datagram(payload({0x10U}));
    } catch (const std::logic_error&) {
        threw = true;
    }

    SP_EXPECT(threw);
}
