cmake_minimum_required(VERSION 3.25)

project(
    saltpepper
    VERSION 0.1.0
    DESCRIPTION "Beginner-friendly prototype Media over QUIC relay"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SALTPEPPER_BUILD_TESTS "Build unit and integration-style tests." ON)
option(SALTPEPPER_ENABLE_CLANG_TIDY "Enable clang-tidy when available." ON)
option(SALTPEPPER_ENABLE_SANITIZERS "Enable address/undefined sanitizers in supported builds." OFF)
option(SALTPEPPER_WARNINGS_AS_ERRORS "Treat warnings as errors." OFF)

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

find_program(SALTPEPPER_CLANG_TIDY_EXE NAMES clang-tidy)

add_library(
    saltpepper_core
    src/moqt/full_track_name.cpp
    src/moqt/namespace_registry.cpp
    src/transport/fake_transport_session.cpp)

target_include_directories(
    saltpepper_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_compile_features(saltpepper_core PUBLIC cxx_std_20)

saltpepper_enable_warnings(saltpepper_core)
saltpepper_enable_sanitizers(saltpepper_core)

if(SALTPEPPER_ENABLE_CLANG_TIDY AND SALTPEPPER_CLANG_TIDY_EXE)
    set_target_properties(
        saltpepper_core
        PROPERTIES
            CXX_CLANG_TIDY "${SALTPEPPER_CLANG_TIDY_EXE};--config-file=${PROJECT_SOURCE_DIR}/.clang-tidy")
endif()

if(SALTPEPPER_BUILD_TESTS)
    include(CTest)
    enable_testing()

    add_executable(
        saltpepper_tests
        tests/test_main.cpp
        tests/moqt_name_tests.cpp
        tests/moqt_static_track_descriptor_tests.cpp
        tests/moqt_namespace_registry_tests.cpp
        tests/transport_fake_transport_session_tests.cpp)

    target_link_libraries(saltpepper_tests PRIVATE saltpepper_core)

    saltpepper_enable_warnings(saltpepper_tests)
    saltpepper_enable_sanitizers(saltpepper_tests)

    if(SALTPEPPER_ENABLE_CLANG_TIDY AND SALTPEPPER_CLANG_TIDY_EXE)
        set_target_properties(
            saltpepper_tests
            PROPERTIES
                CXX_CLANG_TIDY "${SALTPEPPER_CLANG_TIDY_EXE};--config-file=${PROJECT_SOURCE_DIR}/.clang-tidy")
    endif()

    add_test(NAME saltpepper_tests COMMAND saltpepper_tests)
endif()
