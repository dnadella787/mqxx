find_package(GTest QUIET)

if(TARGET GTest::gtest_main)
    set(MQXX_GTEST_MAIN_TARGET GTest::gtest_main)
elseif(TARGET GTest::Main)
    set(MQXX_GTEST_MAIN_TARGET GTest::Main)
else()
    include(FetchContent)

    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

    if(WIN32)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
    FetchContent_MakeAvailable(googletest)
    set(MQXX_GTEST_MAIN_TARGET GTest::gtest_main)
endif()

include(GoogleTest)

add_executable(mqxx_tests)
target_sources(
    mqxx_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/moqt_name_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/moqt_static_track_descriptor_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/moqt_namespace_registry_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/moqt_session_roles_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/transport_fake_session_tests.cpp)
target_link_libraries(mqxx_tests PRIVATE mqxx_core ${MQXX_GTEST_MAIN_TARGET})
mqxx_configure_target(mqxx_tests)

gtest_discover_tests(mqxx_tests)
