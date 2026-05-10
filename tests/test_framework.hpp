#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace mqxx::test {

struct test_case {
    std::string_view name;
    void (*function)();
};

inline auto registry() -> std::vector<test_case>& {
    static std::vector<test_case> tests;
    return tests;
}

struct test_registration {
    template <typename Function> test_registration(std::string_view name, Function&& function) {
        registry().push_back(test_case{
            .name = name,
            .function = function,
        });
    }
};

struct test_context {
    std::string_view name;
    int failures{0};
};

inline thread_local test_context* current_context = nullptr;

inline void start_test(test_context& context) {
    current_context = &context;
}

inline void finish_test() {
    current_context = nullptr;
}

inline void record_failure(const std::string& message) {
    if (current_context == nullptr) {
        return;
    }

    ++current_context->failures;
    std::cerr << "[FAIL] " << current_context->name << ": " << message << '\n';
}

template <typename Left, typename Right>
void expect_equal(const Left& left, const Right& right, const char* left_text,
                  const char* right_text, const char* file, int line) {
    if (left == right) {
        return;
    }

    std::ostringstream message;
    message << file << ':' << line << ": expected " << left_text << " == " << right_text;
    record_failure(message.str());
}

inline void expect_true(bool condition, const char* expression, const char* file, int line) {
    if (condition) {
        return;
    }

    std::ostringstream message;
    message << file << ':' << line << ": expected " << expression;
    record_failure(message.str());
}

} // namespace mqxx::test
