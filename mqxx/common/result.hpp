#pragma once

#include <utility>
#include <variant>

namespace mqxx {

struct unit {
    bool operator==(const unit& other) const = default;
};

template <typename Value, typename Error> class result {
  public:
    using value_type = Value;
    using error_type = Error;

    [[nodiscard]] static result success(Value value) {
        return result(std::move(value));
    }

    [[nodiscard]] static result failure(Error error) {
        return result(std::move(error));
    }

    [[nodiscard]] bool ok() const {
        return std::holds_alternative<Value>(storage_);
    }

    [[nodiscard]] Value& value() & {
        return std::get<Value>(storage_);
    }

    [[nodiscard]] const Value& value() const& {
        return std::get<Value>(storage_);
    }

    [[nodiscard]] Value&& value() && {
        return std::get<Value>(std::move(storage_));
    }

    [[nodiscard]] const Error& error() const {
        return std::get<Error>(storage_);
    }

  private:
    explicit result(Value value) : storage_(std::move(value)) {}
    explicit result(Error error) : storage_(std::move(error)) {}

    std::variant<Value, Error> storage_;
};

} // namespace mqxx::common
