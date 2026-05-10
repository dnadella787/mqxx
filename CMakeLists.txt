cmake_minimum_required(VERSION 3.30)

project(
    mqxx
    VERSION 0.1.0
    DESCRIPTION "Beginner-friendly prototype Media over QUIC relay"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MQXX_BUILD_TESTS "Build unit and integration-style tests." ON)
option(MQXX_ENABLE_CLANG_TIDY "Enable clang-tidy when available." ON)
option(MQXX_ENABLE_SANITIZERS "Enable address/undefined sanitizers in supported builds." OFF)
option(MQXX_WARNINGS_AS_ERRORS "Treat warnings as errors." OFF)

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

find_program(MQXX_CLANG_TIDY_EXE NAMES clang-tidy)

add_library(
    mqxx_core
    src/moqt/full_track_name.cpp
    src/moqt/namespace_registry.cpp
    src/transport/fake_transport_session.cpp)

target_compile_features(mqxx_core PUBLIC cxx_std_26)
target_include_directories(
    mqxx_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/common/include
        ${PROJECT_SOURCE_DIR}/src/moqt/include
        ${PROJECT_SOURCE_DIR}/src/transport/include)

mqxx_enable_warnings(mqxx_core)
mqxx_enable_sanitizers(mqxx_core)

if(MQXX_ENABLE_CLANG_TIDY AND MQXX_CLANG_TIDY_EXE)
    set_target_properties(
        mqxx_core
        PROPERTIES
            CXX_CLANG_TIDY "${MQXX_CLANG_TIDY_EXE};--config-file=${PROJECT_SOURCE_DIR}/.clang-tidy")
endif()

if(MQXX_BUILD_TESTS)
    include(CTest)
    enable_testing()

    add_executable(
        mqxx_tests
        tests/test_main.cpp
        tests/moqt_name_tests.cpp
        tests/moqt_static_track_descriptor_tests.cpp
        tests/moqt_namespace_registry_tests.cpp
        tests/transport_fake_transport_session_tests.cpp)

    target_link_libraries(mqxx_tests PRIVATE mqxx_core)
    target_compile_features(mqxx_tests PRIVATE cxx_std_26)
    target_include_directories(
        mqxx_tests
        PRIVATE
            ${PROJECT_SOURCE_DIR}/tests)

    mqxx_enable_warnings(mqxx_tests)
    mqxx_enable_sanitizers(mqxx_tests)

    if(MQXX_ENABLE_CLANG_TIDY AND MQXX_CLANG_TIDY_EXE)
        set_target_properties(
            mqxx_tests
            PROPERTIES
                CXX_CLANG_TIDY "${MQXX_CLANG_TIDY_EXE};--config-file=${PROJECT_SOURCE_DIR}/.clang-tidy")
    endif()

    add_test(NAME mqxx_tests COMMAND mqxx_tests)
endif()
