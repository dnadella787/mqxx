#pragma once

#include "mqxx/common/result.hpp"
#include "mqxx/moqt/session_types.hpp"

#include <memory>

namespace mqxx::moqt {

class subscription_handle;
class fetch_handle;
class publish_namespace_handle;

enum class subscribe_error {
    unsupported_filter,
    duplicate_request,
    not_authorized,
    internal_error,
};

enum class fetch_error {
    unsupported_range,
    not_authorized,
    internal_error,
};

enum class publish_namespace_error {
    duplicate_namespace,
    not_authorized,
    internal_error,
};

using subscribe_result = result<std::unique_ptr<subscription_handle>, subscribe_error>;
using fetch_result = result<std::unique_ptr<fetch_handle>, fetch_error>;
using publish_namespace_result =
    result<std::unique_ptr<publish_namespace_handle>, publish_namespace_error>;

class track_consumer {
  public:
    virtual ~track_consumer() = default;

    virtual void on_object(const object_message& object) = 0;
    virtual void on_group_boundary(const group_boundary& boundary) = 0;
    virtual void on_end_of_track(const full_track_name& track) = 0;
    virtual void on_track_status(const track_status& status) = 0;
    virtual void on_reset(const request_reset& reset) = 0;
};

class subgroup_consumer {
  public:
    virtual ~subgroup_consumer() = default;

    virtual void on_object(const object_message& object) = 0;
    virtual void on_subgroup_boundary(const subgroup_boundary& boundary) = 0;
    virtual void on_group_boundary(const group_boundary& boundary) = 0;
    virtual void on_end_of_track(const full_track_name& track) = 0;
    virtual void on_reset(const request_reset& reset) = 0;
};

class fetch_consumer {
  public:
    virtual ~fetch_consumer() = default;

    virtual void on_object(const object_message& object) = 0;
    virtual void on_end_of_fetch(const full_track_name& track) = 0;
    virtual void on_track_status(const track_status& status) = 0;
    virtual void on_reset(const request_reset& reset) = 0;
};

class subscription_handle {
  public:
    virtual ~subscription_handle() = default;

    virtual void subscribe_update(subscription_request request) = 0;
    virtual void unsubscribe() = 0;
};

class fetch_handle {
  public:
    virtual ~fetch_handle() = default;

    virtual void cancel() = 0;
};

class publish_namespace_handle {
  public:
    virtual ~publish_namespace_handle() = default;

    [[nodiscard]] virtual const track_namespace& published_namespace() const noexcept = 0;
    virtual void withdraw() = 0;
    virtual void publish_track_status(const track_status& status) = 0;
};

class publisher {
  public:
    virtual ~publisher() = default;

    [[nodiscard]] virtual publish_namespace_result
    publish_namespace(publish_namespace_request request) = 0;
};

class subscriber {
  public:
    virtual ~subscriber() = default;

    [[nodiscard]] virtual subscribe_result subscribe(subscription_request request,
                                                     track_consumer& consumer) = 0;
    [[nodiscard]] virtual fetch_result fetch(fetch_request request, fetch_consumer& consumer) = 0;
};

} // namespace mqxx::moqt
