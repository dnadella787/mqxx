#pragma once

#include <exception>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace saltpepper::test {

class TestFailure : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

struct TestCase {
    std::string_view name;
    std::function<void()> function;
};

inline auto registry() -> std::vector<TestCase>& {
    static std::vector<TestCase> tests;
    return tests;
}

struct TestRegistration {
    TestRegistration(std::string_view name, std::function<void()> function) {
        registry().push_back(TestCase{.name = name, .function = std::move(function)});
    }
};

template <typename Left, typename Right>
inline void expect_equal(const Left& left, const Right& right, const char* left_text,
                         const char* right_text, const char* file, int line) {
    if (left == right) {
        return;
    }

    std::ostringstream message;
    message << file << ':' << line << ": expected " << left_text << " == " << right_text;
    throw TestFailure(message.str());
}

inline void expect_true(bool condition, const char* expression, const char* file, int line) {
    if (condition) {
        return;
    }

    std::ostringstream message;
    message << file << ':' << line << ": expected " << expression;
    throw TestFailure(message.str());
}

} // namespace saltpepper::test

#define SP_CONCAT_IMPL(left, right) left##right
#define SP_CONCAT(left, right) SP_CONCAT_IMPL(left, right)

#define SP_TEST(name)                                                                              \
    static void name();                                                                            \
    static ::saltpepper::test::TestRegistration SP_CONCAT(name, _registration){#name, &name};      \
    static void name()

#define SP_EXPECT(expression)                                                                      \
    ::saltpepper::test::expect_true((expression), #expression, __FILE__, __LINE__)

#define SP_EXPECT_EQ(left, right)                                                                  \
    ::saltpepper::test::expect_equal((left), (right), #left, #right, __FILE__, __LINE__)
