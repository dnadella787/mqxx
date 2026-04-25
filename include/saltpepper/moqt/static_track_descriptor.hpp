#pragma once

#include "saltpepper/moqt/full_track_name.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

namespace saltpepper::moqt {

template <std::size_t Size> struct FixedString {
    std::array<char, Size> value{};

    constexpr FixedString(const char (&text)[Size]) {
        std::copy_n(text, Size, value.begin());
    }

    [[nodiscard]] constexpr auto view() const -> std::string_view {
        return std::string_view{value.data(), Size - 1U};
    }
};

template <FixedString TrackNameValue, FixedString... NamespaceFieldValues>
struct StaticTrackDescriptor {
    static_assert(sizeof...(NamespaceFieldValues) <= 32U,
                  "MOQT allows at most 32 namespace fields");

    static constexpr std::size_t namespace_field_count = sizeof...(NamespaceFieldValues);

    [[nodiscard]] static constexpr auto namespace_fields() {
        return std::array<std::string_view, namespace_field_count>{NamespaceFieldValues.view()...};
    }

    [[nodiscard]] static constexpr auto track_name() -> std::string_view {
        return TrackNameValue.view();
    }

    [[nodiscard]] static auto make_runtime_name() -> FullTrackName {
        FullTrackName runtime_name;
        runtime_name.track_namespace.reserve(namespace_field_count);

        for (const std::string_view field : namespace_fields()) {
            runtime_name.track_namespace.emplace_back(field.begin(), field.end());
        }

        runtime_name.track_name.assign(track_name().begin(), track_name().end());
        return runtime_name;
    }
};

} // namespace saltpepper::moqt
