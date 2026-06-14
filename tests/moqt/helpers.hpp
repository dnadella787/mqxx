#pragma once

#include "mqxx/moqt/track_identity.hpp"

#include <initializer_list>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace mqxx::moqt {

inline track_namespace
make_namespace(std::initializer_list<std::initializer_list<std::byte>> fields) {
    std::vector<byte_buffer> owned_fields;
    owned_fields.reserve(fields.size());
    for (const auto field : fields) {
        owned_fields.emplace_back(field);
    }

    std::vector<byte_buffer_view> field_views;
    field_views.reserve(owned_fields.size());
    for (const auto& field : owned_fields) {
        field_views.emplace_back(field);
    }

    auto name_space = track_namespace::make(field_views);
    EXPECT_TRUE(name_space.has_value());
    return std::move(name_space).value();
}

} // namespace mqxx::moqt
