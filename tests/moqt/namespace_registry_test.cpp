#include "mqxx/moqt/namespace_registry.hpp"
#include "tests/moqt/helpers.hpp"

#include <gtest/gtest.h>

namespace mqxx::moqt {
namespace {

TEST(namespace_registry_test, empty_registry_has_no_exact_match) {
    const namespace_registry registry;
    const auto name_space = make_namespace({{std::byte{0x01}}});

    EXPECT_TRUE(registry.empty());
    EXPECT_EQ(registry.size(), 0U);
    EXPECT_FALSE(registry.find_exact(name_space).has_value());
}

TEST(namespace_registry_test, empty_registry_has_no_prefix_match) {
    const namespace_registry registry;
    const auto name_space = make_namespace({{std::byte{0x01}}});

    EXPECT_FALSE(registry.find_longest_prefix(name_space).has_value());
}

TEST(namespace_registry_test, insertion_stores_namespace_entry) {
    namespace_registry registry;
    auto name_space = make_namespace({{std::byte{0x01}}});

    const auto result = registry.insert(name_space);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get().ns, name_space);
    EXPECT_FALSE(registry.empty());
    EXPECT_EQ(registry.size(), 1U);

    const auto match = registry.find_exact(name_space);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->get().ns, name_space);
}

TEST(namespace_registry_test, exact_lookup_uses_byte_exact_namespace_identity) {
    namespace_registry registry;
    auto first = make_namespace({{std::byte{0x01}}});
    auto second = make_namespace({{std::byte{0x02}}});

    ASSERT_TRUE(registry.insert(first).has_value());
    ASSERT_TRUE(registry.insert(second).has_value());

    const auto first_match = registry.find_exact(first);
    const auto second_match = registry.find_exact(second);

    ASSERT_TRUE(first_match.has_value());
    ASSERT_TRUE(second_match.has_value());
    EXPECT_EQ(first_match->get().ns, first);
    EXPECT_EQ(second_match->get().ns, second);
}

TEST(namespace_registry_test, duplicate_insert_is_explicit_and_preserves_existing_entry) {
    namespace_registry registry;
    auto name_space = make_namespace({{std::byte{0x01}}});

    ASSERT_TRUE(registry.insert(name_space).has_value());
    const auto duplicate_result = registry.insert(name_space);

    ASSERT_FALSE(duplicate_result.has_value());
    EXPECT_EQ(duplicate_result.error(), namespace_registry_error::duplicate_namespace);
    EXPECT_EQ(registry.size(), 1U);

    const auto match = registry.find_exact(name_space);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->get().ns, name_space);
}

TEST(namespace_registry_test, prefix_lookup_returns_nullopt_when_no_registered_prefix_matches) {
    namespace_registry registry;
    auto registered = make_namespace({{std::byte{0x01}}});
    auto query = make_namespace({{std::byte{0x02}}, {std::byte{0x03}}});

    ASSERT_TRUE(registry.insert(registered).has_value());

    EXPECT_FALSE(registry.find_longest_prefix(query).has_value());
}

TEST(namespace_registry_test, empty_namespace_matches_any_query_as_prefix) {
    namespace_registry registry;
    auto empty = make_namespace({});
    auto query = make_namespace({{std::byte{0x01}}, {std::byte{0x02}}});

    ASSERT_TRUE(registry.insert(empty).has_value());

    const auto match = registry.find_longest_prefix(query);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->get().ns, empty);
}

TEST(namespace_registry_test, prefix_lookup_returns_longest_matching_namespace) {
    namespace_registry registry;
    auto short_prefix = make_namespace({{std::byte{0x01}}});
    auto long_prefix = make_namespace({{std::byte{0x01}}, {std::byte{0x02}}});
    auto query = make_namespace({{std::byte{0x01}}, {std::byte{0x02}}, {std::byte{0x03}}});

    ASSERT_TRUE(registry.insert(short_prefix).has_value());
    ASSERT_TRUE(registry.insert(long_prefix).has_value());

    const auto match = registry.find_longest_prefix(query);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->get().ns, long_prefix);
}

TEST(namespace_registry_test, exact_match_wins_over_shorter_prefix) {
    namespace_registry registry;
    auto short_prefix = make_namespace({{std::byte{0x01}}});
    auto exact = make_namespace({{std::byte{0x01}}, {std::byte{0x02}}});

    ASSERT_TRUE(registry.insert(short_prefix).has_value());
    ASSERT_TRUE(registry.insert(exact).has_value());

    const auto match = registry.find_longest_prefix(exact);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->get().ns, exact);
}

TEST(namespace_registry_test, prefix_matching_compares_whole_fields_not_field_byte_prefixes) {
    namespace_registry registry;
    auto byte_prefix_inside_field = make_namespace({{std::byte{0x01}}});
    auto query = make_namespace(
        {{std::byte{0x01}, std::byte{0x02}}}); // rmbr a field is denoted by vec<byte>

    ASSERT_TRUE(registry.insert(byte_prefix_inside_field).has_value());

    EXPECT_FALSE(registry.find_longest_prefix(query).has_value());
}

} // namespace
} // namespace mqxx::moqt
