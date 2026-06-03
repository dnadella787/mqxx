find_package(GTest REQUIRED)

include(GoogleTest)

add_executable(mqxx_tests)
target_sources(
    mqxx_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/common/byte_view_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/moqt/track_identity_test.cpp)
target_link_libraries(mqxx_tests PRIVATE mqxx_common mqxx_moqt GTest::gtest_main)
mqxx_configure_target(mqxx_tests)

gtest_discover_tests(mqxx_tests)
