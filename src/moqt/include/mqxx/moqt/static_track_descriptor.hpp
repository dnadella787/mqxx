#pragma once

#include "mqxx/moqt/full_track_name.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

namespace mqxx::moqt {

template <std::size_t size> struct fixed_string {
    std::array<char, size> value{};

    constexpr fixed_string(const char (&text)[size]) {
        std::copy_n(text, size, value.begin());
    }

    [[nodiscard]] constexpr auto view() const -> std::string_view {
        return std::string_view{value.data(), size - 1U};
    }
};

template <fixed_string track_name_value, fixed_string... namespace_field_values>
struct static_track_descriptor {
    static_assert(sizeof...(namespace_field_values) <= 32U,
                  "MOQT allows at most 32 namespace fields");

    static constexpr std::size_t namespace_field_count = sizeof...(namespace_field_values);

    [[nodiscard]] static constexpr auto namespace_fields() {
        return std::array<std::string_view, namespace_field_count>{
            namespace_field_values.view()...};
    }

    [[nodiscard]] static constexpr auto track_name() -> std::string_view {
        return track_name_value.view();
    }

    [[nodiscard]] static auto make_runtime_name() -> full_track_name {
        full_track_name runtime_name;
        runtime_name.track_namespace.reserve(namespace_field_count);

        for (const std::string_view field : namespace_fields()) {
            runtime_name.track_namespace.emplace_back(field.begin(), field.end());
        }

        runtime_name.track_name.assign(track_name().begin(), track_name().end());
        return runtime_name;
    }
};

} // namespace mqxx::moqt
