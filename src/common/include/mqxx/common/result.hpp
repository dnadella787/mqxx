#pragma once

#include <utility>
#include <variant>

namespace mqxx::common {

struct unit {
    auto operator==(const unit& other) const -> bool = default;
};

template <typename Value, typename Error> class result {
  public:
    using value_type = Value;
    using error_type = Error;

    [[nodiscard]] static auto success(Value value) -> result {
        return result(std::move(value));
    }

    [[nodiscard]] static auto failure(Error error) -> result {
        return result(std::move(error));
    }

    [[nodiscard]] auto ok() const -> bool {
        return std::holds_alternative<Value>(storage_);
    }

    [[nodiscard]] auto value() & -> Value& {
        return std::get<Value>(storage_);
    }

    [[nodiscard]] auto value() const& -> const Value& {
        return std::get<Value>(storage_);
    }

    [[nodiscard]] auto value() && -> Value&& {
        return std::get<Value>(std::move(storage_));
    }

    [[nodiscard]] auto error() const -> const Error& {
        return std::get<Error>(storage_);
    }

  private:
    explicit result(Value value) : storage_(std::move(value)) {}
    explicit result(Error error) : storage_(std::move(error)) {}

    std::variant<Value, Error> storage_;
};

} // namespace mqxx::common
