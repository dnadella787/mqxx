#include "test_framework.hpp"

#include <iostream>

auto main() -> int {
    int failures = 0;

    for (const auto& test_case : mqxx::test::registry()) {
        mqxx::test::test_context context{.name = test_case.name};
        mqxx::test::start_test(context);
        test_case.function();
        mqxx::test::finish_test();
        failures += context.failures == 0 ? 0 : 1;
    }

    if (failures == 0) {
        std::cout << "[PASS] " << mqxx::test::registry().size() << " tests passed\n";
        return 0;
    }

    std::cerr << "[FAIL] " << failures << " test(s) failed\n";
    return 1;
}
