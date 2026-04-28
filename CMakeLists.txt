cmake_minimum_required(VERSION 3.30)

project(
    mqxx
    VERSION 0.1.0
    DESCRIPTION "Beginner-friendly prototype Media over QUIC relay"
    LANGUAGES CXX)

if(NOT CMAKE_GENERATOR MATCHES "Ninja" AND NOT CMAKE_GENERATOR MATCHES "Visual Studio")
    message(
        FATAL_ERROR
            "mqxx uses named C++ modules and requires a generator with module support. "
            "Configure with Ninja, Ninja Multi-Config, or a recent Visual Studio generator.")
endif()

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

add_library(mqxx_core)

target_sources(
    mqxx_core
    PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        FILES
            src/moqt/full_track_name.cppm
            src/moqt/namespace_registry.cppm
            src/moqt/static_track_descriptor.cppm
            src/transport/transport_session.cppm
            src/transport/fake_transport_session.cppm)

target_compile_features(mqxx_core PUBLIC cxx_std_26)

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

    target_sources(
        mqxx_tests
        PRIVATE
            FILE_SET cxx_modules
            TYPE CXX_MODULES
            FILES
                tests/test_framework.cppm)

    target_link_libraries(mqxx_tests PRIVATE mqxx_core)
    target_compile_features(mqxx_tests PRIVATE cxx_std_26)

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
