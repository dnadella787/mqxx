#include "mqxx/moqt/wire.hpp"

#include "tests/moqt/helpers.hpp"

#include <array>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace mqxx::moqt {
namespace {

constexpr std::byte b(const char ch) {
    return static_cast<std::byte>(static_cast<unsigned char>(ch));
}

byte_buffer bytes(std::initializer_list<unsigned int> values) {
    byte_buffer out;
    out.reserve(values.size());
    for (const unsigned int value : values) {
        out.push_back(static_cast<std::byte>(value));
    }
    return out;
}

full_track_name
make_full_track_name(std::initializer_list<std::initializer_list<std::byte>> namespace_fields,
                     byte_buffer name_bytes) {
    auto name = track_name::make(name_bytes);
    EXPECT_TRUE(name.has_value());

    auto full_name =
        full_track_name::make(make_namespace(namespace_fields), std::move(name).value());
    EXPECT_TRUE(full_name.has_value());
    return std::move(full_name).value();
}

TEST(varint_test, decodes_draft_examples) {
    const std::array cases = {
        std::pair{bytes({0x25}), std::uint64_t{37}},
        std::pair{bytes({0x80, 0x25}), std::uint64_t{37}},
        std::pair{bytes({0xbb, 0xbd}), std::uint64_t{15293}},
        std::pair{bytes({0xed, 0x7f, 0x3e, 0x7d}), std::uint64_t{226442877}},
        std::pair{bytes({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}),
                  std::numeric_limits<std::uint64_t>::max()},
    };

    for (const auto& [encoded, expected_value] : cases) {
        const auto decoded = decode_varint(encoded);
        ASSERT_TRUE(decoded.has_value());
        EXPECT_EQ(decoded->value, expected_value);
        EXPECT_EQ(decoded->encoded_size, encoded.size());
    }
}

TEST(varint_test, encodes_minimal_lengths_at_each_boundary) {
    const std::array values = {
        std::pair{std::uint64_t{0}, std::size_t{1}},
        std::pair{(std::uint64_t{1} << 7) - 1, std::size_t{1}},
        std::pair{(std::uint64_t{1} << 7), std::size_t{2}},
        std::pair{(std::uint64_t{1} << 14) - 1, std::size_t{2}},
        std::pair{(std::uint64_t{1} << 14), std::size_t{3}},
        std::pair{(std::uint64_t{1} << 21) - 1, std::size_t{3}},
        std::pair{(std::uint64_t{1} << 21), std::size_t{4}},
        std::pair{(std::uint64_t{1} << 28) - 1, std::size_t{4}},
        std::pair{(std::uint64_t{1} << 28), std::size_t{5}},
        std::pair{(std::uint64_t{1} << 35) - 1, std::size_t{5}},
        std::pair{(std::uint64_t{1} << 35), std::size_t{6}},
        std::pair{(std::uint64_t{1} << 42) - 1, std::size_t{6}},
        std::pair{(std::uint64_t{1} << 42), std::size_t{7}},
        std::pair{(std::uint64_t{1} << 49) - 1, std::size_t{7}},
        std::pair{(std::uint64_t{1} << 49), std::size_t{8}},
        std::pair{(std::uint64_t{1} << 56) - 1, std::size_t{8}},
        std::pair{(std::uint64_t{1} << 56), std::size_t{9}},
    };

    for (const auto& [value, expected_size] : values) {
        const auto encoded = encode_varint(value);
        EXPECT_EQ(encoded.size, expected_size);
        const auto decoded = decode_varint(encoded.view());
        ASSERT_TRUE(decoded.has_value());
        EXPECT_EQ(decoded->value, value);
        EXPECT_EQ(decoded->encoded_size, expected_size);
    }
}

TEST(varint_test, accepts_non_minimal_encodings) {
    const auto two_byte = decode_varint(bytes({0x80, 0x01}));
    ASSERT_TRUE(two_byte.has_value());
    EXPECT_EQ(two_byte->value, 1U);
    EXPECT_EQ(two_byte->encoded_size, 2U);

    const auto four_byte = decode_varint(bytes({0xe0, 0x00, 0x00, 0x7f}));
    ASSERT_TRUE(four_byte.has_value());
    EXPECT_EQ(four_byte->value, 127U);
    EXPECT_EQ(four_byte->encoded_size, 4U);
}

TEST(varint_test, rejects_empty_and_truncated_input) {
    const auto empty = decode_varint({});
    ASSERT_FALSE(empty.has_value());
    EXPECT_EQ(empty.error(), wire_error::truncated_input);

    const auto truncated = decode_varint(bytes({0xc0, 0x01}));
    ASSERT_FALSE(truncated.has_value());
    EXPECT_EQ(truncated.error(), wire_error::truncated_input);
}

TEST(varint_test, accepts_maximum_nine_byte_value) {
    const auto encoded = bytes({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
    const auto decoded = decode_varint(encoded);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->value, std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(decoded->encoded_size, 9U);
}

TEST(location_test, roundtrips_and_orders) {
    const location low{1, 2};
    const location high{1, 3};
    EXPECT_LT(low, high);

    const auto encoded = encode_location(high);
    const auto decoded = decode_location(encoded);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->value, high);
    EXPECT_EQ(decoded->encoded_size, encoded.size());
}

TEST(key_value_pair_test, decodes_mixed_even_and_odd_types) {
    const auto encoded = bytes({0x02, 0x25, 0x03, 0x03, 0xaa, 0xbb, 0xcc});
    const auto decoded = decode_key_value_pairs(encoded);

    ASSERT_TRUE(decoded.has_value());
    ASSERT_EQ(decoded->size(), 2U);
    EXPECT_EQ((*decoded)[0], (key_value_pair{2, std::uint64_t{37}}));
    EXPECT_EQ((*decoded)[1],
              (key_value_pair{5, byte_buffer{std::byte{0xaa}, std::byte{0xbb}, std::byte{0xcc}}}));
}

TEST(key_value_pair_test, roundtrips_sequences_without_reordering) {
    const std::vector<key_value_pair> pairs = {
        {2, std::uint64_t{10}},
        {3, bytes({0x11, 0x22})},
        {3, bytes({0x33})},
        {8, std::uint64_t{999}},
    };

    const auto encoded = encode_key_value_pairs(pairs);
    ASSERT_TRUE(encoded.has_value());

    const auto decoded = decode_key_value_pairs(*encoded);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(*decoded, pairs);
}

TEST(key_value_pair_test, rejects_delta_type_overflow) {
    const auto decoded = decode_key_value_pairs(
        bytes({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00}));
    ASSERT_FALSE(decoded.has_value());
    EXPECT_EQ(decoded.error(), wire_error::key_type_overflow);
}

TEST(key_value_pair_test, rejects_decreasing_type_sequence_on_encode) {
    const std::vector<key_value_pair> pairs = {
        {3, bytes({0x01})},
        {2, std::uint64_t{1}},
    };

    const auto encoded = encode_key_value_pairs(pairs);
    ASSERT_FALSE(encoded.has_value());
    EXPECT_EQ(encoded.error(), wire_error::key_type_overflow);
}

TEST(key_value_pair_test,
     rejects_odd_value_lengths_above_sixty_five_thousand_five_hundred_thirty_five) {
    const std::vector<key_value_pair> pairs = {
        {1, byte_buffer(65536, std::byte{0x01})},
    };

    const auto encoded = encode_key_value_pairs(pairs);
    ASSERT_FALSE(encoded.has_value());
    EXPECT_EQ(encoded.error(), wire_error::key_value_length_too_large);

    byte_buffer wire;
    const auto type = encode_varint(1);
    const auto length = encode_varint(65536);
    wire.insert(wire.end(), type.view().begin(), type.view().end());
    wire.insert(wire.end(), length.view().begin(), length.view().end());

    const auto decoded = decode_key_value_pairs(wire);
    ASSERT_FALSE(decoded.has_value());
    EXPECT_EQ(decoded.error(), wire_error::key_value_length_too_large);
}

TEST(key_value_pair_test, rejects_truncated_odd_payloads_and_even_varints) {
    const auto odd = decode_key_value_pairs(bytes({0x01, 0x03, 0xaa, 0xbb}));
    ASSERT_FALSE(odd.has_value());
    EXPECT_EQ(odd.error(), wire_error::truncated_input);

    const auto even = decode_key_value_pairs(bytes({0x02, 0xc0, 0x01}));
    ASSERT_FALSE(even.has_value());
    EXPECT_EQ(even.error(), wire_error::truncated_input);
}

TEST(reason_phrase_test, roundtrips_and_enforces_limits) {
    const reason_phrase phrase{"hello"};
    const auto encoded = encode_reason_phrase(phrase);
    ASSERT_TRUE(encoded.has_value());

    const auto decoded = decode_reason_phrase(*encoded);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->value, phrase);
    EXPECT_EQ(decoded->encoded_size, encoded->size());

    const reason_phrase max_phrase{std::string(reason_phrase::max_byte_count, 'x')};
    const auto max_encoded = encode_reason_phrase(max_phrase);
    ASSERT_TRUE(max_encoded.has_value());

    const reason_phrase oversized{std::string(reason_phrase::max_byte_count + 1, 'x')};
    const auto oversized_encoded = encode_reason_phrase(oversized);
    ASSERT_FALSE(oversized_encoded.has_value());
    EXPECT_EQ(oversized_encoded.error(), wire_error::reason_phrase_too_large);

    byte_buffer oversized_wire;
    const auto oversized_length = encode_varint(reason_phrase::max_byte_count + 1);
    oversized_wire.insert(oversized_wire.end(), oversized_length.view().begin(),
                          oversized_length.view().end());
    oversized_wire.insert(oversized_wire.end(), reason_phrase::max_byte_count + 1, b('x'));
    const auto oversized_decoded = decode_reason_phrase(oversized_wire);
    ASSERT_FALSE(oversized_decoded.has_value());
    EXPECT_EQ(oversized_decoded.error(), wire_error::reason_phrase_too_large);
}

TEST(reason_phrase_test, rejects_invalid_utf8) {
    const auto decoded = decode_reason_phrase(bytes({0x02, 0xc3, 0x28}));
    ASSERT_FALSE(decoded.has_value());
    EXPECT_EQ(decoded.error(), wire_error::invalid_utf8);

    const reason_phrase invalid{std::string("\xc3\x28", 2)};
    const auto encoded = encode_reason_phrase(invalid);
    ASSERT_FALSE(encoded.has_value());
    EXPECT_EQ(encoded.error(), wire_error::invalid_utf8);
}

TEST(track_namespace_wire_test, roundtrips_empty_multiple_binary_and_limit_cases) {
    const auto empty_namespace = make_namespace({});
    const auto empty_encoded = encode_track_namespace(empty_namespace);
    const auto empty_decoded = decode_track_namespace(empty_encoded);
    ASSERT_TRUE(empty_decoded.has_value());
    EXPECT_EQ(empty_decoded->value, empty_namespace);

    const auto binary_namespace =
        make_namespace({{std::byte{0x00}, std::byte{0xff}}, {b('a')}, {b('b'), b('c')}});
    const auto binary_encoded = encode_track_namespace(binary_namespace);
    const auto binary_decoded = decode_track_namespace(binary_encoded);
    ASSERT_TRUE(binary_decoded.has_value());
    EXPECT_EQ(binary_decoded->value, binary_namespace);
    EXPECT_EQ(binary_decoded->encoded_size, binary_encoded.size());

    constexpr std::array<std::byte, 1> one = {std::byte{0x01}};
    std::array<byte_buffer_view, track_namespace::max_field_count> views{};
    views.fill(byte_buffer_view(one));
    const auto many_fields = track_namespace::make(views);
    ASSERT_TRUE(many_fields.has_value());
    const auto many_encoded = encode_track_namespace(*many_fields);
    const auto many_decoded = decode_track_namespace(many_encoded);
    ASSERT_TRUE(many_decoded.has_value());
    EXPECT_EQ(many_decoded->value, *many_fields);

    const byte_buffer exact_field(track_namespace::max_byte_count, std::byte{0x11});
    const std::array exact_views = {byte_buffer_view(exact_field)};
    const auto exact_namespace = track_namespace::make(exact_views);
    ASSERT_TRUE(exact_namespace.has_value());
    const auto exact_encoded = encode_track_namespace(*exact_namespace);
    const auto exact_decoded = decode_track_namespace(exact_encoded);
    ASSERT_TRUE(exact_decoded.has_value());
    EXPECT_EQ(exact_decoded->value, *exact_namespace);
}

TEST(track_namespace_wire_test, rejects_zero_length_and_over_limit_namespace_fields) {
    const auto empty_field = decode_track_namespace(bytes({0x01, 0x00}));
    ASSERT_FALSE(empty_field.has_value());
    EXPECT_EQ(empty_field.error(), wire_error::empty_namespace_field_on_wire);

    const auto too_many_fields = decode_track_namespace(bytes({0x21}));
    ASSERT_FALSE(too_many_fields.has_value());
    EXPECT_EQ(too_many_fields.error(), wire_error::namespace_field_count_too_large);

    byte_buffer oversized;
    const auto count = encode_varint(1);
    const auto length = encode_varint(track_namespace::max_byte_count + 1);
    oversized.insert(oversized.end(), count.view().begin(), count.view().end());
    oversized.insert(oversized.end(), length.view().begin(), length.view().end());
    const auto oversized_decoded = decode_track_namespace(oversized);
    ASSERT_FALSE(oversized_decoded.has_value());
    EXPECT_EQ(oversized_decoded.error(), wire_error::namespace_too_large);
}

TEST(full_track_name_wire_test, roundtrips_empty_name_empty_namespace_and_limit_case) {
    const auto empty_name = make_full_track_name({{b('n'), b('s')}}, {});
    const auto empty_name_encoded = encode_full_track_name(empty_name);
    const auto empty_name_decoded = decode_full_track_name(empty_name_encoded);
    ASSERT_TRUE(empty_name_decoded.has_value());
    EXPECT_EQ(empty_name_decoded->value, empty_name);

    const auto empty_namespace = make_full_track_name({}, byte_buffer{b('t')});
    const auto empty_namespace_encoded = encode_full_track_name(empty_namespace);
    const auto empty_namespace_decoded = decode_full_track_name(empty_namespace_encoded);
    ASSERT_TRUE(empty_namespace_decoded.has_value());
    EXPECT_EQ(empty_namespace_decoded->value, empty_namespace);

    const auto exact_limit =
        make_full_track_name({}, byte_buffer(full_track_name::max_byte_count, std::byte{0x33}));
    const auto exact_limit_encoded = encode_full_track_name(exact_limit);
    const auto exact_limit_decoded = decode_full_track_name(exact_limit_encoded);
    ASSERT_TRUE(exact_limit_decoded.has_value());
    EXPECT_EQ(exact_limit_decoded->value, exact_limit);
}

TEST(full_track_name_wire_test, rejects_over_limit_and_truncated_inputs) {
    byte_buffer oversized = encode_track_namespace(make_namespace({{b('a')}}));
    const auto length = encode_varint(full_track_name::max_byte_count);
    oversized.insert(oversized.end(), length.view().begin(), length.view().end());
    oversized.insert(oversized.end(), full_track_name::max_byte_count, b('x'));

    const auto oversized_decoded = decode_full_track_name(oversized);
    ASSERT_FALSE(oversized_decoded.has_value());
    EXPECT_EQ(oversized_decoded.error(), wire_error::full_track_name_too_large);

    const auto truncated = decode_full_track_name(bytes({0x00, 0x03, 0x01, 0x02}));
    ASSERT_FALSE(truncated.has_value());
    EXPECT_EQ(truncated.error(), wire_error::truncated_input);
}

} // namespace
} // namespace mqxx::moqt
