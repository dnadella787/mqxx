add_library(mqxx_common INTERFACE)
target_compile_features(mqxx_common INTERFACE cxx_std_26)
target_include_directories(mqxx_common INTERFACE ${PROJECT_SOURCE_DIR})
target_sources(
    mqxx_common
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS ${PROJECT_SOURCE_DIR}
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/byte_buffer.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/result.hpp)
