#include "mqxx/moqt/relay_seams.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

using mqxx::byte_string;
using mqxx::moqt::fetch_consumer;
using mqxx::moqt::fetch_handle;
using mqxx::moqt::fetch_request;
using mqxx::moqt::forwarding_decision;
using mqxx::moqt::forwarding_policy;
using mqxx::moqt::full_track_name;
using mqxx::moqt::group_boundary;
using mqxx::moqt::group_order;
using mqxx::moqt::object_location;
using mqxx::moqt::object_message;
using mqxx::moqt::publish_namespace_handle;
using mqxx::moqt::publish_namespace_request;
using mqxx::moqt::publisher;
using mqxx::moqt::relay_endpoint;
using mqxx::moqt::relay_object_cache;
using mqxx::moqt::request_reset;
using mqxx::moqt::request_reset_code;
using mqxx::moqt::subgroup_boundary;
using mqxx::moqt::subscriber;
using mqxx::moqt::subscription_aggregator;
using mqxx::moqt::subscription_handle;
using mqxx::moqt::subscription_request;
using mqxx::moqt::track_consumer;
using mqxx::moqt::track_namespace;
using mqxx::moqt::track_status;
using mqxx::moqt::track_status_code;

namespace {

byte_string bytes(std::string_view text) {
    return byte_string{text.begin(), text.end()};
}

full_track_name make_track(std::string_view namespace_a, std::string_view namespace_b,
                           std::string_view track_name) {
    return full_track_name{
        .track_namespace = track_namespace{bytes(namespace_a), bytes(namespace_b)},
        .track_name = bytes(track_name),
    };
}

object_message make_object(std::string_view track_name, std::uint64_t object_id) {
    return object_message{
        .header =
            {
                .track = make_track("example.com", "meeting_7", track_name),
                .location =
                    object_location{
                        .group_id = 4U,
                        .subgroup_id = 0U,
                        .object_id = object_id,
                    },
                .publisher_priority = 3U,
                .end_of_group = false,
                .end_of_track = false,
            },
        .payload = bytes("payload"),
    };
}

class recording_track_consumer final : public track_consumer {
  public:
    void on_object(const object_message& object) override {
        objects.push_back(object);
    }
    void on_group_boundary(const group_boundary& boundary) override {
        boundaries.push_back(boundary);
    }
    void on_end_of_track(const full_track_name& track) override {
        end_of_track = track;
    }
    void on_track_status(const track_status& status) override {
        statuses.push_back(status);
    }
    void on_reset(const request_reset& reset) override {
        resets.push_back(reset);
    }

    std::vector<object_message> objects;
    std::vector<group_boundary> boundaries;
    std::optional<full_track_name> end_of_track;
    std::vector<track_status> statuses;
    std::vector<request_reset> resets;
};

class recording_fetch_consumer final : public fetch_consumer {
  public:
    void on_object(const object_message& object) override {
        objects.push_back(object);
    }
    void on_end_of_fetch(const full_track_name& track) override {
        end_of_fetch = track;
    }
    void on_track_status(const track_status& status) override {
        statuses.push_back(status);
    }
    void on_reset(const request_reset& reset) override {
        resets.push_back(reset);
    }

    std::vector<object_message> objects;
    std::optional<full_track_name> end_of_fetch;
    std::vector<track_status> statuses;
    std::vector<request_reset> resets;
};

class recording_subscription_handle final : public subscription_handle {
  public:
    explicit recording_subscription_handle(subscription_request request)
        : current_request(std::move(request)) {}

    void subscribe_update(subscription_request request) override {
        current_request = std::move(request);
        ++update_calls;
    }

    void unsubscribe() override {
        unsubscribed = true;
    }

    subscription_request current_request;
    std::size_t update_calls{0};
    bool unsubscribed{false};
};

class recording_fetch_handle final : public fetch_handle {
  public:
    void cancel() override {
        cancelled = true;
    }

    bool cancelled{false};
};

class recording_publish_namespace_handle final : public publish_namespace_handle {
  public:
    explicit recording_publish_namespace_handle(track_namespace published_namespace)
        : published_namespace_(std::move(published_namespace)) {}

    [[nodiscard]] const track_namespace& published_namespace() const noexcept override {
        return published_namespace_;
    }

    void withdraw() override {
        withdrawn = true;
    }

    void publish_track_status(const track_status& status) override {
        statuses.push_back(status);
    }

    track_namespace published_namespace_;
    std::vector<track_status> statuses;
    bool withdrawn{false};
};

class recording_cache final : public relay_object_cache {
  public:
    void retain_object(const object_message& object) override {
        objects.push_back(object);
    }

    [[nodiscard]] std::vector<object_message>
    lookup(const subscription_request& request) const override {
        std::vector<object_message> matches;
        for (const object_message& object : objects) {
            if (object.header.track == request.track) {
                matches.push_back(object);
            }
        }
        return matches;
    }

    std::vector<object_message> objects;
};

class recording_aggregator final : public subscription_aggregator {
  public:
    [[nodiscard]] bool should_aggregate(const subscription_request&) const override {
        return aggregate;
    }

    void note_subscribe(const full_track_name& track) override {
        subscribed.push_back(track);
    }
    void note_unsubscribe(const full_track_name& track) override {
        unsubscribed.push_back(track);
    }

    bool aggregate{true};
    std::vector<full_track_name> subscribed;
    std::vector<full_track_name> unsubscribed;
};

class allow_all_policy final : public forwarding_policy {
  public:
    [[nodiscard]] forwarding_decision decide(const subscription_request&) const override {
        return forwarding_decision::satisfy_from_cache;
    }
};

class demo_relay final : public relay_endpoint {
  public:
    demo_relay(relay_object_cache& cache, subscription_aggregator& aggregator,
               forwarding_policy& policy)
        : cache_(cache), aggregator_(aggregator), policy_(policy) {}

    [[nodiscard]] mqxx::moqt::publish_namespace_result
    publish_namespace(publish_namespace_request request) override {
        auto handle =
            std::make_unique<recording_publish_namespace_handle>(request.published_namespace);
        last_namespace_handle = handle.get();
        return mqxx::moqt::publish_namespace_result::success(std::move(handle));
    }

    [[nodiscard]] mqxx::moqt::subscribe_result subscribe(subscription_request request,
                                                         track_consumer& consumer) override {
        aggregator_.note_subscribe(request.track);

        if (policy_.decide(request) == forwarding_decision::satisfy_from_cache) {
            const auto cached = cache_.lookup(request);
            for (const object_message& object : cached) {
                consumer.on_object(object);
            }
        }

        auto handle = std::make_unique<recording_subscription_handle>(request);
        last_subscription_handle = handle.get();
        return mqxx::moqt::subscribe_result::success(std::move(handle));
    }

    [[nodiscard]] mqxx::moqt::fetch_result fetch(fetch_request request,
                                                 fetch_consumer& consumer) override {
        const subscription_request equivalent_request{
            .track = request.track,
            .range = request.range,
            .subscriber_priority = std::nullopt,
            .group_ordering = std::nullopt,
            .group_boundary_updates = true,
            .end_of_track_updates = true,
        };

        for (const object_message& object : cache_.lookup(equivalent_request)) {
            consumer.on_object(object);
        }
        consumer.on_end_of_fetch(request.track);

        auto handle = std::make_unique<recording_fetch_handle>();
        last_fetch_handle = handle.get();
        return mqxx::moqt::fetch_result::success(std::move(handle));
    }

    relay_object_cache& cache_;
    subscription_aggregator& aggregator_;
    forwarding_policy& policy_;
    recording_subscription_handle* last_subscription_handle{nullptr};
    recording_fetch_handle* last_fetch_handle{nullptr};
    recording_publish_namespace_handle* last_namespace_handle{nullptr};
};

} // namespace

TEST(MoqtSessionRolesTest, RoleBasedRelaySurfaceSupportsPublishSubscribeAndFetchFlows) {
    recording_cache cache;
    recording_aggregator aggregator;
    allow_all_policy policy;
    demo_relay relay(cache, aggregator, policy);
    recording_track_consumer track_consumer;
    recording_fetch_consumer fetch_consumer;

    cache.retain_object(make_object("camera", 1U));
    cache.retain_object(make_object("camera", 2U));

    const subscription_request subscribe_request{
        .track = make_track("example.com", "meeting_7", "camera"),
        .range = {},
        .subscriber_priority = 7U,
        .group_ordering = group_order::ascending,
        .group_boundary_updates = true,
        .end_of_track_updates = true,
    };

    const auto subscribe_result = relay.subscribe(subscribe_request, track_consumer);
    ASSERT_TRUE(subscribe_result.ok());
    EXPECT_EQ(track_consumer.objects.size(), 2U);
    EXPECT_EQ(aggregator.subscribed.size(), 1U);

    subscribe_result.value()->subscribe_update(subscription_request{
        .track = subscribe_request.track,
        .range =
            {
                .start_group = 4U,
                .start_object = 1U,
                .end_group = std::nullopt,
                .end_object = std::nullopt,
            },
        .subscriber_priority = 5U,
        .group_ordering = group_order::descending,
        .group_boundary_updates = false,
        .end_of_track_updates = true,
    });
    subscribe_result.value()->unsubscribe();

    ASSERT_NE(relay.last_subscription_handle, nullptr);
    EXPECT_EQ(relay.last_subscription_handle->update_calls, 1U);
    EXPECT_TRUE(relay.last_subscription_handle->unsubscribed);

    const auto fetch_result =
        relay.fetch(fetch_request{.track = subscribe_request.track}, fetch_consumer);
    ASSERT_TRUE(fetch_result.ok());
    fetch_result.value()->cancel();
    EXPECT_EQ(fetch_consumer.objects.size(), 2U);
    EXPECT_TRUE(fetch_consumer.end_of_fetch.has_value());
    ASSERT_NE(relay.last_fetch_handle, nullptr);
    EXPECT_TRUE(relay.last_fetch_handle->cancelled);

    const auto publish_result = relay.publish_namespace(
        publish_namespace_request{.published_namespace = subscribe_request.track.track_namespace});
    ASSERT_TRUE(publish_result.ok());
    publish_result.value()->publish_track_status(track_status{
        .track = subscribe_request.track,
        .code = track_status_code::active,
        .latest_object =
            object_location{
                .group_id = 4U,
                .subgroup_id = 0U,
                .object_id = 2U,
            },
    });
    publish_result.value()->withdraw();

    ASSERT_NE(relay.last_namespace_handle, nullptr);
    EXPECT_EQ(relay.last_namespace_handle->statuses.size(), 1U);
    EXPECT_TRUE(relay.last_namespace_handle->withdrawn);
}

TEST(MoqtSessionRolesTest, ConsumersCaptureStatusAndResetSignals) {
    recording_track_consumer track_consumer;
    recording_fetch_consumer fetch_consumer;
    const auto track = make_track("example.com", "meeting_7", "camera");
    const auto status = track_status{
        .track = track,
        .code = track_status_code::ended,
        .latest_object =
            object_location{
                .group_id = 4U,
                .subgroup_id = 0U,
                .object_id = 8U,
            },
    };
    const auto reset = request_reset{
        .code = request_reset_code::transport_error,
        .wire_code = 77U,
    };

    track_consumer.on_track_status(status);
    track_consumer.on_group_boundary(group_boundary{.track = track, .group_id = 4U});
    track_consumer.on_end_of_track(track);
    track_consumer.on_reset(reset);

    fetch_consumer.on_track_status(status);
    fetch_consumer.on_reset(reset);

    EXPECT_EQ(track_consumer.statuses.size(), 1U);
    EXPECT_EQ(track_consumer.boundaries.size(), 1U);
    EXPECT_TRUE(track_consumer.end_of_track.has_value());
    EXPECT_EQ(track_consumer.resets.size(), 1U);
    EXPECT_EQ(fetch_consumer.statuses.size(), 1U);
    EXPECT_EQ(fetch_consumer.resets.size(), 1U);
}
