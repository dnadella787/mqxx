cmake_minimum_required(VERSION 3.30)

project(
    mqxx
    VERSION 0.1.0
    DESCRIPTION "Moxygen-aligned Media over QUIC library and relay stack"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MQXX_BUILD_TESTS "Build unit and integration-style tests." OFF)
option(MQXX_ENABLE_CLANG_TIDY "Enable clang-tidy when available." ON)
option(MQXX_ENABLE_SANITIZERS "Enable address/undefined sanitizers in supported builds." OFF)
option(MQXX_WARNINGS_AS_ERRORS "Treat warnings as errors." OFF)

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

find_program(MQXX_CLANG_TIDY_EXE NAMES clang-tidy)

function(mqxx_configure_target target)
    target_compile_features(${target} PRIVATE cxx_std_26)
    mqxx_enable_warnings(${target})
    mqxx_enable_sanitizers(${target})

    if(MQXX_ENABLE_CLANG_TIDY AND MQXX_CLANG_TIDY_EXE)
        set_target_properties(
            ${target}
            PROPERTIES
                CXX_CLANG_TIDY
                    "${MQXX_CLANG_TIDY_EXE};--config-file=${PROJECT_SOURCE_DIR}/.clang-tidy")
    endif()
endfunction()

add_subdirectory(mqxx/common)
add_subdirectory(mqxx/moqt)
add_subdirectory(mqxx/transport)

add_library(mqxx_core INTERFACE)
target_compile_features(mqxx_core INTERFACE cxx_std_26)
target_link_libraries(mqxx_core INTERFACE mqxx_common mqxx_moqt mqxx_transport)

if(MQXX_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()
