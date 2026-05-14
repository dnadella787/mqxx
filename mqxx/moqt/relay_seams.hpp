#pragma once

#include "mqxx/moqt/session_roles.hpp"

#include <vector>

namespace mqxx::moqt {

class relay_endpoint : public publisher, public subscriber {
  public:
    ~relay_endpoint() override = default;
};

enum class forwarding_decision {
    reject,
    satisfy_from_cache,
    forward_upstream,
};

class relay_object_cache {
  public:
    virtual ~relay_object_cache() = default;

    virtual void retain_object(const object_message& object) = 0;
    [[nodiscard]] virtual std::vector<object_message>
    lookup(const subscription_request& request) const = 0;
};

class subscription_aggregator {
  public:
    virtual ~subscription_aggregator() = default;

    [[nodiscard]] virtual bool should_aggregate(const subscription_request& request) const = 0;
    virtual void note_subscribe(const full_track_name& track) = 0;
    virtual void note_unsubscribe(const full_track_name& track) = 0;
};

class forwarding_policy {
  public:
    virtual ~forwarding_policy() = default;

    [[nodiscard]] virtual forwarding_decision decide(const subscription_request& request) const = 0;
};

} // namespace mqxx::moqt
