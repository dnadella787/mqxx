#include "test_framework.hpp"

#include <exception>
#include <iostream>

auto main() -> int {
    int failures = 0;

    for (const auto& test_case : mqxx::test::registry()) {
        try {
            test_case.function();
        } catch (const std::exception& exception) {
            std::cerr << "[FAIL] " << test_case.name << ": " << exception.what() << '\n';
            ++failures;
        }
    }

    if (failures == 0) {
        std::cout << "[PASS] " << mqxx::test::registry().size() << " tests passed\n";
        return 0;
    }

    std::cerr << "[FAIL] " << failures << " test(s) failed\n";
    return 1;
}
