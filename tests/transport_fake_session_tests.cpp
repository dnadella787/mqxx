#include "mqxx/transport/fake_session.hpp"

#include <gtest/gtest.h>

#include <initializer_list>
#include <variant>
#include <vector>

using mqxx::transport::datagram_send_error;
using mqxx::transport::delivery_notification;
using mqxx::transport::fake_session;
using mqxx::transport::flow_control_update;
using mqxx::transport::inbound_control_message;
using mqxx::transport::inbound_datagram;
using mqxx::transport::inbound_uni_stream_data;
using mqxx::transport::open_uni_stream_error;
using mqxx::transport::session_shutdown_signal;
using mqxx::transport::session_write_error;
using mqxx::transport::stream_id;
using mqxx::transport::stream_reset_signal;

namespace {

std::vector<std::uint8_t> payload(std::initializer_list<std::uint8_t> bytes) {
    return std::vector<std::uint8_t>{bytes};
}

} // namespace

TEST(TransportFakeSessionTest, RecordsControlStreamAndDatagramWrites) {
    fake_session session;
    const auto stream = session.open_uni_stream();
    const auto control_bytes = payload({0x01U, 0x02U});
    const auto stream_bytes = payload({0x03U, 0x04U, 0x05U});
    const auto datagram_bytes = payload({0xaaU});

    ASSERT_TRUE(stream.ok());
    EXPECT_TRUE(session.send_control(control_bytes).ok());
    EXPECT_TRUE(session.send_uni_stream(stream.value(), stream_bytes, true).ok());
    EXPECT_TRUE(session.send_datagram(datagram_bytes).ok());

    EXPECT_EQ(stream.value().value, 0U);
    ASSERT_EQ(session.control_writes().size(), 1U);
    EXPECT_EQ(session.control_writes()[0].bytes, control_bytes);
    ASSERT_EQ(session.stream_writes().size(), 1U);
    EXPECT_EQ(session.stream_writes()[0].bytes, stream_bytes);
    EXPECT_TRUE(session.stream_writes()[0].fin);
    ASSERT_EQ(session.datagram_writes().size(), 1U);
    EXPECT_EQ(session.datagram_writes()[0].bytes, datagram_bytes);
}

TEST(TransportFakeSessionTest, SurfacesInboundEventsInFifoOrder) {
    fake_session session;
    session.push_event(inbound_control_message{.bytes = payload({0x10U})});
    session.push_event(inbound_uni_stream_data{
        .stream_id = stream_id{.value = 7U},
        .bytes = payload({0x11U, 0x12U}),
        .fin = true,
    });
    session.push_event(inbound_datagram{.bytes = payload({0x13U})});
    session.push_event(stream_reset_signal{
        .stream_id = stream_id{.value = 7U},
        .error_code = 99U,
        .peer_initiated = true,
    });
    session.push_event(session_shutdown_signal{
        .error_code = 123U,
        .peer_initiated = true,
    });
    session.push_event(flow_control_update{
        .stream_id = stream_id{.value = 7U},
        .available_credit = 4096U,
    });
    session.push_event(delivery_notification{
        .stream_id = stream_id{.value = 7U},
        .delivered_bytes = 2U,
        .fin_delivered = true,
    });

    const auto control = session.poll_event();
    const auto stream = session.poll_event();
    const auto datagram = session.poll_event();
    const auto reset = session.poll_event();
    const auto shutdown = session.poll_event();
    const auto flow = session.poll_event();
    const auto delivery = session.poll_event();

    ASSERT_TRUE(control.has_value());
    ASSERT_TRUE(stream.has_value());
    ASSERT_TRUE(datagram.has_value());
    ASSERT_TRUE(reset.has_value());
    ASSERT_TRUE(shutdown.has_value());
    ASSERT_TRUE(flow.has_value());
    ASSERT_TRUE(delivery.has_value());
    EXPECT_FALSE(session.poll_event().has_value());

    EXPECT_EQ(std::get<inbound_control_message>(*control).bytes, payload({0x10U}));
    EXPECT_EQ(std::get<inbound_uni_stream_data>(*stream).stream_id.value, 7U);
    EXPECT_EQ(std::get<inbound_datagram>(*datagram).bytes, payload({0x13U}));
    EXPECT_EQ(std::get<stream_reset_signal>(*reset).error_code, 99U);
    EXPECT_EQ(std::get<session_shutdown_signal>(*shutdown).error_code, 123U);
    EXPECT_EQ(std::get<flow_control_update>(*flow).available_credit, 4096U);
    EXPECT_EQ(std::get<delivery_notification>(*delivery).delivered_bytes, 2U);
}

TEST(TransportFakeSessionTest, RejectsSendsAfterClose) {
    fake_session session;
    session.close_for_sends();

    const auto stream = session.open_uni_stream();
    const auto control = session.send_control(payload({0x01U}));
    const auto write = session.send_uni_stream(stream_id{.value = 1U}, payload({0x02U}), false);
    const auto datagram = session.send_datagram(payload({0x03U}));

    ASSERT_FALSE(stream.ok());
    EXPECT_EQ(stream.error(), open_uni_stream_error::session_closed);
    ASSERT_FALSE(control.ok());
    EXPECT_EQ(control.error(), session_write_error::session_closed);
    ASSERT_FALSE(write.ok());
    EXPECT_EQ(write.error(), session_write_error::session_closed);
    ASSERT_FALSE(datagram.ok());
    EXPECT_EQ(datagram.error(), datagram_send_error::session_closed);
}

TEST(TransportFakeSessionTest, RejectsDatagramsWhenDisabled) {
    fake_session session(false);
    const auto send_result = session.send_datagram(payload({0x10U}));

    ASSERT_FALSE(send_result.ok());
    EXPECT_EQ(send_result.error(), datagram_send_error::datagrams_not_supported);
}
