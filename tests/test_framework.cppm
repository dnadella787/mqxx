module;

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module mqxx.test.framework;

export namespace mqxx::test {

class test_failure : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

struct test_case {
    std::string_view name;
    std::function<void()> function;
};

inline auto registry() -> std::vector<test_case>& {
    static std::vector<test_case> tests;
    return tests;
}

struct test_registration {
    template <typename Function> test_registration(std::string_view name, Function&& function) {
        registry().push_back(test_case{
            .name = name,
            .function = std::function<void()>{std::forward<Function>(function)},
        });
    }
};

template <typename Left, typename Right>
void expect_equal(const Left& left, const Right& right, const char* left_text,
                  const char* right_text, const char* file, int line) {
    if (left == right) {
        return;
    }

    std::ostringstream message;
    message << file << ':' << line << ": expected " << left_text << " == " << right_text;
    throw test_failure(message.str());
}

inline void expect_true(bool condition, const char* expression, const char* file, int line) {
    if (condition) {
        return;
    }

    std::ostringstream message;
    message << file << ':' << line << ": expected " << expression;
    throw test_failure(message.str());
}

} // namespace mqxx::test
