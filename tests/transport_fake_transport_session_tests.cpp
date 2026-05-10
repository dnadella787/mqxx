#include "mqxx/transport/fake_transport_session.hpp"
#include "test_framework.hpp"

#include <initializer_list>
#include <vector>

#define SP_TEST(name)                                                                              \
    static void name();                                                                            \
    static const ::mqxx::test::test_registration name##_registration{#name, &name};                 \
    static void name()

#define SP_EXPECT(expression)                                                                      \
    ::mqxx::test::expect_true((expression), #expression, __FILE__, __LINE__)

#define SP_EXPECT_EQ(left, right)                                                                  \
    ::mqxx::test::expect_equal((left), (right), #left, #right, __FILE__, __LINE__)

using mqxx::transport::datagram_send_error;
using mqxx::transport::fake_transport_session;
using mqxx::transport::stream_id;

namespace {

auto payload(std::initializer_list<std::uint8_t> bytes) -> std::vector<std::uint8_t> {
    return std::vector<std::uint8_t>{bytes};
}

} // namespace

SP_TEST(fake_transport_records_stream_writes) {
    fake_transport_session transport;
    const stream_id stream_id = transport.open_uni_stream();
    const auto bytes = payload({0x01U, 0x02U, 0x03U});

    transport.write_stream(stream_id, bytes, true);

    SP_EXPECT_EQ(stream_id.value, 0U);
    SP_EXPECT_EQ(transport.stream_writes().size(), 1U);
    SP_EXPECT_EQ(transport.stream_writes()[0].bytes, bytes);
    SP_EXPECT(transport.stream_writes()[0].fin);
}

SP_TEST(fake_transport_records_datagrams) {
    fake_transport_session transport(true);
    const auto bytes = payload({0xaaU, 0xbbU});

    const auto send_result = transport.send_datagram(bytes);

    SP_EXPECT(send_result.ok());
    SP_EXPECT_EQ(transport.datagram_writes().size(), 1U);
    SP_EXPECT_EQ(transport.datagram_writes()[0].bytes, bytes);
}

SP_TEST(fake_transport_rejects_datagrams_when_disabled) {
    fake_transport_session transport(false);
    const auto send_result = transport.send_datagram(payload({0x10U}));

    SP_EXPECT(!send_result.ok());
    SP_EXPECT_EQ(send_result.error(), datagram_send_error::datagrams_not_supported);
}

#undef SP_EXPECT_EQ
#undef SP_EXPECT
#undef SP_TEST
